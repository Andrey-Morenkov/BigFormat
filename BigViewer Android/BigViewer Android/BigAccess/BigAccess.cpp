//---------------------------------------------------------------------------
/*
#include "stdafx.h"

#ifndef _WIN32_WCE
#define LOADLIBRARY(str) LoadLibrary(str)
#else
#define LOADLIBRARY(str) LoadLibrary(TEXT(str))
#endif

#ifndef _WIN32_WCE
#define GETPROCADDRESS(handle,str) GetProcAddress(handle,str)
#else
#define GETPROCADDRESS(handle,str) GetProcAddressA(handle,(str+1))
#endif

#ifndef _WIN32_WCE
	#include <vcl.h>
	#include "packitem.h"
*/

	#include <inifiles.hpp>
	#include <mem.h>
	#include <stdio.h>
	#include <..\\Unpack\\unpack\\unpack.h>

/*
	//#include <Winbase.h>
#else
	#include "WINCE.h"
#endif

#define BIGDLLEXPORT extern "C" __declspec(dllexport)
*/

#include <math.h>

//#define _MDEBUG_MODE
//#pragma hdrstop

//---------------------------------------------------------------------------
//   Important note about DLL memory management when your DLL uses the
//   static version of the RunTime Library:
//
//   If your DLL exports any functions that pass String objects (or structs/
//   classes containing nested Strings) as parameter or function results,
//   you will need to add the library MEMMGR.LIB to both the DLL project and
//   any other projects that use the DLL.  You will also need to use MEMMGR.LIB
//   if any other projects which use the DLL will be perfomring new or delete
//   operations on any non-TObject-derived classes which are exported from the
//   DLL. Adding MEMMGR.LIB to your project will change the DLL and its calling
//   EXE's to use the BORLNDMM.DLL as their memory manager.  In these cases,
//   the file BORLNDMM.DLL should be deployed along with your DLL.
//
//   To avoid using BORLNDMM.DLL, pass string information using "char *" or
//   ShortString parameters.
//
//   If your DLL uses the dynamic version of the RTL, you do not need to
//   explicitly add MEMMGR.LIB as this will be done implicitly for you
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------

#define _SRC Sources[Handle]

FILE *LogFile = NULL;

#define _BASE_CHECK \
{\
  if (!SystemInited) \
  { \
    LeaveCriticalSection(&Lock); \
    return rcSystemNotInited;\
  } \
  if (!_SRC.SrcOpened) \
  { \
    LeaveCriticalSection(&Lock); \
    return rcSrcAlreadyClosed;\
  } \
}

typedef int (*PUnpack) (void * source, void * dest, short int len);
/*
PUnpack mydecode;
PUnpack decode_x2;
PUnpack decode_x4;
PUnpack decode_x8;
*/

int UnpackBlock(unsigned char *Dst, unsigned char *Src, int PackedSize);
//---------------------------------------------------------------------------

const int
  rcOk = 0,
  rcSrcAlreadyOpened = -101,
  rcSrcAlreadyClosed = -102,
  rcCantOpenIdxFile = -103,
  rcCantOpenBigFile = -104,
  rcInvalidIniFile = -105,
  rcMemoryError = -106,
  rcCantIncludeUnpackDll = -107,
  rcInvalidUnpackDll = -108,
  rcDiskError = -109,
  rcUnpackError = -110,
  rcZoomNotImplemented = -111,
  rcIniNotOpened = -112,
  rcNoMapPresent = -113,
  rcLayerNotPresent = -114,
  rcSystemAlreadyInited = -115,
  rcSystemNotInited = -116,
  rcTooManyOpenedSources = -117,
  rcFatalError = -118,
  rcBppNotImplemented = -119,
  rcInvalidBpp = -120,
  rcNoPalette = -121;

char v1[8] = {3, 3, 3, 3, 3, 3, 3, 3};
char v2[8] = {7, 7, 7, 7, 7, 7, 7, 7};
unsigned short int v3[4] = {255, 255, 255, 255};
unsigned int v4[2] = {65535, 65535};

const int MapBigBlockPerRow[4] = {5, 7, 8, 10};
const int MaxSourceCount = 50;
const int MaxLayerCount = 15;
const int LayerNameLen = 15;
const int MaxPriorityCount = 16;
const int MaxBigBlockSize = 81 * 1024; // 81Kb > 64Kb + 16Kb + LayerTableSize

enum TBpp {bpp332, bppGrayscale8, bpp888, bppAnyPal8};

typedef struct
{
  char Style;
  char UnicColor;
} TLayerInfo;

class TSrcDesc
{
  public:
    bool SrcOpened, IniOpened;
    int UnicHandle;

	#ifndef _WIN32_WCE
		int BigFile;
	#else
		HANDLE BigFile;
	#endif

	unsigned int *IdxInfo;

    int BigPixelWidth, BigPixelHeight, BigRowCount, BigColCount, ZoomOut;
    unsigned int *BigBlockOffsets;
    bool BigOldVersion;
    TBpp Bpp;

    int SmallMapIndex, LargeMapIndex;
    unsigned int *MapBigBlockOffsets[4], MapWidths[4], MapHeights[4];

    unsigned int AllLayers ;
    int LayerCount;
    unsigned int ActiveLayers;
    char ActiveLayerCount;
    int ActiveSortedLayers[MaxLayerCount + 1];
    TLayerInfo Layers[MaxLayerCount];
    int SortedLayers[MaxPriorityCount * MaxLayerCount];
    int Priorities[MaxLayerCount];

    int ColorsCount;
    unsigned char *ColorTable;

    double XOffset, YOffset, Scale;

  public:
    TSrcDesc(void)
    {
      SetDefault();
    }
    void SetDefault(void)
    {
      SrcOpened = false;
      XOffset = 0.0;
      YOffset = 0.0;
      Scale = 1.0;

	#ifndef _WIN32_WCE 
      BigFile = -1;
	#else
      BigFile = INVALID_HANDLE_VALUE;
	#endif
      
	  IdxInfo = NULL;
    }
};

TSrcDesc Sources[MaxSourceCount];
int OpenedSourceCount = 0;
CRITICAL_SECTION Lock;

//---------------------------------------------------------------------------
typedef void (*PMixer) (unsigned char *, unsigned char *, char, char);
PMixer MixLayers;
/*
void MixerNoMMX(void *Dst, void *Src, char Style, char UnicColor)
{
  memcpy(Dst, Src, 16384);
}
*/

static unsigned char T_3x3[8][8] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1, 1},
  {1, 2, 2, 1, 2, 2, 2, 2},
  {2, 2, 3, 3, 2, 3, 3, 3},
  {2, 3, 3, 4, 4, 3, 4, 4},
  {3, 3, 4, 4, 5, 5, 5, 5},
  {3, 4, 4, 5, 5, 6, 6, 6},
  {4, 4, 5, 5, 6, 6, 7, 7}};
static unsigned char T_2x2[4][4] = {
  {0, 0, 0, 0},
  {1, 1, 1, 1},
  {1, 2, 2, 2},
  {2, 2, 3, 3}};

void MixerNoMMX(unsigned char *dest, unsigned char * sour, char type, char attr)
{
	if (type == 0)
	{
		for (int i = 0; i < 16384; i++)
		{
			if (sour[i] == 0xFF)
				continue;
			dest[i] = (unsigned char)((T_3x3[sour[i]>>5][dest[i]>>5]<<5)|
			(T_3x3[(sour[i]&0x1F)>>2][(dest[i]&0x1F)>>2]<<2)|
			T_2x2[sour[i]&0x03][dest[i]&0x03]);
		}
	}
	if (type == 1)
	{
		for (int i = 0; i < 16384; i++)
		{
			switch (sour[i]>>6)
			{
				case 3:
					dest[i] = attr;
				case 0:
					continue;
				case 1:
					dest[i] = (unsigned char)(((((attr>>5)+((dest[i]>>5)<<1))/3)<<5)|
					(((((attr&0x1F)>>2)+(((dest[i]&0x1F)>>2)<<1))/3)<<2)|
					(((attr&0x03)+((dest[i]&0x03)<<1))/3));
					continue;
				case 2:
					dest[i] = (unsigned char)(((((dest[i]>>5)+((attr>>5)<<1))/3)<<5)|
					(((((dest[i]&0x1F)>>2)+(((attr&0x1F)>>2)<<1))/3)<<2)|
					(((dest[i]&0x03)+((attr&0x03)<<1))/3));
					continue;
			}
		}
	}
	if (type == 2)
	{
		for (int i = 0; i < 16384; i++)
			dest[i] = (unsigned char)(((((dest[i]>>5)*(7-(sour[i]>>5))+
			(attr>>5)*(sour[i]>>5))/7)<<5)|(((((dest[i]&0x1F)>>2)*(7-(sour[i]>>5))+
			((attr&0x1F)>>2)*(sour[i]>>5))/7)<<2)|(((dest[i]&0x03)*(7-(sour[i]>>5))+
			(attr&0x03)*(sour[i]>>5))/7));
	}
	if (type == 3)
	{
    if (attr == 0)
      for (int i = 0; i < 16384; i++)
        dest[i] = (unsigned char)(sour[i] & 0xE0);
    if (attr == 3)
      for (int i = 0; i < 16384; i++)
        dest[i] |= (unsigned char)((sour[i] & 0xE0) >> 3);
    if (attr == 6)
      for (int i = 0; i < 16384; i++)
        dest[i] |= (unsigned char)((sour[i] & 0xC0) >> 6);
  }
}

void Mixer2NoMMX(unsigned char *dest, unsigned char * sour, char type, char attr)
{
	if (type == 3)
	{
    memcpy ( dest, sour, 16384 ) ;
  }
}

//---------------------------------------------------------------------------

class TBigBlock
{
	public:
		unsigned char Image[128 * 128];
};

class TCacheLineInfo
{
	public:
		int Row, Col;
		char Zoomed;
    unsigned int Layers;
    int UnicHandle;
		unsigned int LastAccessTime;
};

class TCache
{
	private:
		unsigned int AccessCount, SuccessCount;
		int LastError;
	protected:
		TBigBlock *Buffer; //!!!!!!!!!!!
		TCacheLineInfo *CacheTable; //!!!!!!!!!!!
		unsigned char PackedImage[MaxBigBlockSize], ResultImage[128 * 128], LayerImage[128 * 128], AllLayers[(MaxLayerCount + 1) * 16384];
		int MaxBufSize, CacheLineCount;
		unsigned int AccessTimeCount;
	protected:
		int ReadBigBlock(int Handle, int Row, int Col);
    void UnpackAllLayers(int Handle, unsigned char *PackedImage, int PackedSize,
	  unsigned char *Unpack, PUnpack UseUnpack);
    void MixAllLayers(int Handle, unsigned char *Unpacked, unsigned char *Dest);
	public:
		TCache(int _MaxBufSize);
		~TCache(void);
		unsigned char *GetBigBlock(int Handle, int Row, int Col, int ZoomOut);
		unsigned char *GetFastBigLogicBlock(int Handle, int Row, int Col, int ZoomOut, unsigned char *Dst = NULL);
    void ExtractMapBlock(int Handle, unsigned int *BlockOffset, unsigned char *Dest);
		int GetLastError(void);
		void IncAccessTime(void);
};

TCache::TCache(int _MaxBufSize) : MaxBufSize(_MaxBufSize),
	Buffer(NULL), CacheTable(NULL), AccessTimeCount(0),
	AccessCount(0), SuccessCount(0),
	LastError(rcOk)
{
	CacheLineCount = MaxBufSize / sizeof(TBigBlock) + 1;
	try {
		Buffer = new TBigBlock[CacheLineCount];
		CacheTable = new TCacheLineInfo[CacheLineCount];
		memset(CacheTable, 0, sizeof(TCacheLineInfo) * CacheLineCount);
	}
	catch (...) {
		if (Buffer != NULL)
		{
			delete[] Buffer;
			Buffer = NULL;
		}
		if (CacheTable != NULL)
		{
			delete[] CacheTable;
			CacheTable = NULL;
		}
		throw 1;
	}
}

TCache::~TCache(void)
{
	if (Buffer != NULL)
	{
		delete[] Buffer;
		Buffer = NULL;
	}
	if (CacheTable != NULL)
	{
		delete[] CacheTable;
		CacheTable = NULL;
	}
}

void TCache::IncAccessTime(void)
{
	AccessTimeCount++;
}

unsigned char *TCache::GetFastBigLogicBlock(int Handle, int Row, int Col, int ZoomOut, unsigned char *Dst)
{
  int PackedSize;
  unsigned char *Result, *Unpacked;
  if (Dst == NULL) Result = ResultImage;
  else Result = Dst;
  if (_SRC.LayerCount) Unpacked = AllLayers;
  else Unpacked = Result;
	switch (ZoomOut)
	{
		case 1:
			{
	  PackedSize = ReadBigBlock(Handle, Row, Col);
	  UnpackAllLayers(Handle, PackedImage, PackedSize, Unpacked, decode);
    break;
			}
		case 2:
			{
			if ((Row + 1 >= _SRC.BigRowCount) || (Col + 1 >= _SRC.BigColCount)) memset(Result, 0xFF, 16384);
      for (int dR = 0; dR < 2; dR++)
        for (int dC = 0; dC < 2; dC++)
          if ((Row + dR < _SRC.BigRowCount) && (Col + dC < _SRC.BigColCount))
          {
            PackedSize = ReadBigBlock(Handle, Row + dR, Col + dC);
            UnpackAllLayers(Handle, PackedImage, PackedSize, Unpacked + (dR << 13) + (dC << 6), decode_x2);
          }
	break;
			}
		case 4:
			{
			if ((Row + 3 >= _SRC.BigRowCount) || (Col + 3 >= _SRC.BigColCount)) memset(Result, 0xFF, 16384);
      for (int dR = 0; dR < 4; dR++)
        for (int dC = 0; dC < 4; dC++)
          if ((Row + dR < _SRC.BigRowCount) && (Col + dC < _SRC.BigColCount))
          {
            PackedSize = ReadBigBlock(Handle, Row + dR, Col + dC);
            UnpackAllLayers(Handle, PackedImage, PackedSize, Unpacked + (dR << 12) + (dC << 5), decode_x4);
          }
    break;
			}
		case 8:
			{
			if ((Row + 7 >= _SRC.BigRowCount) || (Col + 7 >= _SRC.BigColCount)) memset(Result, 0xFF, 16384);
      for (int dR = 0; dR < 8; dR++)
        for (int dC = 0; dC < 8; dC++)
          if ((Row + dR < _SRC.BigRowCount) && (Col + dC < _SRC.BigColCount))
          {
            PackedSize = ReadBigBlock(Handle, Row + dR, Col + dC);
            UnpackAllLayers(Handle, PackedImage, PackedSize, Unpacked + (dR << 11) + (dC << 4), decode_x8);
          }
    break;
			}
		default :
			{
      throw 1;
			}
	}
  if (_SRC.LayerCount) MixAllLayers(Handle, Unpacked, Result);
  return Result;
}

unsigned char *TCache::GetBigBlock(int Handle, int Row, int Col, int ZoomOut)
{
	LastError = rcOk;

  AccessCount++;
	int CacheLine, OldestCacheLine = 0;
	unsigned int OldestAccessTime = CacheTable[OldestCacheLine].LastAccessTime;

	for (CacheLine = 0; CacheLine < CacheLineCount; CacheLine++)
	{
		if (!CacheTable[CacheLine].LastAccessTime)
		{
			OldestCacheLine = CacheLine;
      break;
		}
    if (
      (CacheTable[CacheLine].UnicHandle == _SRC.UnicHandle) &&
      (CacheTable[CacheLine].Row == Row) &&
      (CacheTable[CacheLine].Col == Col) &&
      (CacheTable[CacheLine].Layers == _SRC.ActiveLayers) &&
      (CacheTable[CacheLine].Zoomed == ZoomOut)
      )
    {
      SuccessCount++;
      CacheTable[CacheLine].LastAccessTime = AccessTimeCount;
      return Buffer[CacheLine].Image;
    }
		if (CacheTable[CacheLine].LastAccessTime < OldestAccessTime)
		{
			OldestAccessTime = CacheTable[CacheLine].LastAccessTime;
			OldestCacheLine = CacheLine;
		}
	}

  // Load block
  CacheLine = OldestCacheLine;
  try {
    #ifdef _MDEBUG_MODE
    DWORD st_time=GetTickCount();
    #endif
  
	GetFastBigLogicBlock(Handle, Row, Col, ZoomOut, Buffer[CacheLine].Image);

    #ifdef _MDEBUG_MODE
    DWORD en_time=GetTickCount();
    fprintf(LogFile, "\ngetbigblock speed(ms)=%ld", en_time-st_time);
	#endif

	CacheTable[CacheLine].LastAccessTime = AccessTimeCount;
    CacheTable[CacheLine].Row = Row;
    CacheTable[CacheLine].Col = Col;
    CacheTable[CacheLine].Zoomed = ZoomOut;
    CacheTable[CacheLine].Layers = _SRC.ActiveLayers;
    CacheTable[CacheLine].UnicHandle = _SRC.UnicHandle;
  }
  catch (...) {
    CacheTable[CacheLine].LastAccessTime = 1;
    CacheTable[CacheLine].Row = -1;
    CacheTable[CacheLine].Col = -1;
    CacheTable[CacheLine].Zoomed = 0;
    CacheTable[CacheLine].Layers = 0x80000000;
    CacheTable[CacheLine].UnicHandle = -1;
    throw 1;
  }

  return Buffer[CacheLine].Image;
}

int TCache::ReadBigBlock(int Handle, int Row, int Col)
{
	LastError = rcOk;

	int BlockIndex = Row * _SRC.BigColCount + Col;
	int PackedSize = _SRC.BigBlockOffsets[BlockIndex + 1] - _SRC.BigBlockOffsets[BlockIndex];
	if(PackedSize > MaxBigBlockSize) PackedSize = MaxBigBlockSize;
	FileSeek(_SRC.BigFile, (int)(_SRC.BigBlockOffsets[BlockIndex]), 0);
	
	if (FileRead(_SRC.BigFile, PackedImage, PackedSize) != PackedSize)
	{
		LastError = rcDiskError;
		throw 1;
	}
  return PackedSize;
}

void TCache::UnpackAllLayers(int Handle, unsigned char *PackedImage, int PackedSize,
  unsigned char *Unpacked, PUnpack UseUnpack)
{
  LastError = rcOk;
  int L = 0;

  for (int i = 0; i < _SRC.ActiveLayerCount; i++)
  {
    if (_SRC.ActiveSortedLayers[i] == -1) // �������� ����
    {
	    if (_SRC.LayerCount == 0)
      {
        if (UseUnpack(PackedImage, Unpacked + (L << 14), (short int)PackedSize))
        {
          LastError = rcUnpackError;
          throw 1;
        }
        L++;
      }
      else
      {
        if (UseUnpack(PackedImage + 2 * _SRC.LayerCount, Unpacked + (L << 14),
          (short int)(((unsigned short int *)PackedImage)[0] - _SRC.LayerCount * 2)))
        {
          LastError = rcUnpackError;
          throw 1;
        }
        L++;
      }
    }
    else {
      int Num = _SRC.ActiveSortedLayers[i];
      int PackedLayerSize;
      if (Num < _SRC.LayerCount - 1)
        PackedLayerSize = ((unsigned short int *)PackedImage)[Num + 1] - ((unsigned short int *)PackedImage)[Num];
      else PackedLayerSize = PackedSize - ((unsigned short int *)PackedImage)[Num];

      if (UseUnpack(PackedImage + ((unsigned short int *)PackedImage)[Num], Unpacked + (L << 14), (short int)PackedLayerSize))
      {
        LastError = rcUnpackError;
        throw 1;
      }
      L++;
    }
  }
}

void TCache::MixAllLayers(int Handle, unsigned char *Unpacked, unsigned char *Dest)
{
	LastError = rcOk;
  int L = 0;

  if (_SRC.LayerCount) memset(Dest, 0xFF, 16384);

  for (int i = 0; i < _SRC.ActiveLayerCount; i++)
  {
    if (_SRC.ActiveSortedLayers[i] == -1) // �������� ����
    {
	    if (_SRC.LayerCount == 0) memcpy(Dest, Unpacked, 16384);
      else
      {
        MixLayers(Dest, Unpacked + (L << 14), 0, 0);
        L++;
      }
    }
    else {
      int Num = _SRC.ActiveSortedLayers[i];
      char Style = _SRC.Layers[Num].Style;
      char UnicColor = _SRC.Layers[Num].UnicColor;
      MixLayers(Dest, Unpacked + (L << 14), Style, UnicColor);
      L++;
    }
  }
}

void TCache::ExtractMapBlock(int Handle, unsigned int *BlockOffset, unsigned char *Dest)
{
	LastError = rcOk;

	int PackedSize = BlockOffset[1] - BlockOffset[0];
	if(PackedSize > MaxBigBlockSize) PackedSize = MaxBigBlockSize;
	FileSeek(_SRC.BigFile, (int)(BlockOffset[0]), 0);
	if (FileRead(_SRC.BigFile, PackedImage, PackedSize) != PackedSize)
	{
		LastError = rcDiskError;
		throw 1;
	}

	if (UnpackBlock(Dest, PackedImage, PackedSize))
  {
    LastError = rcUnpackError;
    throw 1;
  }
}

int TCache::GetLastError(void)
{
	return LastError;
}
//---------------------------------------------------------------------------

bool SystemInited = false;
TCache *Cache = NULL;
//===========================================================================

void PrintVars(void)
{
  /*
  fprintf(LogFile, "\nSrcOpened = %ld", (int)SrcOpened);
  fprintf(LogFile, "\nIniOpened = %ld", (int)IniOpened);
  fprintf(LogFile, "\nZoomOut = %ld", ZoomOut);
  fprintf(LogFile, "\nXOffset = %lf", XOffset);
  fprintf(LogFile, "\nYOffset = %lf", YOffset);
  fprintf(LogFile, "\nScale = %lf", Scale);
  fprintf(LogFile, "\nLayerCount = %ld", LayerCount);
  fprintf(LogFile, "\nActiveLayers = %ld\n", ActiveLayers);
  */
}

void PrintOk(void)
{
  fprintf(LogFile, "\nNormal Finish.\n\n\n");
}

//===========================================================================

int inline Mul(int X, int ZoomOut)
{
  return (ZoomOut > 0) ? X * ZoomOut : - X / ZoomOut;
}

int inline Div(int X, int ZoomOut)
{
  return (ZoomOut > 0) ? X / ZoomOut : - X * ZoomOut;
}

int inline Mod(int X, int ZoomOut)
{
  return (ZoomOut > 0) ? X % ZoomOut : 0;
}

//===========================================================================

/*
#ifndef _WIN32_WCE
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void*)
#else
BOOL APIENTRY DllMain(HANDLE Handle,DWORD  reason,LPVOID lpReserved)
#endif
*/
void LibAttach()
{
InitializeCriticalSection(&Lock);
#ifdef _MDEBUG_MODE
if (LogFile == NULL) LogFile = fopen("BigAccess.log", "wt");
#endif
}

void LibDetach()
{
#ifdef _MDEBUG_MODE
if (LogFile != NULL)
	{
	fclose(LogFile);
	LogFile = NULL;
	}
#endif

if (SystemInited)
	  {
		delete Cache;
		Cache = NULL;
		DeleteCriticalSection(&Lock);
		SystemInited = false;
	  }
}
//---------------------------------------------------------------------------

int InitBigAccess(int MaxCacheSize)
{
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nInitBigAccess(%ld);", MaxCacheSize);
  #endif

  if (SystemInited) return rcSystemAlreadyInited;
  SystemInited=true;

  int RC = rcFatalError;

  if (MaxCacheSize < 102400) MaxCacheSize = 102400;
		// � ���� ������ ���� ������ 3 ������
	try {
	  Cache = new TCache(MaxCacheSize);
	}
	catch (...) {
	  RC = rcMemoryError;
	  SystemInited = false;
	}

	if (!SystemInited) return RC;

	for (int i = 0; i < MaxSourceCount; i++) Sources[i].SetDefault();

	MixLayers = MixerNoMMX;

	SystemInited = true;
	RC = rcOk;

	#ifdef _DEBUG_MODE
	PrintVars();
	PrintOk();
	#endif

	return RC;
}
//---------------------------------------------------------------------------

int GetLayers(int Handle, unsigned int *Layers) ;

int SetLayers(int, unsigned int);

int OpenSource(char *SourceName, int *_Handle)
{
EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nOpenSource(%s, ptr);", SourceName);
  #endif

	if (!SystemInited)
  {
    LeaveCriticalSection(&Lock);
    return rcSystemNotInited;
  }
  int Handle;
  for (Handle = 0; Handle < MaxSourceCount; Handle++)
    if (!_SRC.SrcOpened) break;
  if (Handle == MaxSourceCount)
  {
    LeaveCriticalSection(&Lock);
    return rcTooManyOpenedSources;
  }

	TIniFile *Ini = NULL;

	#ifndef _WIN32_WCE
		int IdxFile = -1;
	#else
		HANDLE IdxFile =  INVALID_HANDLE_VALUE;
	#endif

  int RC = rcFatalError;
   try
	{
    _SRC.ZoomOut = 1;

		AnsiString SrcName = AnsiString(SourceName).UpperCase();
		AnsiString BigName, IdxName;

		if (ExtractFileExt(SrcName) == ".INI")
		{
      _SRC.IniOpened = true;
			AnsiString Name;
			try {
				Ini = new TIniFile(SrcName);
				}
			catch (...)
			{
				RC = rcMemoryError;
        throw 1;
			}

	Name="";
	Name = Ini->ReadString("REGION", "Name", "");
	AnsiString tmp = Ini->ReadString("REGION", "Xoffset", "aa");

	if (!strcmp(tmp.c_str(), "aa"))
      {
        tmp = Ini->ReadString("ITEM001", "Xoffset", "aa");
        if (!strcmp(tmp.c_str(), "aa"))
        {
          RC = rcInvalidIniFile;
          throw 1;
        }
        else _SRC.XOffset = atof(tmp.c_str());
      }
      else _SRC.XOffset = atof(tmp.c_str());

      tmp = Ini->ReadString("REGION", "Yoffset", "aa");
      if (!strcmp(tmp.c_str(), "aa"))
      {
        tmp = Ini->ReadString("ITEM001", "Yoffset", "aa");
        if (!strcmp(tmp.c_str(), "aa"))
        {
          RC = rcInvalidIniFile;
          throw 1;
        }
        else _SRC.YOffset = atof(tmp.c_str());
      }
      else _SRC.YOffset = atof(tmp.c_str());

      tmp = Ini->ReadString("REGION", "ScaleView", "aa");
      if (!strcmp(tmp.c_str(), "aa"))
      {
        tmp = Ini->ReadString("ITEM001", "Scale", "aa");
        if (!strcmp(tmp.c_str(), "aa"))
        {
          RC = rcInvalidIniFile;
          throw 1;
        }
        else _SRC.Scale = atof(tmp.c_str());
      }
      else _SRC.Scale = atof(tmp.c_str());

			delete Ini;
			Ini = NULL;

			if (Name == "")
			{
				RC = rcInvalidIniFile;
        throw 1;
			}
			if (_SRC.Scale == 0.0)
			{
				RC = rcInvalidIniFile;
        throw 1;
			}

			if (ExtractFilePath(Name) == "")
			{
				BigName = ExtractFilePath(SrcName) + Name + ".big";
				IdxName = ExtractFilePath(SrcName) + Name + ".idx";
			}
			else
			{
				BigName = Name + ".big";
				IdxName = Name + ".idx";
			}

		}
		else {
      _SRC.IniOpened = false;

		SrcName.SetLength(SrcName.Length() - (ExtractFileExt(SrcName)).Length());
			BigName = SrcName + ".big";
			IdxName = SrcName + ".idx";

	  _SRC.XOffset = 0.0;
      _SRC.YOffset = 0.0;
      _SRC.Scale = 1.0;
		}

		IdxFile = FileOpen(IdxName, fmOpenRead | fmShareDenyWrite);
	#ifndef _WIN32_WCE
		if (IdxFile == -1)
	#else
		if (IdxFile ==  INVALID_HANDLE_VALUE)
	#endif
		{
			RC = rcCantOpenIdxFile;
      throw 1;
		}
		_SRC.BigFile = FileOpen(BigName, fmOpenRead | fmShareDenyWrite);

		#ifndef _WIN32_WCE 
		if (_SRC.BigFile == -1)
		#else
		if (_SRC.BigFile == INVALID_HANDLE_VALUE)
		#endif
		{
			RC = rcCantOpenBigFile;
      throw 1;
		}

		int IdxFileSize = FileSeek(IdxFile, 0, 2);
    try {
      _SRC.IdxInfo = new unsigned int[IdxFileSize / 4 + 1];
    }
    catch (...) {
      RC = rcMemoryError;
      throw 1;
    }
    FileSeek(IdxFile, 0, 0);
    if (FileRead(IdxFile, _SRC.IdxInfo, IdxFileSize) != IdxFileSize)
    {
      RC = rcDiskError;
      throw 1;
    }
    FileClose(IdxFile);

	#ifndef _WIN32_WCE
		IdxFile = -1;
	#else
		IdxFile =  INVALID_HANDLE_VALUE;
	#endif

    _SRC.BigPixelWidth = _SRC.IdxInfo[0];
    _SRC.BigPixelHeight = _SRC.IdxInfo[1];
    _SRC.BigRowCount = _SRC.BigPixelHeight / 128 + ((_SRC.BigPixelHeight % 128) ? 1 : 0);
    _SRC.BigColCount = _SRC.BigPixelWidth / 128 + ((_SRC.BigPixelWidth % 128) ? 1 : 0);
    _SRC.BigOldVersion = ((_SRC.IdxInfo[3] == 0) && (_SRC.IdxInfo[4] != 0));
    _SRC.LayerCount = _SRC.BigOldVersion ? 0 : ((_SRC.IdxInfo[3]  >> 28) & 0x0000000F);
    _SRC.BigBlockOffsets = _SRC.IdxInfo + 3 + 4 * _SRC.LayerCount + (_SRC.BigOldVersion ? 0 : 2);
    if (!_SRC.BigOldVersion)
    {
      if (_SRC.IdxInfo[3] & 0x08000000)
      {
        if (_SRC.IdxInfo[3] & 0x00010000)
        {
          RC = rcInvalidBpp;
          throw 1;
        }
        else {
          _SRC.Bpp = bpp888;
        }
      }
      else {
        if (_SRC.IdxInfo[3] & 0x00010000) _SRC.Bpp = bppGrayscale8;
        else if (_SRC.IdxInfo[3] & 0x04000000)
        {
          // New anypal mode
          _SRC.Bpp = bppAnyPal8;
          _SRC.ColorsCount = _SRC.BigBlockOffsets[_SRC.IdxInfo[2] + _SRC.IdxInfo[4] + 1];
          _SRC.ColorTable = (unsigned char *)(_SRC.BigBlockOffsets + _SRC.IdxInfo[2] + _SRC.IdxInfo[4] + 2);
        }
        else _SRC.Bpp = bpp332;
      }
    }
    else _SRC.Bpp = bpp332;

    _SRC.SmallMapIndex = _SRC.LargeMapIndex = -1;
    if (!_SRC.BigOldVersion)
    {
      unsigned int MinZoomOut = 0xFFFFFFFF, MaxZoomOut = 0;
      unsigned int Flags = _SRC.IdxInfo[3];
      for (int j = 0; j < 4; j++) _SRC.MapBigBlockOffsets[j] = _SRC.BigBlockOffsets + _SRC.IdxInfo[2];
      for (int i = 0; i < 4; i++)
      {
        unsigned int MapFlags = (Flags >> (i * 4));
        unsigned int MapZoomOut = 1 << ((MapFlags >> 1) & 0x7);

        if (MapFlags & 0x1)
        {
          if (MaxZoomOut < MapZoomOut)
            MaxZoomOut = MapZoomOut, _SRC.SmallMapIndex = i;
          if (MinZoomOut > MapZoomOut)
            MinZoomOut = MapZoomOut, _SRC.LargeMapIndex = i;
          _SRC.MapWidths[i] = _SRC.BigPixelWidth / MapZoomOut + ((_SRC.BigPixelWidth % MapZoomOut) ? 1 : 0);
          _SRC.MapHeights[i] = _SRC.BigPixelHeight / MapZoomOut + ((_SRC.BigPixelHeight % MapZoomOut) ? 1 : 0);
          if (i)
          {
            int MapBigBlockCount;
            switch (i)
            {
              case 1: MapBigBlockCount = 35;
              break;
              case 2: MapBigBlockCount = 48;
              break;
              case 3: MapBigBlockCount = 80;
              break;
            }
            for (int j = 0; j < i; j++) _SRC.MapBigBlockOffsets[j] += MapBigBlockCount;
          }
        }
        else _SRC.MapBigBlockOffsets[i] = 0;
      }
    }

    memset(_SRC.SortedLayers, 0xFF, sizeof(_SRC.SortedLayers));
    _SRC.AllLayers = 0;
    for (int i = 0; i < _SRC.LayerCount; i++)
    {
      int Priority = (((char *)(_SRC.IdxInfo + i * 4 + 5))[0] & 0xF) ^ 0x8;
      int Index = Priority * MaxLayerCount + MaxLayerCount - i - 1;
      _SRC.Layers[i].Style = (char)(((char *)(_SRC.IdxInfo + i * 4 + 5))[0] >> 4);
      _SRC.Layers[i].UnicColor = ((char *)(_SRC.IdxInfo + i * 4 + 5))[1];
      _SRC.SortedLayers[Index] = i;
      _SRC.Priorities[i] = Priority;
      _SRC.AllLayers <<= 1;
      _SRC.AllLayers++;
    }

    #ifdef _DEBUG_MODE
    fprintf(LogFile, "\n************ System Call *************");
    #endif
    _SRC.UnicHandle = OpenedSourceCount++;
    _SRC.SrcOpened = true;
    SetLayers(Handle, _SRC.AllLayers);
    #ifdef _DEBUG_MODE
    fprintf(LogFile, "\n************ System Call *************");
    #endif

    *_Handle = Handle;
		RC = rcOk;

    #ifdef _DEBUG_MODE
    fprintf(LogFile, "\n*ptr1 = %ld\n", *_Handle);
    PrintVars();
    PrintOk();
    #endif
    }
  catch (...) {
    if (_SRC.IdxInfo != NULL)
    {
      delete[] _SRC.IdxInfo;
      _SRC.IdxInfo = NULL;
    }
    if (Ini != NULL)
    {
      delete Ini;
      Ini = NULL;
    }

	#ifndef _WIN32_WCE
		if (IdxFile != -1)
	#else
		if (IdxFile != INVALID_HANDLE_VALUE)
	#endif
    {
      FileClose(IdxFile);
	#ifndef _WIN32_WCE
		IdxFile = -1;
	#else
		IdxFile =  INVALID_HANDLE_VALUE;
	#endif
    }

	#ifndef _WIN32_WCE 
	if (_SRC.BigFile != -1)
	#else
	if (_SRC.BigFile != INVALID_HANDLE_VALUE)
	#endif
    {
      FileClose(_SRC.BigFile);
      #ifndef _WIN32_WCE 
		_SRC.BigFile = -1;
	  #else
		_SRC.BigFile = INVALID_HANDLE_VALUE;
	  #endif
    }

	_SRC.SrcOpened = false;
	}
  LeaveCriticalSection(&Lock);

  return RC;
}
//---------------------------------------------------------------------------

int CloseSource(int Handle)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nCloseSource(%ld);", Handle);
  #endif

  _BASE_CHECK;

  FileClose(_SRC.BigFile);

	#ifndef _WIN32_WCE 
		_SRC.BigFile = -1;
	#else
		_SRC.BigFile = INVALID_HANDLE_VALUE;
	#endif

	delete[] _SRC.IdxInfo;
  _SRC.IdxInfo = NULL;
  _SRC.SrcOpened = false;

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetSourceSizes(int Handle, unsigned int *_BigPixelWidth, unsigned int *_BigPixelHeight)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetSourceSizes(%ld, ptr1, ptr2);", Handle);
  #endif

  _BASE_CHECK;

  *_BigPixelWidth = Div(_SRC.BigPixelWidth, _SRC.ZoomOut) + (Mod(_SRC.BigPixelWidth, _SRC.ZoomOut) ? 1 : 0);
  *_BigPixelHeight = Div(_SRC.BigPixelHeight, _SRC.ZoomOut) + (Mod(_SRC.BigPixelHeight, _SRC.ZoomOut) ? 1 : 0);

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr1 = %ld", *_BigPixelWidth);
  fprintf(LogFile, "\n*ptr2 = %ld\n", *_BigPixelHeight);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int SetZoomOut(int Handle, int _ZoomOut)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nSetZoomOut(%ld, %ld);", Handle, _ZoomOut);
  #endif

  _BASE_CHECK;

  if ((_ZoomOut != 1) && (_ZoomOut != 2) && (_ZoomOut != 4) && (_ZoomOut != 8) &&
    (_ZoomOut != -2))
  {
    LeaveCriticalSection(&Lock);
    return rcZoomNotImplemented;
  }
  _SRC.ZoomOut = _ZoomOut;

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetZoomOut(int Handle, int *_ZoomOut)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetZoomOut(%ld, ptr);", Handle);
  #endif

  _BASE_CHECK;

  *_ZoomOut = _SRC.ZoomOut;

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr = %ld\n", *_ZoomOut);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetImage2(int Handle, int X0, int Y0, BITMAP *Bitmap)
{

  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetImage(%ld, %ld, %ld, ptr);", Handle, X0, Y0);
  #endif

  _BASE_CHECK;

  int Bpp ;
  int NeedBpp = Bitmap->bmBitsPixel ;
  int NeedBytesPerPixel = NeedBpp >> 3 ;
  unsigned int Layers ;
  int LayersSaved = 0 ;
  PMixer Mixer ;
  int MixerSaved = 0 ;

  Bitmap->bmWidthBytes = Bitmap->bmWidth * ( NeedBpp >> 3 ) ;
  Bitmap->bmWidthBytes = Bitmap->bmWidthBytes & 0x03 ? ( Bitmap->bmWidthBytes & 0xFFFFFFFC ) + 4 : Bitmap->bmWidthBytes ;
  if (_SRC.Bpp == bpp888)
  {
    if ( NeedBpp == 8 ) Bpp = bpp332 ;
    else
      if ( NeedBpp == 16 || NeedBpp == 24 )
      {
        GetLayers ( Handle, & Layers ) ;
        LayersSaved = 1 ;
        Mixer = MixLayers ;
        MixLayers = Mixer2NoMMX ;
        MixerSaved = 1 ;
        Bpp = bpp888 ;
      }
      else
      {
        LeaveCriticalSection(&Lock);
        return rcBppNotImplemented;
      }
  }
  else Bpp = _SRC.Bpp ;

  int ZoomOut = _SRC.ZoomOut;
  int AbsZoom = abs(ZoomOut);

  X0 = Mul(X0, ZoomOut);
  Y0 = Mul(Y0, ZoomOut);
// not used now
//  int ImageRight = Bitmap->bmWidth - 1;
//  int ImageBottom = Bitmap->bmHeight - 1;
  int BigRight;
  int BigBottom;
  if (ZoomOut > 0)
  {
    BigRight = X0 + Bitmap->bmWidth * ZoomOut - 1;
    BigBottom = Y0 + Bitmap->bmHeight * ZoomOut - 1;
  }
  else {
    BigRight = X0 + Bitmap->bmWidth / AbsZoom + ((Bitmap->bmWidth % AbsZoom) ? 1 : 0) - 1;
    BigBottom = Y0 + Bitmap->bmHeight / AbsZoom + ((Bitmap->bmHeight % AbsZoom) ? 1 : 0) - 1;
  }
  int RowcharSize = Bitmap->bmWidthBytes;
  int LogicBlockSize = Mul(128, ZoomOut);
  unsigned char *BmpPtr ;
  unsigned int   BlockPtrOffs ;
  unsigned char *BlockPtr ;
  unsigned char *BlockImage;

  BmpPtr = (unsigned char *)(Bitmap->bmBits);
  memset(BmpPtr, 0xFF, RowcharSize * Bitmap->bmHeight);

  if ((X0 >= _SRC.BigPixelWidth) || (BigRight < 0) ||
    (Y0 >= _SRC.BigPixelHeight) || (BigBottom < 0))
  {
    // all empty
    if (LayersSaved) SetLayers ( Handle, Layers ) ;
    if (MixerSaved) MixLayers = Mixer ;
    LeaveCriticalSection(&Lock);
    return rcOk;
  }

  Cache->IncAccessTime();

  int LeftBlock = X0 / 128;
  int RightBlock = BigRight / 128;
  int TopBlock = Y0 / 128;
  int BottomBlock = BigBottom / 128;
  if (LeftBlock < 0) LeftBlock = 0;
  if (TopBlock < 0) TopBlock = 0;
  if (RightBlock >= _SRC.BigColCount) RightBlock = _SRC.BigColCount - 1;
  if (BottomBlock >= _SRC.BigRowCount) BottomBlock = _SRC.BigRowCount - 1;

  int X2=0, Y2=0, W, H;
  if (ZoomOut > 0)
  {
    for (int Row = TopBlock, BlockY = TopBlock * 128, Y1 = BlockY, Y2 = BlockY + LogicBlockSize - 1;
      Row <= BottomBlock; Row += ZoomOut, BlockY += LogicBlockSize, Y1 = BlockY, Y2 = Y1 + LogicBlockSize - 1)
    {
      if (Y1 < Y0) Y1 = Y0;
      if (Y2 > BigBottom) Y2 = BigBottom;

      for (int Col = LeftBlock, BlockX = LeftBlock * 128, X1 = BlockX, X2 = BlockX + LogicBlockSize - 1;
        Col <= RightBlock; Col += ZoomOut, BlockX += LogicBlockSize, X1 = BlockX, X2 = X1 + LogicBlockSize - 1)
      {
        if (X1 < X0) X1 = X0;
        if (X2 > BigRight) X2 = BigRight;

        BmpPtr = ((unsigned char *)(Bitmap->bmBits)) + (BigBottom - Y1) / ZoomOut * RowcharSize + ( (X1 - X0) / ZoomOut ) * NeedBytesPerPixel ;
        BlockPtrOffs = (Y1 - BlockY) / ZoomOut * 128 + (X1 - BlockX) / ZoomOut;
        W = (X2 - X1 + 1) / ZoomOut;
        H = (Y2 - Y1 + 1) / ZoomOut;

        try {
          if ( Bpp == bpp888 )
          {
            unsigned char * SavedBmpPtr = BmpPtr ;
            unsigned int  SavedBlockPtrOffs = BlockPtrOffs ;
            for ( int CC = 0, ICC = 2 ; CC < 3; CC ++, ICC -- )
            {
              BmpPtr = SavedBmpPtr ;
              BlockPtrOffs = SavedBlockPtrOffs ;
              SetLayers ( Handle, 1 << CC ) ;
              BlockImage = Cache->GetBigBlock(Handle, Row, Col, ZoomOut);
              if ( NeedBpp == 24 )
              {
                for (int Y = 0 ; Y < H ; Y ++, BlockPtrOffs += 128, BmpPtr -= RowcharSize )
                {
                  BlockPtr = BlockImage + BlockPtrOffs ;
                  for ( int X = 0, DX = ICC ; X < W ; X ++, DX += 3, BlockPtr ++ )
                    BmpPtr [ DX ] = * BlockPtr ;
                }
              }
              else
                if ( NeedBpp == 16 )
                {
                  if ( CC == 0 )
                  {
                    for (int Y = 0 ; Y < H ; Y ++, BlockPtrOffs += 128, BmpPtr -= RowcharSize )
                    {
                      BlockPtr = BlockImage + BlockPtrOffs ;
                      for ( int X = 0, DX = 1 ; X < W ; X ++, DX += 2, BlockPtr ++ )
                        BmpPtr [ DX ] = ( unsigned char ) ( ( * BlockPtr ) & 0xF8 );
                    }
                  }
                  else
                  if ( CC == 1 )
                  {
                    for (int Y = 0 ; Y < H ; Y ++, BlockPtrOffs += 128, BmpPtr -= RowcharSize )
                    {
                      BlockPtr = BlockImage + BlockPtrOffs ;
                      for ( int X = 0, DX = 0 ; X < W ; X ++, DX += 2, BlockPtr ++ )
                        BmpPtr [ DX ] = ( unsigned char ) ( ( ( ( * BlockPtr ) >> 2 )& 0x07 ) << 5 ),
                        BmpPtr [ DX + 1 ] |= ( unsigned char ) ( ( * BlockPtr ) >> 5 ) ;
                    }
                  }
                  else
                  if ( CC == 2 )
                  {
                    for (int Y = 0 ; Y < H ; Y ++, BlockPtrOffs += 128, BmpPtr -= RowcharSize )
                    {
                      BlockPtr = BlockImage + BlockPtrOffs ;
                      for ( int X = 0, DX = 0 ; X < W ; X ++, DX += 2, BlockPtr ++ )
                        BmpPtr [ DX ] |= ( unsigned char ) ( ( * BlockPtr ) >> 3 ) ;
                    }
                  }
                }
            }
          }
          else
          {
            BlockImage = Cache->GetBigBlock(Handle, Row, Col, ZoomOut);
            BlockPtr = BlockImage + BlockPtrOffs ;
            for (int Y = 0; Y < H; Y++, BmpPtr -= RowcharSize, BlockPtr += 128)
              memcpy(BmpPtr, BlockPtr, W);
          }
        }
        catch (...) {
          if (LayersSaved) SetLayers ( Handle, Layers ) ;
          if (MixerSaved) MixLayers = Mixer ;
          LeaveCriticalSection(&Lock);
          return Cache->GetLastError();
        }
      }
    }
  }
  else {
    for (int Row = TopBlock, BlockY = TopBlock * 128, Y1 = BlockY, Y2 = BlockY + 128 - 1;
      Row <= BottomBlock; Row += 1, BlockY += 128, Y1 = BlockY, Y2 = Y1 + 128 - 1)
    {
      if (Y1 < Y0) Y1 = Y0;
      if (Y2 > BigBottom) Y2 = BigBottom;

      for (int Col = LeftBlock, BlockX = LeftBlock * 128, X1 = BlockX, X2 = BlockX + 128 - 1;
        Col <= RightBlock; Col += 1, BlockX += 128, X1 = BlockX, X2 = X1 + 128 - 1)
      {
        if (X1 < X0) X1 = X0;
        if (X2 > BigRight) X2 = BigRight;

        BmpPtr = ((unsigned char *)(Bitmap->bmBits)) + (BigBottom - Y1) * AbsZoom * RowcharSize + (X1 - X0) * AbsZoom * NeedBytesPerPixel ;
        BlockPtrOffs = (Y1 - BlockY) * 128 + (X1 - BlockX);
        W = (X2 - X1 + 1);
        H = (Y2 - Y1 + 1);
        int RealW = W * AbsZoom;
        unsigned char *BaseBmpPtr;

        try {
          if ( Bpp == bpp888 )
          {
            unsigned char * SavedBmpPtr = BmpPtr ;
            unsigned int  SavedBlockPtrOffs = BlockPtrOffs ;
            for ( int CC = 0, ICC = 2 ; CC < 3; CC ++, ICC -- )
            {
              BmpPtr = SavedBmpPtr ;
              BlockPtrOffs = SavedBlockPtrOffs ;
              SetLayers ( Handle, 1 << CC ) ;
              BlockImage = Cache->GetBigBlock(Handle, Row, Col, 1);
              BlockPtr = BlockImage + BlockPtrOffs ;
              if ( NeedBpp == 24 )
              {
                BmpPtr += ICC ;
                for (int Y = 0; Y < H; Y++, BlockPtr += 128 - W)
                {
                  for (int X = 0; X < W; X++, BlockPtr++) *BmpPtr = *BlockPtr, BmpPtr+=3, *BmpPtr = *BlockPtr, BmpPtr+=3; //////// ZoomOut = -2
                  BmpPtr -= RowcharSize + RealW * NeedBytesPerPixel ;
                  for (int i = 1; i < AbsZoom; i++)
                    if (BmpPtr >= (unsigned char *)(Bitmap->bmBits))
                      BmpPtr -= RowcharSize;
                }
              }
              else
                if ( NeedBpp == 16 )
                {
                  if ( CC == 0 )
                  {
                    BmpPtr ++ ;
                    for (int Y = 0; Y < H; Y++, BlockPtr += 128 - W)
                    {
                      for (int X = 0; X < W; X++, BlockPtr++)
                      {
                        * BmpPtr = ( unsigned char )( ( * BlockPtr ) & 0xF8 ); BmpPtr+=2;
                        * BmpPtr = ( unsigned char )( ( * BlockPtr ) & 0xF8 ); BmpPtr+=2; //////// ZoomOut = -2
                      }
                      BmpPtr -= RowcharSize + RealW * NeedBytesPerPixel ;
                      for (int i = 1; i < AbsZoom; i++)
                        if (BmpPtr >= (unsigned char *)(Bitmap->bmBits))
                          BmpPtr -= RowcharSize;
                    }
                  }
                  else
                  if ( CC == 1 )
                  {
                    for (int Y = 0; Y < H; Y++, BlockPtr += 128 - W)
                    {
                      for (int X = 0; X < W; X++, BlockPtr++)
                      {
                        * BmpPtr = ( unsigned char ) ( ( ( ( * BlockPtr ) >> 2 )& 0x07 ) << 5 ); BmpPtr ++;
                        * BmpPtr |= ( unsigned char ) ( ( * BlockPtr ) >> 5 ); BmpPtr ++;
                        * BmpPtr = ( unsigned char ) ( ( ( ( * BlockPtr ) >> 2 )& 0x07 ) << 5 ); BmpPtr ++;
                        * BmpPtr |= ( unsigned char ) ( ( * BlockPtr ) >> 5 ); BmpPtr ++ ;
                      }
                      BmpPtr -= RowcharSize + RealW * NeedBytesPerPixel ;
                      for (int i = 1; i < AbsZoom; i++)
                        if (BmpPtr >= (unsigned char *)(Bitmap->bmBits))
                          BmpPtr -= RowcharSize;
                    }
                  }
                  else
                  if ( CC == 2 )
                  {
                    for (int Y = 0; Y < H; Y++, BlockPtr += 128 - W)
                    {
                      for (int X = 0; X < W; X++, BlockPtr++)
                      {
                        * BmpPtr |= ( unsigned char ) ( ( * BlockPtr ) >> 3 ); BmpPtr += 2;
                        * BmpPtr |= ( unsigned char ) ( ( * BlockPtr ) >> 3 ); BmpPtr += 2 ;
                      }
                      BmpPtr -= RowcharSize + RealW * NeedBytesPerPixel ;
                      for (int i = 1; i < AbsZoom; i++)
                        if (BmpPtr >= (unsigned char *)(Bitmap->bmBits))
                          BmpPtr -= RowcharSize;
                    }
                  }
                }
            }

            BmpPtr = SavedBmpPtr ;
            for (int Y = 0; Y < H; Y++, BlockPtr += 128 - W)
            {
              BaseBmpPtr = BmpPtr;
              for (int i = 1; i < AbsZoom; i++)
              {
                BmpPtr -= RowcharSize;
                if (BmpPtr >= (unsigned char *)(Bitmap->bmBits))
                {
                  memcpy(BmpPtr, BaseBmpPtr, RealW * NeedBytesPerPixel);
                  BmpPtr -= RowcharSize;
                }
              }
            }
          }
          else
          {
            BlockImage = Cache->GetBigBlock(Handle, Row, Col, 1);
            BlockPtr = BlockImage + BlockPtrOffs ;
            for (int Y = 0; Y < H; Y++, BlockPtr += 128 - W)
            {
              BaseBmpPtr = BmpPtr;
              for (int X = 0; X < W; X++, BlockPtr++) *BmpPtr = *BlockPtr, BmpPtr++, *BmpPtr = *BlockPtr, BmpPtr++; //////// ZoomOut = -2
              BmpPtr -= RowcharSize + RealW;
              for (int i = 1; i < AbsZoom; i++)
              {
                if (BmpPtr >= (unsigned char *)(Bitmap->bmBits))
                {
                  memcpy(BmpPtr, BaseBmpPtr, RealW);
                  BmpPtr -= RowcharSize;
                }
              }
            }
          }
        }
        catch (...) {
          if (LayersSaved) SetLayers ( Handle, Layers ) ;
          if (MixerSaved) MixLayers = Mixer ;
          LeaveCriticalSection(&Lock);
          return Cache->GetLastError();
        }

      }
    }
  }

  if (LayersSaved) SetLayers ( Handle, Layers ) ;
  if (MixerSaved) MixLayers = Mixer ;
  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}

int GetImage(int Handle, int X0, int Y0, BITMAP *Bitmap)
{
  Bitmap->bmBitsPixel = 8 ;
  return GetImage2 ( Handle, X0, Y0, Bitmap ) ;
}

//---------------------------------------------------------------------------
int GetStdImage(int Handle, int Row, int Col, char *AData)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetImage(%ld, %ld, %ld, ptr1, ptr2);", Handle, Row, Col);
  #endif

  #ifdef _MDEBUG_MODE
  DWORD st_time=GetTickCount();
  #endif

  _BASE_CHECK;

  unsigned char *Data = (unsigned char *)AData;
// not used now
//  int Error = rcOk;
  int ZoomOut = _SRC.ZoomOut;
  TBpp Bpp = _SRC.Bpp;
  PMixer Mixer ;
  int MixerSaved = 0 ;

  try
  {
    if (ZoomOut > 0)
    {
      if (Bpp != bpp888) Cache->GetFastBigLogicBlock(Handle, Row, Col, ZoomOut, Data);
      else
      {
        int SaveActiveSortedLayers[3];
        char SaveActiveLayerCount;
        unsigned int SaveActiveLayers;

        Mixer = MixLayers ;
        MixLayers = Mixer2NoMMX ;
        MixerSaved = 1 ;

        memcpy(SaveActiveSortedLayers, _SRC.ActiveSortedLayers + 1, 12);
        SaveActiveLayerCount = _SRC.ActiveLayerCount;
        SaveActiveLayers = _SRC.ActiveLayers;
        _SRC.ActiveLayerCount = 2;
        Data[0] = 0x00;
        Data[1] = 0x40;
        Data[2] = 0x00;
        Data[3] = 0x40;
        Data[4] = 0x00;
        Data[5] = 0x40;

        _SRC.ActiveLayers = 0x00000001;
        _SRC.ActiveSortedLayers[1] = 0;
        Cache->GetFastBigLogicBlock(Handle, Row, Col, ZoomOut, Data + 6);

        _SRC.ActiveLayers = 0x00000002;
        _SRC.ActiveSortedLayers[1] = 1;
        Cache->GetFastBigLogicBlock(Handle, Row, Col, ZoomOut, Data + 16384 + 6);

        _SRC.ActiveLayers = 0x00000004;
        _SRC.ActiveSortedLayers[1] = 2;
        Cache->GetFastBigLogicBlock(Handle, Row, Col, ZoomOut, Data + 32768 + 6);

        _SRC.ActiveLayers = SaveActiveLayers;
        _SRC.ActiveLayerCount = SaveActiveLayerCount;
        memcpy(_SRC.ActiveSortedLayers + 1, SaveActiveSortedLayers, 12);

        if (MixerSaved) MixLayers = Mixer, MixerSaved = 0 ;
      }
    }
    else {
	  LeaveCriticalSection(&Lock);
      return rcZoomNotImplemented;
    }
  }
  catch (...)
  {
    if (MixerSaved) MixLayers = Mixer, MixerSaved = 0 ;
    LeaveCriticalSection(&Lock);
    return Cache->GetLastError();
  }

  #ifdef _MDEBUG_MODE
  DWORD en_time=GetTickCount();
  fprintf(LogFile, "\nGetBlock speed(ms)=%ld", en_time-st_time);
  #endif

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nptr1[0] = %ld", (int)(Data[0]));
//  fprintf(LogFile, "\n*ptr2 = %ld", *DataSize);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

typedef struct {
  TLogPalette lpal;
  TPaletteEntry dummy[256];
} TLogPal;

HPALETTE CreatePalette332(void)
{
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nCreatePalette332();");
  #endif

	TLogPal Pal;
	int B, G, R;

	Pal.lpal.palVersion = 0x300;
	Pal.lpal.palNumEntries = 256;
	for (int i = 0; i < 256; i++)
	{
		R = i & 0xE0;
		if (R)
			R |= 0x1F;
		G = (i & 0x1C) << 3;
		if (G)
			G |= 0x1F;
		B = (i & 0x03) << 6;
		if (B)
			B |= 0x3F;

		Pal.lpal.palPalEntry[i].peRed = (char)R;
		Pal.lpal.palPalEntry[i].peGreen = (char)G;
		Pal.lpal.palPalEntry[i].peBlue = (char)B;
		Pal.lpal.palPalEntry[i].peFlags = (char)0;
	}

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
	return CreatePalette(&(Pal.lpal));
}
//---------------------------------------------------------------------------

HPALETTE CreateGrayscalePalette(void)
{
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nCreateGrayscalePalette();");
  #endif

	TLogPal Pal;

	Pal.lpal.palVersion = 0x300;
	Pal.lpal.palNumEntries = 256;
	for (int i = 0; i < 256; i++)
	{
		Pal.lpal.palPalEntry[i].peRed = (char)i;
		Pal.lpal.palPalEntry[i].peGreen = (char)i;
		Pal.lpal.palPalEntry[i].peBlue = (char)i;
		Pal.lpal.palPalEntry[i].peFlags = (char)0;
	}

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
	return CreatePalette(&(Pal.lpal));
}
//---------------------------------------------------------------------------

HPALETTE CreateSpecPalette(int Handle)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nCreateSpecPalette();");
  #endif

	TLogPal Pal;

	Pal.lpal.palVersion = 0x300;
	Pal.lpal.palNumEntries = 256;

	for (int i = 0; i < _SRC.ColorsCount; i++)
	{
    // specpal by Handle
		Pal.lpal.palPalEntry[i].peRed = _SRC.ColorTable[i * 3];
		Pal.lpal.palPalEntry[i].peGreen = _SRC.ColorTable[i * 3 + 1];
		Pal.lpal.palPalEntry[i].peBlue = _SRC.ColorTable[i * 3 + 2];
		Pal.lpal.palPalEntry[i].peFlags = (char)0;
	}
	for (int i = _SRC.ColorsCount; i < 256; i++)
	{
    // fill other colors by zeros
		Pal.lpal.palPalEntry[i].peRed = 0;
		Pal.lpal.palPalEntry[i].peGreen = 0;
		Pal.lpal.palPalEntry[i].peBlue = 0;
		Pal.lpal.palPalEntry[i].peFlags = (char)0;
	}

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
	return CreatePalette(&(Pal.lpal));
}
//---------------------------------------------------------------------------

int GetPaletteData(int Handle, unsigned char *ColorTable)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetPaletteData();");
  #endif

  switch (_SRC.Bpp)
  {
    case bppAnyPal8:
		{  
		int i;
		for (i = 0; i < _SRC.ColorsCount; i++)
			{
			// specpal by Handle
			ColorTable[i * 4] = _SRC.ColorTable[i * 3];
			ColorTable[i * 4 + 1] = _SRC.ColorTable[i * 3 + 1];
			ColorTable[i * 4 + 2] = _SRC.ColorTable[i * 3 + 2];
			ColorTable[i * 4 + 3] = 0;
			}
		for (i = _SRC.ColorsCount; i < 256; i++)
			{
			// fill other colors by zeros
			ColorTable[i * 4] = 0;
			ColorTable[i * 4 + 1] = 0;
			ColorTable[i * 4 + 2] = 0;
			ColorTable[i * 4 + 3] = 0;
			}
		break;
		}

    case bpp332:
    case bpp888:
		{
		int R, G, B;
		for (int i = 0; i < 256; i++)
			{
			R = i & 0xE0;
			if (R)
				R |= 0x1F;
			G = (i & 0x1C) << 3;
			if (G)
				G |= 0x1F;
			B = (i & 0x03) << 6;
			if (B)
				B |= 0x3F;

	        ColorTable[i * 4] = (unsigned char)R;
			ColorTable[i * 4 + 1] = (unsigned char)G;
			ColorTable[i * 4 + 2] = (unsigned char)B;
			ColorTable[i * 4 + 3] = (unsigned char)0;
			}
		break;
		}
    case bppGrayscale8:
		{
			for (int i = 0; i < 256; i++)
				{
				ColorTable[i * 4] = (unsigned char)i;
				ColorTable[i * 4 + 1] = (unsigned char)i;
				ColorTable[i * 4 + 2] = (unsigned char)i;
				ColorTable[i * 4 + 3] = (unsigned char)0;
				}
		break;
		}
    default:
      LeaveCriticalSection(&Lock);
      return rcFatalError;
  }

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetPalette(int Handle, HPALETTE *HPalette)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetPalette(%ld, ptr);", Handle);
  #endif

  _BASE_CHECK;

  switch (_SRC.Bpp)
  {
    case bppAnyPal8:
      *HPalette = CreateSpecPalette(Handle);
      break;
    case bpp332:
      *HPalette = CreatePalette332();
      break;
    case bppGrayscale8:
      *HPalette = CreateGrayscalePalette();
      break;
    case bpp888:
      *HPalette = CreatePalette332(); // ��� ��������
      break;
    default:
    LeaveCriticalSection(&Lock);
    return rcFatalError;
  }

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr = %ld", (DWORD)(*HPalette));
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}

//---------------------------------------------------------------------------

int GetColorStruct(int Handle, unsigned char *Type)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetPalette(%ld, ptr);", Handle);
  #endif

  _BASE_CHECK;

  switch (_SRC.Bpp)
  {
    case bpp332:
      *Type = 0x01;
    break;
    case bppGrayscale8:
      *Type = 0x02;
    break;
    case bpp888:
      *Type = 0x03;
    break;
    case bppAnyPal8:
      *Type = 0x04;
    break;
    default:
    LeaveCriticalSection(&Lock);
    return rcFatalError;
  }

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr = %ld", (DWORD)(*Type));
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}

//---------------------------------------------------------------------------

int GetColorsNum(int Handle, int *ColorsNum)
{
	EnterCriticalSection(&Lock);
	#ifdef _DEBUG_MODE
	fprintf(LogFile, "\nGetColorsNum(%ld, ptr);", Handle);
	#endif

	_BASE_CHECK;

	switch (_SRC.Bpp)
	{
		case bpp332:
			*ColorsNum = 256;
		break;
		case bppGrayscale8:
			*ColorsNum = 256;
		break;
		case bpp888:
			*ColorsNum = 256*256*256;
		break;
		case bppAnyPal8:
			*ColorsNum = _SRC.ColorsCount;
		break;
		default:
		LeaveCriticalSection(&Lock);
		return rcFatalError;
	}

	#ifdef _DEBUG_MODE
	fprintf(LogFile, "\n*ptr = %ld", (DWORD)(*ColorsNum));
	PrintVars();
	PrintOk();
	#endif
	LeaveCriticalSection(&Lock);
	return rcOk;
}
//---------------------------------------------------------------------------

int GetBaseCoords(int Handle, int X, int Y, double *BaseX, double *BaseY)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetBaseCoords(%ld, %ld, %ld, ptr1, ptr2);", Handle, X, Y);
  #endif

  _BASE_CHECK;
  if (!_SRC.IniOpened)
  {
    LeaveCriticalSection(&Lock);
    return rcIniNotOpened;
  }

  X = Mul(X, _SRC.ZoomOut);
  Y = Mul(Y, _SRC.ZoomOut);
  *BaseX = _SRC.Scale * (X - _SRC.XOffset);
  *BaseY = _SRC.Scale * (_SRC.BigPixelHeight - Y - 1 - _SRC.YOffset);

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr1 = %lf", *BaseX);
  fprintf(LogFile, "\n*ptr2 = %lf\n", *BaseY);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetPixelCoords(int Handle, int *X, int *Y, double BaseX, double BaseY)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetPixelCoords(%ld, ptr1, ptr2, %lf, %lf);", Handle, BaseX, BaseY);
  #endif

  _BASE_CHECK;
  if (!_SRC.IniOpened)
  {
    LeaveCriticalSection(&Lock);
    return rcIniNotOpened;
  }

  *X = Div(((int)(BaseX / _SRC.Scale + _SRC.XOffset)), _SRC.ZoomOut);
  *Y = Div(((int)(_SRC.BigPixelHeight - 1 - _SRC.YOffset - BaseY / _SRC.Scale)), _SRC.ZoomOut);

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr1 = %ld", *X);
  fprintf(LogFile, "\n*ptr2 = %ld\n", *Y);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetSmallMapSizes(int Handle, unsigned int *_Width, unsigned int *_Height)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetSmallMapSizes(%ld, ptr1, ptr2);", Handle);
  #endif

  _BASE_CHECK;
  if (_SRC.SmallMapIndex == -1)
  {
    LeaveCriticalSection(&Lock);
    return rcNoMapPresent;
  }

  *_Width = _SRC.MapWidths[_SRC.SmallMapIndex];
  *_Height = _SRC.MapHeights[_SRC.SmallMapIndex];

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr1 = %ld", *_Width);
  fprintf(LogFile, "\n*ptr2 = %ld\n", *_Height);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetLargeMapSizes(int Handle, unsigned int *_Width, unsigned int *_Height)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetLargeMapSizes(%ld, ptr1, ptr2);", Handle);
  #endif

  _BASE_CHECK;
  if (_SRC.LargeMapIndex == -1)
  {
    LeaveCriticalSection(&Lock);
    return rcNoMapPresent;
  }

  *_Width = _SRC.MapWidths[_SRC.LargeMapIndex];
  *_Height = _SRC.MapHeights[_SRC.LargeMapIndex];

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr1 = %ld", *_Width);
  fprintf(LogFile, "\n*ptr2 = %ld\n", *_Height);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetMap(int Handle, BITMAP *Bitmap, int MapIndex)
{
  int BlockWidth, BlockHeight;
  unsigned int *BlockOffset;
  unsigned char BlockImage[128 * 128];
	unsigned char *BmpPtr;
  unsigned char *BlockPtr;
	int RowcharSize = Bitmap->bmWidthBytes;

  for (unsigned int Top = 0, i = 0; Top < _SRC.MapHeights[MapIndex]; Top += 128, i++)
  {
    BlockOffset = _SRC.MapBigBlockOffsets[MapIndex] + i * MapBigBlockPerRow[MapIndex];
    BlockHeight = _SRC.MapHeights[MapIndex] - Top;
    if (BlockHeight > 128) BlockHeight = 128;
    for (unsigned int Left = 0; Left < _SRC.MapWidths[MapIndex]; Left += 128)
    {
      BlockWidth = _SRC.MapWidths[MapIndex] - Left;
      if (BlockWidth > 128) BlockWidth = 128;

      try {
		  Cache->ExtractMapBlock(Handle, BlockOffset, BlockImage);
      }
      catch (...) {
        return Cache->GetLastError();
      }

      BmpPtr = ((unsigned char *)(Bitmap->bmBits)) + (_SRC.MapHeights[MapIndex] - 1 - Top) * RowcharSize + Left;
      BlockPtr = BlockImage;
      for (int Y = 0; Y < BlockHeight; Y++, BmpPtr -= RowcharSize, BlockPtr += 128)
        memcpy(BmpPtr, BlockPtr, BlockWidth);

      BlockOffset++;
    }
  }

  return rcOk;
}
//---------------------------------------------------------------------------

int GetSmallMap(int Handle, BITMAP *Bitmap)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetSmallMap(%ld, ptr);", Handle);
  #endif

  _BASE_CHECK;
  if (_SRC.SmallMapIndex == -1)
  {
    LeaveCriticalSection(&Lock);
    return rcNoMapPresent;
  }

  int RC = GetMap(Handle, Bitmap, _SRC.SmallMapIndex);
  #ifdef _DEBUG_MODE
  PrintVars();
  if (RC == 0) PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return RC;
}
//---------------------------------------------------------------------------

int GetLargeMap(int Handle, BITMAP *Bitmap)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetLargeMap(%ld, ptr);", Handle);
  #endif

  _BASE_CHECK;
  if (_SRC.LargeMapIndex == -1)
  {
    LeaveCriticalSection(&Lock);
    return rcNoMapPresent;
  }

  int RC = GetMap(Handle, Bitmap, _SRC.LargeMapIndex);
  #ifdef _DEBUG_MODE
  PrintVars();
  if (RC == 0) PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return RC;
}
//---------------------------------------------------------------------------

int GetLargeMapBlock(int Handle, int Row, int Col, char *PackedImage)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetMapBlock(%ld, %ld, %ld, ptr);", Handle, Row, Col);
  #endif

  _BASE_CHECK;
  if (_SRC.LargeMapIndex == -1)
  {
    LeaveCriticalSection(&Lock);
    return rcNoMapPresent;
  }

  unsigned int *BlockOffset = _SRC.MapBigBlockOffsets[_SRC.LargeMapIndex] +
    Row * MapBigBlockPerRow[_SRC.LargeMapIndex] + Col;
  int PackedSize = BlockOffset[1] - BlockOffset[0];

  if (PackedSize > MaxBigBlockSize) PackedSize = MaxBigBlockSize;
  FileSeek(_SRC.BigFile, (int)(BlockOffset[0]), 0);
  if (FileRead(_SRC.BigFile, PackedImage, PackedSize) != PackedSize)
  {
    LeaveCriticalSection(&Lock);
    return rcDiskError;
  }

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetLargeMapBlockSize(int Handle, int Row, int Col, int *PackedSize)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetMapBlockSize(%ld, %ld, %ld, ptr);", Handle, Row, Col);
  #endif

  _BASE_CHECK;
  if (_SRC.LargeMapIndex == -1)
  {
    LeaveCriticalSection(&Lock);
    return rcNoMapPresent;
  }

  unsigned int *BlockOffset = _SRC.MapBigBlockOffsets[_SRC.LargeMapIndex] +
    Row * MapBigBlockPerRow[_SRC.LargeMapIndex] + Col;
  *PackedSize = BlockOffset[1] - BlockOffset[0];
  if ((*PackedSize) > MaxBigBlockSize) *PackedSize = MaxBigBlockSize;

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr1 = %ld", *PackedSize);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetLayersList(int Handle, int *_LayerCount, char *LayerNames)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetLayersList(%ld, ptr1, ptr2);", Handle);
  #endif

  _BASE_CHECK;

  for (int i = 0; i < _SRC.LayerCount; i++)
  {
    memcpy(LayerNames + i * LayerNameLen, ((char *)(_SRC.IdxInfo + i * 4 + 5)) + 2, 14);
    LayerNames[i * LayerNameLen + 14] = 0;
  }
  *_LayerCount = _SRC.LayerCount;

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr1 = %ld", *_LayerCount);
  fprintf(LogFile, "\n*ptr2 = '");
  for (int i = 0; i < *_LayerCount * LayerNameLen; i++)
    fprintf(LogFile, "%c", LayerNames[i]);
  fprintf(LogFile, "'\n");
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetLayersPriority(int Handle, int *_Priorities)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetLayersPriorities(%ld, ptr1);", Handle);
  #endif

  _BASE_CHECK;

  memcpy(_Priorities, _SRC.Priorities, _SRC.LayerCount*sizeof(int));

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr1 = ");
  for (int i = 0; i < _SRC.LayerCount; i++)
    fprintf(LogFile, "%ld ", _Priorities[i]);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetLayerMixInfo(int Handle, int LayerIndex, char *Style, char *UnicColor)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetLayerMixInfo(%ld, %ld, ptr1, ptr2);", Handle, LayerIndex);
  #endif

  _BASE_CHECK;

  *Style = _SRC.Layers[LayerIndex].Style;
  *UnicColor = _SRC.Layers[LayerIndex].UnicColor;

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr1 = %ld", (int)(*Style));
  fprintf(LogFile, "\n*ptr2 = %ld", (int)(*UnicColor));
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int MixTwoLayers(unsigned char *Dest, unsigned char *Src, char Style, char UnicColor)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetLayerMixInfo(ptr1, ptr2, %ld, %ld);", (int)Style, (int)UnicColor);
  #endif

  if (!SystemInited)
  {
    LeaveCriticalSection(&Lock);
    return rcSystemNotInited;
  }

  MixLayers(Dest, Src, Style, UnicColor);

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int SetLayers(int Handle, unsigned int Layers)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nSetLayers(%ld, %ld);", Handle, Layers);
  #endif

  _BASE_CHECK;

  if (Layers >= ((unsigned int)1 << _SRC.LayerCount))
  {
    LeaveCriticalSection(&Lock);
    return rcLayerNotPresent;
  }
  _SRC.ActiveLayers = Layers;
  _SRC.ActiveLayerCount = 0;
  if (_SRC.LayerCount)
  {
    for (int i = 0; i < MaxPriorityCount * MaxLayerCount; i++)
    {
      if (i == 8 * MaxLayerCount + MaxLayerCount - 1)
        _SRC.ActiveSortedLayers[_SRC.ActiveLayerCount++] = -1;
      if ((_SRC.SortedLayers[i] != -1) && (Layers & (1 << _SRC.SortedLayers[i])))
        _SRC.ActiveSortedLayers[_SRC.ActiveLayerCount++] = _SRC.SortedLayers[i];
    }
  }
  else _SRC.ActiveSortedLayers[_SRC.ActiveLayerCount++] = -1;

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int GetLayers(int Handle, unsigned int *Layers)
{
  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nGetLayers(%ld, ptr);", Handle);
  #endif

  _BASE_CHECK;
  *Layers = _SRC.ActiveLayers;

  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\n*ptr = %ld\n", *Layers);
  PrintVars();
  PrintOk();
  #endif
  LeaveCriticalSection(&Lock);
  return rcOk;
}
//---------------------------------------------------------------------------

int UnpackBlock(char *Dst, char *Src, int PackedSize)
{
  return UnpackBlock((unsigned char *)Dst, (unsigned char *)Src, PackedSize);
}
//---------------------------------------------------------------------------

int UnpackBlock(unsigned char *Dst, unsigned char *Src, int PackedSize)
{
//  EnterCriticalSection(&Lock);
  #ifdef _DEBUG_MODE
  fprintf(LogFile, "\nUnpackBlock(ptr1, ptr2, %ld);", PackedSize);
  #endif

  DWORD st_time=GetTickCount();

  if (decode(Src, Dst, (short int)PackedSize))
  {
//    LeaveCriticalSection(&Lock);
  #ifdef _MDEBUG_MODE
  fprintf(LogFile, "\nfinish unpack error");
  #endif
    return rcUnpackError;
  }

  #ifdef _DEBUG_MODE
  PrintVars();
  PrintOk();
  #endif

//  LeaveCriticalSection(&Lock);
  #ifdef _MDEBUG_MODE
  DWORD en_time=GetTickCount();
  fprintf(LogFile, "\nfinish unpack speed(ms)=%ld", en_time-st_time);
  #endif

  return rcOk;
}
//---------------------------------------------------------------------------