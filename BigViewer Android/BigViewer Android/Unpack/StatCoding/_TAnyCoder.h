// _TAnyCoder.h: interface for the _TAnyCoder class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX__TANYCODER_H__8F27E2D7_3FD8_11D6_A242_00D0B7A7A8D4__INCLUDED_)
#define AFX__TANYCODER_H__8F27E2D7_3FD8_11D6_A242_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "errors.h"

typedef unsigned __int64 uint64;
typedef unsigned __int32 uint32;
typedef unsigned __int16 uint16;
typedef unsigned __int8 uint8;

class _TAnyCoder  
{
public:
	_TAnyCoder();
	virtual ~_TAnyCoder();

	virtual uint64 GetPassed() = 0;
	virtual void FinishEncode() = 0;
	virtual void StartEncode() = 0;
	virtual void FinishDecode() = 0;
	virtual void StartDecode() = 0;

  virtual void Encode(uint32 cumFreq, uint32 freq, uint32 totFreq) = 0;
  virtual void EncodeShift(uint32 cumFreq, uint32 freq, int totFreqSize) = 0;

  virtual uint32 GetFreq(uint32 totFreq) = 0;
  virtual uint32 GetFreqShift(int totFreqSize) = 0;
  virtual void Decode(uint32 cumFreq, uint32 freq, uint32 totFreq) = 0;

};

#endif // !defined(AFX__TANYCODER_H__8F27E2D7_3FD8_11D6_A242_00D0B7A7A8D4__INCLUDED_)
