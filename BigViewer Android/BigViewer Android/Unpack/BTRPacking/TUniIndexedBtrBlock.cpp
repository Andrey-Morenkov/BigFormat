// TUniIndexedBtrBlock.cpp: implementation of the TUniIndexedBtrBlock class.
//
//////////////////////////////////////////////////////////////////////
//#include <stdafx.h>

#include "TUniIndexedBtrBlock.h"
#include <math.h>
#include <memory.h>

//////////////////////////////////////////////////////////////////////
// Internal usage definitions and functions
//////////////////////////////////////////////////////////////////////

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TUniIndexedBtrBlock::TUniIndexedBtrBlock(_TAnyCoder *_Coder, int LogSize, bool Flat, int _Layers):
	TBtrBlock(_Coder, LogSize, Flat, _Layers),SimpleModel(Coder)
{
	ErrModel = new TUNIErrorModel[136];

	for (int m = 0; m < 136; m ++)
		ErrModel[m].SetCoder(_Coder);
	
	int fem = 0,i;
	for (i = 1; i < 16; i ++)
  	for (int j = 1; j <= i; j ++)
    	ErrModelIdx[i][j] = ErrModelIdx[j][i] = fem ++;
  for (i = 1; i < 16; i ++)
  	ErrModelIdx[i][0] = ErrModelIdx[0][i] = fem ++;
  ErrModelIdx[0][0] = fem;
}

TUniIndexedBtrBlock::~TUniIndexedBtrBlock()
{
	delete[] ErrModel;
}

//////////////////////////////////////////////////////////////////////
// External usage methods
//////////////////////////////////////////////////////////////////////

int TUniIndexedBtrBlock::Unpack(uint8 *Points, int UseLevels)
{
	try
  {
    CHECK_USER(((UseLevels <= MaxTreeLevel) && (UseLevels > 1)),
      "Ошибка в параметре UseLevels");

    Coder->StartDecode();

		SimpleModel.Clear();
    for (int m = 0; m < 136; m ++)
      ErrModel[m].Clear();

    int FullTreeLevels = SimpleModel.Decode(); // неважно, т.к. err = hierr = 0 всегда
    if (FullTreeLevels < UseLevels)
      UseLevels = FullTreeLevels;
    int err = SimpleModel.Decode(); // макс. ошибка
    int hierr = SimpleModel.Decode(); // ошибка на верхних уровнях (1 .. MaxTreeLevel - 2)
    Points[FirstOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextIncOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextRowOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]] =
      SimpleModel.Decode();

	int i;
    for (i = 0; i < sizeof(GoodColors); i ++)
      GoodColors[i] = SimpleModel.Decode();
    memset(ColorIdx, 0, sizeof(ColorIdx));
    for (i = 0; i < 15; i ++)
      ColorIdx[GoodColors[i]] = i + 1;

    int posinc, rowinc, rowsize, rowcount, arclen, p, row, rpos, level;

    // Распакуем первые UseLevels уровней
    for (level = 1; level <= UseLevels; level ++)
    {
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];

      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
          Points[p] =
            ErrModel[ErrModelIdx[ColorIdx[Points[p - arclen]]][ColorIdx[Points[p + arclen]]]].
              Decode();

          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row
    } // for level

    // Оставшиеся уровни просто проинтерполируем
    for (level = UseLevels + 1; level <= MaxTreeLevel; level ++)
    {
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];

      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
          Points[p] = Points[p - arclen];
          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row
    } // for level

    Coder->FinishDecode();
    return Coder->GetPassed();
  }
  catch(...)
  {
  	return -1;
  }
}


int TUniIndexedBtrBlock::Pack(int err, int hierr,	uint8 *Points,	int *LevelSizes)
{
	try
  {
    CHECK_USER(((err == 0) && (hierr == 0)),
      "Ошибка для индексированных данных не поддерживается");

    Coder->StartEncode();

    SimpleModel.Clear();
    for (int m = 0; m < 136; m ++)
      ErrModel[m].Clear();

    SimpleModel.Encode(MaxTreeLevel);
    SimpleModel.Encode(err);       // макс. ошибка
    SimpleModel.Encode(hierr);     // ошибка на верхних уровнях (1 .. MaxTreeLevel - 2)
    SimpleModel.Encode(Points[FirstOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextIncOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]]);

    memset(ColorIdx, 0, sizeof(ColorIdx));
    memset(ColorCount, 0, sizeof(ColorCount));
    
	int i;
	for (i = 0; i < PointsBufSize; i++)
      ColorCount[Points[i]] ++;
    for (i = 0; i < 15; i ++)
    {
      GoodColors[i] = 0;
      for (int j = 0; j < 256; j ++)
        if (ColorCount[j] > ColorCount[GoodColors[i]])
          GoodColors[i] = j;
      ColorCount[GoodColors[i]] = -1;
      ColorIdx[GoodColors[i]] = i + 1;
    }
    for (i = 0; i < sizeof(GoodColors); i ++)
      SimpleModel.Encode(GoodColors[i]);

    int posinc, rowinc, rowsize, rowcount, arclen, p, row, rpos;

    for (int level = 1; level <= MaxTreeLevel; level ++)
    {
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];

      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
          ErrModel[ErrModelIdx[ColorIdx[Points[p - arclen]]][ColorIdx[Points[p + arclen]]]].
            Encode(Points[p]);

          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row

      if (LevelSizes != NULL)
        LevelSizes[level] = Coder->GetPassed() + 4;
    } // for level

    Coder->FinishEncode();
    return Coder->GetPassed();
  }
  catch(...)
  {
  	return -1;
  }
}

