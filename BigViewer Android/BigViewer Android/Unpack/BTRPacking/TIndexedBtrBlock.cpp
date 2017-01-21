// TIndexedBtrBlock.cpp: implementation of the TIndexedBtrBlock class.
//
//////////////////////////////////////////////////////////////////////
//#include <stdafx.h>

#include "TIndexedBtrBlock.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Internal usage definitions and functions
//////////////////////////////////////////////////////////////////////

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TIndexedBtrBlock::TIndexedBtrBlock(_TAnyCoder *_Coder, int LogSize, bool Flat, int _Layers):
	TBtrBlock(_Coder, LogSize, Flat, _Layers),SimpleModel(Coder)
{
	ErrModel = new TIDXErrorModel[256];

	for (int i = 0; i < 256; i ++)
		ErrModel[i].SetCoder(_Coder);
}

TIndexedBtrBlock::~TIndexedBtrBlock()
{
	delete[] ErrModel;
}

//////////////////////////////////////////////////////////////////////
// External usage methods
//////////////////////////////////////////////////////////////////////

int TIndexedBtrBlock::Unpack(uint8 *Points, int UseLevels)
{
	try
  {
    CHECK_USER(((UseLevels <= MaxTreeLevel) && (UseLevels > 1)),
      "������ � ��������� UseLevels");

    Coder->StartDecode();

    SimpleModel.Clear();
		for (int m = 0; m < 256; m ++)
      ErrModel[m].Clear();

    int FullTreeLevels = SimpleModel.Decode(); // �������, �.�. err = hierr = 0 ������
    if (FullTreeLevels < UseLevels)
      UseLevels = FullTreeLevels;
    int err = SimpleModel.Decode(); // ����. ������
    int hierr = SimpleModel.Decode(); // ������ �� ������� ������� (1 .. MaxTreeLevel - 2)
    Points[FirstOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextIncOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextRowOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]] =
      SimpleModel.Decode();


    int posinc, rowinc, rowsize, rowcount, arclen, p, row, rpos, level;

    // ��������� ������ UseLevels �������
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
            ErrModel[(Points[p - arclen] + Points[p + arclen]) >> 1].Decode();

          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row
    } // for level

    // ���������� ������ ������ ����������������
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


int TIndexedBtrBlock::Pack(int err, int hierr, uint8 *Points, int *LevelSizes)
{
	try
  {
    CHECK_USER(((err == 0) && (hierr == 0)),
      "����������� � �������� ��� ��������������� ������ �� ��������������");

    Coder->StartEncode();

		SimpleModel.Clear();
    for (int m = 0; m < 256; m ++)
      ErrModel[m].Clear();

    SimpleModel.Encode(MaxTreeLevel);
    SimpleModel.Encode(err);       // ����. ������
    SimpleModel.Encode(hierr);     // ������ �� ������� ������� (1 .. MaxTreeLevel - 2)
    SimpleModel.Encode(Points[FirstOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextIncOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]]);

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
          ErrModel[(Points[p - arclen] + Points[p + arclen]) >> 1].Encode(Points[p]);

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

