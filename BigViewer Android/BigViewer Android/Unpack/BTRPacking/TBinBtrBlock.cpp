// TIndexedBtrBlock.cpp: implementation of the TIndexedBtrBlock class.
//
//////////////////////////////////////////////////////////////////////
//#include <stdafx.h>

#include "TBinBtrBlock.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Internal usage definitions and functions
//////////////////////////////////////////////////////////////////////

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))

//#define INTER 0
//#define INTER (((Points[p - arclen] & 0x01) << 1) + (Points[p + arclen] & 0x01))
#define INTER \
		( ((! Flat) && (! (level % 2)) && (rpos > 0) && (rpos < (rowsize - 1))) ? \
		( ((Points[p - arclen] & 0x01) << 4) + ((Points[p + arclen] & 0x01) << 3) + \
    ((Points[p - posinc] & 0x01) << 2) + \
    ((Points[p + posinc + arclen] & 0x01) << 1) + \
    ((Points[p + posinc - arclen] & 0x01) << 0) ) + 4 \
		: \
    (((Points[p - arclen] & 0x01) << 1) + (Points[p + arclen] & 0x01)) )
#define CONTNUM 40
/*#define INTER \
		( ((! Flat) && (! (level % 2)) && (rpos > 0) && (rpos < (rowsize - 1))) ? \
		( ((Points[p - arclen] & 0x01) << 4) + ((Points[p + arclen] & 0x01) << 3) + \
    ((Points[p - posinc] & 0x01) << 2) + \
    ((Points[p + posinc + arclen] & 0x01) << 1) + \
    ((Points[p + posinc - arclen] & 0x01) << 0) ) + 4 \
    + 32 * (level > 12 ? 0 : (level > 10 ? 1 : 2)) \
		: \
    (((Points[p - arclen] & 0x01) << 1) + (Points[p + arclen] & 0x01)) )
#define CONTNUM 270*/
/*#define INTER \
		( ((! Flat) && (! (level % 2)) && (rpos > 0) && (rpos < (rowsize - 1))) ? \
		( (9 * (Points[p - arclen] & 0x01)) + (9 * (Points[p + arclen] & 0x01)) + \
    (10 * (Points[p - posinc] & 0x01)) + \
    (2 * (Points[p + posinc + arclen] & 0x01)) + \
    (2 * (Points[p + posinc - arclen] & 0x01)) ) / 4 + 4 \
		: \
    (((Points[p - arclen] & 0x01) << 1) + (Points[p + arclen] & 0x01)) )
*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TBinBtrBlock::TBinBtrBlock(_TAnyCoder *_Coder, int LogSize, bool Flat, int _Layers):
	TBtrBlock(_Coder, LogSize, Flat, _Layers),SimpleModel(Coder)
{
	ErrModel = new TBinErrorModel[CONTNUM];

	for (int i = 0; i < CONTNUM; i ++)
		ErrModel[i].SetCoder(_Coder);
}

TBinBtrBlock::~TBinBtrBlock()
{
	delete[] ErrModel;
}

//////////////////////////////////////////////////////////////////////
// External usage methods
//////////////////////////////////////////////////////////////////////

int TBinBtrBlock::Unpack(uint8 *Points, int UseLevels)
{
	try
  {
    CHECK_USER(((UseLevels <= MaxTreeLevel) && (UseLevels > 1)),
      "������ � ��������� UseLevels");

    Coder->StartDecode();

    SimpleModel.Clear();
		for (int m = 0; m < CONTNUM; m ++)
      ErrModel[m].Clear();

    int FullTreeLevels = MaxTreeLevel; //SimpleModel.Decode(); // �������, �.�. err = hierr = 0 ������
    if (FullTreeLevels < UseLevels)
      UseLevels = FullTreeLevels;
//    int err = SimpleModel.Decode(); // ����. ������
//    int hierr = SimpleModel.Decode(); // ������ �� ������� ������� (1 .. MaxTreeLevel - 2)
    Points[FirstOnLevel[0]] = SimpleModel.DecodeBit();
    Points[FirstOnLevel[0] + NextIncOnLevel[0]] = SimpleModel.DecodeBit();
    Points[FirstOnLevel[0] + NextRowOnLevel[0]] = SimpleModel.DecodeBit();
    Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]] =
      SimpleModel.DecodeBit();


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
          Points[p] = ErrModel[INTER].Decode();

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


int TBinBtrBlock::Pack(int err, int hierr, uint8 *Points, int *LevelSizes)
{
	try
  {
    CHECK_USER(((err == 0) && (hierr == 0)),
      "����������� � �������� ��� ��������������� ������ �� ��������������");

    Coder->StartEncode();

		SimpleModel.Clear();
    for (int m = 0; m < CONTNUM; m ++)
      ErrModel[m].Clear();

//    SimpleModel.Encode(MaxTreeLevel);
//    SimpleModel.Encode(err);       // ����. ������
//    SimpleModel.Encode(hierr);     // ������ �� ������� ������� (1 .. MaxTreeLevel - 2)
    SimpleModel.EncodeBit(Points[FirstOnLevel[0]]);
    SimpleModel.EncodeBit(Points[FirstOnLevel[0] + NextIncOnLevel[0]]);
    SimpleModel.EncodeBit(Points[FirstOnLevel[0] + NextRowOnLevel[0]]);
    SimpleModel.EncodeBit(Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]]);

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
          ErrModel[INTER].Encode(Points[p]);

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

