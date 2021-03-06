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

int decode (void * source, void * dest,	short int len)
{
  int ERR = -1;
  uint8 *src = (uint8 *)source, *dst = (uint8 *)dest;

  if ((src[0] == 0xFF) && (len > 1) && (len < 16384))
  {
  	uint8 pbuf[16641];
		TMainCoder Coder(src + 2, false, len - 2);
    TBtrBlock *Block = NULL;
    if (src[1] == 1)
	    Block = new TGrayscaleBtrBlock(&Coder, 7, false);
    else if (src[1] == 2)
    	Block = new TIndexedBtrBlock(&Coder, 7, false);
    else if (src[1] == 3)
    	Block = new TBinBtrBlock(&Coder, 7, false);
    else
    	return ERR;

    int sz = Block->UnpackAll(pbuf);
    if (Block)
    	delete Block;

    if (sz > 0)
    {
      int p = 0;
      for (int i = 0; i < 128; i ++, p += 129)
        memcpy(dst + (i << 7), pbuf + p, 128);
      ERR = 0;
    }
  }
  else
  {
	TClassicUnpack* thr=new TClassicUnpack();

    thr->from = (unsigned char *)source;
    thr->to = (unsigned char *)dest;
    thr->size = len;
    thr->Execute();
    ERR = thr->result;

	delete(thr);
  }

  return ERR;
}

int decode_x0(void * source, void * dest, short int len)
{
  int ERR = -1;
  uint8 *src = (uint8 *)source, *dst = (uint8 *)dest;

  if ((src[0] == 0xFF) && (len > 1) && (len < 16384))
  {
  	uint8 pbuf[66049];
		TMainCoder Coder(src + 2, false, len - 2);
    TBtrBlock *Block = NULL;
    if (src[1] == 1)
	    Block = new TGrayscaleBtrBlock(&Coder, 8, false);
    else if (src[1] == 2)
    	Block = new TIndexedBtrBlock(&Coder, 8, false);
    else if (src[1] == 3)
    	Block = new TBinBtrBlock(&Coder, 8, false);
    else
    	return ERR;

    int sz = Block->UnpackAll(pbuf);
    if (Block)
    	delete Block;

    if (sz > 0)
    {
      int p = 0;
      for (int i = 0; i < 256; i ++, p += 257)
        memcpy(dst + (i << 8), pbuf + p, 256);
      ERR = 0;
    }
  }
  else
  {
	TClassicUnpack* thr=new TClassicUnpack();
		uint8 clbuf[16384];

    thr->from = (unsigned char *)source;
    thr->to = (unsigned char *)clbuf;
    thr->size = len;
    thr->Execute();
    ERR = thr->result;

		if (! ERR)
		{
			uint8 *r = clbuf, *d = (uint8 *)dest;
			for (int i = 0; i < 128; i ++)
			{
				for (int j = 0; j < 256; j ++)
					*(d ++) = *(d ++) = *(r ++);
				memcpy(d, d - 256, 256);
				d += 256;
			}
		}
	delete(thr);
  }

  return ERR;
}

int decode_x1(void * source, void * dest, short int len)
{
	return decode(source, dest, len);
}

int decode_x2(void * source, void * dest, short int len)
{
  int ERR = -1;
  uint8 *src = (uint8 *)source, *dst = (uint8 *)dest;

	if ((src[0] == 0xFF) && (len > 1) && (len < 16384))
  {
  	uint8 pbuf[4225];
		TMainCoder Coder(src + 2, false, len - 2);
    TBtrBlock *Block = NULL;
    if (src[1] == 1)
	    Block = new TGrayscaleBtrBlock(&Coder, 6, false);
    else if (src[1] == 2)
    	Block = new TIndexedBtrBlock(&Coder, 6, false);
    else if (src[1] == 3)
    	Block = new TBinBtrBlock(&Coder, 6, false);
    else
    	return ERR;

    int sz = Block->UnpackAll(pbuf);
    if (Block)
    	delete Block;

    if (sz > 0)
    {
      int p = 0;
      for (int i = 0; i < 64; i ++, p += 65)
        memcpy(dst + (i << 7), pbuf + p, 64);
      ERR = 0;
    }
  }
  else
  {
	TClassicUnpack* thr=new TClassicUnpack();
		uint8 clbuf[16384];

    thr->from = (unsigned char *)source;
    thr->to = (unsigned char *)clbuf;
    thr->size = len;
    thr->Execute();
    ERR = thr->result;

		if (! ERR)
		{
			uint8 *r = clbuf, *d = (uint8 *)dest;
			for (int i = 0; i < 64; i ++, r += 256, d += 128)
				for (int j = 0; j < 64; j ++)
					d[j] = r[j << 1];
		}
	delete(thr);
  }

  return ERR;
}

int decode_x4(void * source, void * dest, short int len)
{
  int ERR = -1;
  uint8 *src = (uint8 *)source, *dst = (uint8 *)dest;

  if ((src[0] == 0xFF) && (len > 1) && (len < 16384))
  {
  	uint8 pbuf[1089];
		TMainCoder Coder(src + 2, false, len - 2);
    TBtrBlock *Block = NULL;
    if (src[1] == 1)
	    Block = new TGrayscaleBtrBlock(&Coder, 5, false);
    else if (src[1] == 2)
    	Block = new TIndexedBtrBlock(&Coder, 5, false);
    else if (src[1] == 3)
    	Block = new TBinBtrBlock(&Coder, 5, false);
    else
    	return ERR;

    int sz = Block->UnpackAll(pbuf);
    if (Block)
    	delete Block;

    if (sz > 0)
    {
      int p = 0;
      for (int i = 0; i < 32; i ++, p += 33)
        memcpy(dst + (i << 7), pbuf + p, 32);
      ERR = 0;
    }
  }
  else
  {
	TClassicUnpack* thr=new TClassicUnpack();
		uint8 clbuf[16384];

    thr->from = (unsigned char *)source;
    thr->to = (unsigned char *)clbuf;
    thr->size = len;
    thr->Execute();
    ERR = thr->result;

		if (! ERR)
		{
			uint8 *r = clbuf, *d = (uint8 *)dest;
			for (int i = 0; i < 32; i ++, r += 512, d += 128)
				for (int j = 0; j < 32; j ++)
					d[j] = r[j << 2];
		}
	delete(thr);
  }

  return ERR;
}

int decode_x8(void * source, void * dest, short int len)
{
  int ERR = -1;
  uint8 *src = (uint8 *)source, *dst = (uint8 *)dest;

  if ((src[0] == 0xFF) && (len > 1) && (len < 16384))
  {
  	uint8 pbuf[289];
		TMainCoder Coder(src + 2, false, len - 2);
    TBtrBlock *Block = NULL;
    if (src[1] == 1)
	    Block = new TGrayscaleBtrBlock(&Coder, 4, false);
    else if (src[1] == 2)
    	Block = new TIndexedBtrBlock(&Coder, 4, false);
    else if (src[1] == 3)
    	Block = new TBinBtrBlock(&Coder, 4, false);
    else
    	return ERR;

    int sz = Block->UnpackAll(pbuf);
    if (Block)
    	delete Block;

    if (sz > 0)
    {
      int p = 0;
      for (int i = 0; i < 16; i ++, p += 17)
        memcpy(dst + (i << 7), pbuf + p, 16);
      ERR = 0;
    }
  }
  else
  {
	TClassicUnpack* thr=new TClassicUnpack();
		uint8 clbuf[16384];

    thr->from = (unsigned char *)source;
    thr->to = (unsigned char *)clbuf;
    thr->size = len;
    thr->Execute();
    ERR = thr->result;

		if (! ERR)
		{
			uint8 *r = clbuf, *d = (uint8 *)dest;
			for (int i = 0; i < 16; i ++, r += 1024, d += 128)
				for (int j = 0; j < 16; j ++)
					d[j] = r[j << 3];
		}
	delete(thr);
  }

  return ERR;
}
