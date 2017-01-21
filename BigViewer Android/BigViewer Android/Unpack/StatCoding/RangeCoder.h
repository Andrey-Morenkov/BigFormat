#ifndef _RANGECODER_H
#define _RANGECODER_H

#ifndef _WIN32_WCE 
#include <io.h>
#endif

#include "_TAnyCoder.h"


#define DO(n) for (int __tmp = 0; __tmp < n; __tmp ++)

template <class BaseInt>
class _TAnyRangeCoder:public _TAnyCoder
{
private:
  typedef BaseInt uint;
  const int BaseSize;

  const int TopShift;
  const int BytesInBase;
  const uint TOP;
  const uint BOT;

  uint Low, Code, Range;

protected:
  bool WriteMode;
  uint64 Passed; 
  virtual void OutByte(uint8 c) = 0;
  virtual void OutFlash(void) = 0;
  virtual uint8 InByte(void) = 0;
  virtual void InFlash(void) = 0;

public:
  _TAnyRangeCoder(bool Write): WriteMode(Write), Passed(0),
  	TOP(1i64 << (BaseSize - 8)), BOT(1i64 << (BaseSize - 16)),
    TopShift(sizeof(uint) * 8 - 8), BytesInBase(sizeof(uint)),
    BaseSize(sizeof(uint) * 8)
  {
  }
  virtual ~_TAnyRangeCoder(void) {}

  virtual uint64 GetPassed(void)
  {
    return Passed;
  }
  virtual void StartEncode(void)
  {
    ASSERT(WriteMode);
    Passed = 0i64; Low = 0; Range = (uint) - 1;
    OutFlash();
  }
  virtual void FinishEncode(void)
  {
    ASSERT(WriteMode);
    DO(BytesInBase) OutByte(Low >> TopShift), Low <<= 8;
    Passed += BytesInBase;
    OutFlash();
  }
  virtual void StartDecode(void)
  {
    ASSERT(!WriteMode);
    InFlash();
    Passed = 0i64; Low = Code = 0; Range = (uint) - 1;
    DO(BytesInBase) Code = (Code << 8) | InByte();
    Passed += BytesInBase;
  };
  virtual void FinishDecode(void)
  {
    ASSERT(!WriteMode);
    InFlash();
  }

  virtual void Encode(uint32 cumFreq, uint32 freq, uint32 totFreq)
  {
    ASSERT(WriteMode);
    ASSERT(cumFreq + freq <= totFreq);
    ASSERT(freq);
    ASSERT(totFreq <= BOT);
    Low += cumFreq * (Range /= totFreq);
    Range *= freq;
    while ((Low ^ Low + Range) < TOP || Range < BOT &&
      ((Range = - Low & BOT -1), 1))
      OutByte(Low >> TopShift), Range <<= 8, Low <<= 8, Passed ++;
  }
  virtual void EncodeShift(uint32 cumFreq, uint32 freq, int totFreqSize)
  {
    ASSERT(WriteMode);
    ASSERT(cumFreq + freq <= (1 << totFreqSize) && freq && (1 << totFreqSize) <= BOT);
    Low += cumFreq * (Range >>= totFreqSize);
    Range *= freq;
    while ((Low ^ Low + Range) < TOP || Range < BOT &&
      ((Range = - Low & BOT -1), 1))
      OutByte(Low >> TopShift), Range <<= 8, Low <<= 8, Passed ++;
  }

  virtual uint32 GetFreq(uint32 totFreq)
  {
    ASSERT(!WriteMode);
    ASSERT((Code - Low)  / (Range / totFreq) <= totFreq);
    return (Code - Low) / (Range /= totFreq);
  }
  virtual uint32 GetFreqShift(int totFreqSize)
  {
    ASSERT(!WriteMode);
    ASSERT((Code - Low) / (Range >> totFreqSize) < (1 << totFreqSize));
    return (Code - Low) / (Range >>= totFreqSize);
  }
  virtual void Decode(uint32 cumFreq, uint32 freq, uint32 totFreq)
  {
    ASSERT(!WriteMode);
    ASSERT(cumFreq + freq <= totFreq && freq && totFreq <= BOT);
    Low += cumFreq * Range;
    Range *= freq;
    while ((Low ^ Low + Range) < TOP || Range < BOT &&
      ((Range = - Low & BOT - 1), 1))
      Code = (Code << 8) | InByte(), Range <<= 8, Low <<= 8, Passed ++;
  }
}; // class _TAnyRangeCoder

template <class BaseClass>
class TMemRangeCoder:public _TAnyRangeCoder<BaseClass>
{
private:
  uint8 *Buffer;
  int BufSize, Ptr;
protected:
  void OutByte(uint8 c)
  {
    Buffer[Ptr ++] = c;
    if (Ptr == BufSize)
	    throw ("Out of buffer");
  }
  void OutFlash(void)
  {
		Ptr = 0;
  }

  uint8 InByte()
  {
    if (Ptr == BufSize)
      throw ("Out of buffer");
    return Buffer[Ptr ++];
  }
  void InFlash(void)
  {
  	Ptr = 0;
  }
public:
  TMemRangeCoder(uint8 *buf, bool Write, int bufSize):
    BufSize(bufSize), Buffer(buf), _TAnyRangeCoder<BaseClass>(Write)
  {
  	Ptr = 0;
  }
  virtual ~TMemRangeCoder(void) {}

  void SetBuffer(uint8 *buf, int bufSize)
  {
    BufSize = bufSize;
    Buffer = buf;
  	Ptr = 0;
  }
}; // class TMemRangeCoder

template <class BaseClass>
class TFileRangeCoder:public _TAnyRangeCoder<BaseClass>
{
private:
  int FileHandle;
  uint8 *Buffer;
  bool killBuf;
  int BufSize, Ptr;
  long int StartPosition;
protected:
  void OutByte(uint8 c)
  {
    Buffer[Ptr ++] = c;
    if (Ptr == BufSize)
      OutFlash();
  }
  void OutFlash(void)
  {
     write(FileHandle, Buffer, Ptr);
     Ptr = 0;
  }

  uint8 InByte()
  {
    if (Ptr == BufSize)
      Ptr = 0;
    if (! Ptr)
      read(FileHandle, Buffer, BufSize);
    return Buffer[Ptr ++];
  }
  void InFlash(void)
  {
    lseek(FileHandle, StartPosition, 0);
    lseek(FileHandle, GetPassed(), 1);
  }
public:
  TFileRangeCoder(int file, bool Write = false, int bufSize = 32768):
    BufSize(bufSize), FileHandle(file), _TAnyRangeCoder<BaseClass>(Write), Ptr(0)
  {
    try
    {
      Buffer = new uint8[BufSize];
      killBuf = true;
    }
    catch(...)
    {
      INTERNAL_ERROR("Memory allocation error");
    }
    StartPosition = lseek(FileHandle, 0L, 1);
  }

  TFileRangeCoder(int file, bool Write, void *buf, int bufSize):
    BufSize(bufSize), FileHandle(file), _TAnyRangeCoder<BaseClass>(Write), Ptr(0)
  {
    Buffer = (uint8 *)buf;
    killBuf = false;
    StartPosition = lseek(FileHandle, 0L, 1);
  }

  virtual ~TFileRangeCoder(void) {if (killBuf) delete[] Buffer;}
}; // class TFileRangeCoder


typedef TMemRangeCoder<uint32> TMainCoder;

#ifndef _WIN32_WCE 
typedef TFileRangeCoder<uint32> TFileCoder;
#endif

#endif
