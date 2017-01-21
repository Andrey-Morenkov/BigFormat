// unpack.cpp : Defines the entry point for the DLL application.
//
//#include <stdafx.h>

#ifndef _WIN32_WCE 
#include <windows.h>
#endif

#include <memory.h>

#include "..\\BTRPacking\\TGrayscaleBtrBlock.h"
#include "..\\BTRPacking\\TIndexedBtrBlock.h"
#include "..\\BTRPacking\\TBinBtrBlock.h"
#include "..\\BTRPacking\\TUniIndexedBtrBlock.h"
#include "..\\StatCoding\\RangeCoder.h"

#include "TClassicUnpack.h"

#include "unpack.h"

#ifndef _WIN32_WCE 
BOOL WINAPI DllMain( HANDLE hModule, DWORD  ul_reason_for_call,LPVOID lpReserved)
#else
BOOL APIENTRY DllMain(HANDLE Handle,DWORD ul_reason_for_call,LPVOID lpReserved)
#endif
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


//  2:1   1:1   1:2   1:4   1:8
//  8     7     6     5     4
/*template <int lnSize>*/ int decodeX (int lnSize,uint8 * src, uint8 * dst,	short int len)
{
  static const int rSize = (1 << lnSize); // row size (std - 128)
  static const int olnSize = (lnSize == 8) ? 8 : 7; // output row size (128/256)
  static const int bSize = rSize + 1; // fbtr row size (std - 129)
  static const int sStep = (1 << (14 - lnSize)); // line step in 128*128 src
  static const int lnZoom = 7 - lnSize; // ln shift zoom factor

  uint8 oc = 0xFF;
  if (
      (len == 0) ||
      ((len == 1) && (src[0] == 0xFF)) ||
      ((len == 1) && (src[0] == 0) && (oc = 0)) || // set black
      ((len == 2) && (src[0] == 1) && (oc = src[1])) // set any color
     )
  {
    // One color
    if (lnSize == 7)
    {
      memset(dst, oc, 16384);
    }
    else if (lnSize == 8) // zoom in
    {
      memset(dst, oc, 65536);
    }
    else // zoom out
    {
        for (int i = 0; i < rSize; i ++, dst += 128)
          memset(dst, oc, rSize);
    }
    return 0;
  }
  else if ((len > 1) && (len < 16384) && (src[0] == 0xFF))
  {
    // FBTR
    uint8 *pbuf=new uint8[bSize * bSize];
    TMainCoder Coder(src + 2, false, len - 2);
    TBtrBlock *Block = NULL;

      switch (src[1])
      {
        case 1:
          Block = new TGrayscaleBtrBlock(&Coder, lnSize, false);
          break;
        case 2:
          Block = new TIndexedBtrBlock(&Coder, lnSize, false);
          break;
        case 3:
    	    Block = new TBinBtrBlock(&Coder, lnSize, false);
          break;
        default:
        	return -1;
      }

      int res=Block->UnpackAll(pbuf);
	  delete Block;

      if (res)
      {
        for (int i = 0, p = 0; i < rSize; i ++, p += bSize)
          memcpy(dst + (i << olnSize), pbuf + p, rSize);
        delete(pbuf);
		return 0;
      }
      else
	  {
		delete(pbuf);
        return -1;
	  }
  }
  else
  {
    // Classic
    TClassicUnpack thr;
    
    thr.from = (unsigned char *)src;
    thr.size = len;

    if (lnSize == 7)
    {
      thr.to = (unsigned char *)dst;
      thr.Execute();
    }
    else if (lnSize == 8) // zoom in
    {
      uint8 clbuf[16384];
      thr.to = (unsigned char *)clbuf;
      thr.Execute();

      if (! thr.result)
      {
        uint8 *r = clbuf, *d = (uint8 *)dst;
        for (int i = 0; i < 128; i ++)
        {
          for (int j = 0; j < 256; j ++)
            *(d ++) = *(d ++) = *(r ++);
          memcpy(d, d - 256, 256);
          d += 256;
        }
      }
    } // zoom in
    else // zoom out
    {
      uint8 clbuf[16384];
      thr.to = (unsigned char *)clbuf;
      thr.Execute();

      if (! thr.result)
      {
        uint8 *r = clbuf, *d = (uint8 *)dst;
        for (int i = 0; i < rSize; i ++, r += sStep, d += 128)
          for (int j = 0; j < rSize; j ++)
            d[j] = r[j << lnZoom];
      }
    } // zoom out

    return thr.result;
  }

  return -1;
}

int decode (void * source, void * dest,	short int len)
{
  return decodeX/*<7> */(7,(uint8 *)source, (uint8 *)dest, len);
}

int decode_x0(void * source, void * dest, short int len)
{
  return decodeX/*<8> */(8,(uint8 *)source, (uint8 *)dest, len);
}

int decode_x1(void * source, void * dest, short int len)
{
  return decodeX/*<7> */(7,(uint8 *)source, (uint8 *)dest, len);
}

int decode_x2(void * source, void * dest, short int len)
{
  return decodeX/*<6> */(6,(uint8 *)source, (uint8 *)dest, len);
}

int decode_x4(void * source, void * dest, short int len)
{
  return decodeX/*<5> */(5,(uint8 *)source, (uint8 *)dest, len);
}

int decode_x8(void * source, void * dest, short int len)
{
  return decodeX/*<4> */(4,(uint8 *)source, (uint8 *)dest, len);
}
