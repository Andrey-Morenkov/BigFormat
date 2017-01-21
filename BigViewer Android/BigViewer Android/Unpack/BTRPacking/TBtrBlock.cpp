// TBtrBlock.cpp: implementation of the TBtrBlock class.
//
//////////////////////////////////////////////////////////////////////
//#include <stdafx.h>

#include "TBtrBlock.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TBtrBlock::TBtrBlock(_TAnyCoder *_Coder, int LogSize, bool _Flat, int _Layers):
	ArcLens(NULL), FirstOnLevel(NULL), NextIncOnLevel(NULL), NextRowOnLevel(NULL),
  RowSize(NULL), RowCount(NULL), Flat(_Flat), Coder(_Coder), Layers(_Layers)
{
  if ((Layers < 1) || (Layers > 32))
		USER_ERROR("Invalid layers number");
	MaxTreeLevel = 2 * LogSize;

  try
  {
    ArcLens = new int[MaxTreeLevel + 1];
    FirstOnLevel = new int[MaxTreeLevel + 1];
    NextIncOnLevel = new int[MaxTreeLevel + 1];
    NextRowOnLevel = new int[MaxTreeLevel + 1];
    RowSize = new int[MaxTreeLevel + 1];
    RowCount = new int[MaxTreeLevel + 1];
  }
  catch (...)
  {
  	INTERNAL_ERROR("Memory allocation error");
  }

 	if (Flat)
  {
	  int BlockSize = (1L << (LogSize * 2)) + 1;
    PointsInRow = PointsBufSize = BlockSize;
    PointsInCol = 1;

    FirstOnLevel[0] = 0;
    NextIncOnLevel[0] = BlockSize - 1;
    NextRowOnLevel[0] = 0;
    RowSize[0] = 2;
    RowCount[0] = 1;
    for (int i = 1; i <= MaxTreeLevel; i++)
    {
      FirstOnLevel[i] = (BlockSize - 1) >> i;
      NextIncOnLevel[i] = (BlockSize - 1) >> (i - 1);
      NextRowOnLevel[i] = 0;
      RowSize[i] = 1 << (i - 1);
      RowCount[i] = 1;
      ArcLens[i] = NextIncOnLevel[i] / 2;
    }
  }
  else
  {
	  int BlockSize = (1L << LogSize) + 1;
    PointsInCol = PointsInRow = BlockSize;
    PointsBufSize = BlockSize * BlockSize;

    FirstOnLevel[0] = 0;
    NextIncOnLevel[0] = BlockSize - 1;
    NextRowOnLevel[0] = BlockSize * (BlockSize - 1);
    RowSize[0] = 2;
    RowCount[0] = 2;

    for (int i = 0; i < MaxTreeLevel / 2; i++)
    {
      FirstOnLevel[2 * i + 1] = (BlockSize - 1) >> (i + 1);
      NextIncOnLevel[2 * i + 1] = ((BlockSize - 1) >> i) * BlockSize;
      NextRowOnLevel[2 * i + 1] = (BlockSize - 1) >> i;
      RowSize[2 * i + 1] = (1 << i) + 1;
      RowCount[2 * i + 1] = 1 << i;
      ArcLens[2 * i + 1] = NextRowOnLevel[2 * i + 1] / 2;
      FirstOnLevel[2 * i + 2] = ((BlockSize - 1) >> (i + 1)) * BlockSize;
      NextIncOnLevel[2 * i + 2] = (BlockSize - 1) >> (i + 1);
      NextRowOnLevel[2 * i + 2] = ((BlockSize - 1) >> i) * BlockSize;
      RowSize[2 * i + 2] = (1 << (i + 1)) + 1;
      RowCount[2 * i + 2] = 1 << i;
      ArcLens[2 * i + 2] = NextRowOnLevel[2 * i + 2] / 2;
    }
  }

	for (int lvl = 0; lvl <= MaxTreeLevel; lvl ++)
	{
		FirstOnLevel[lvl] *= Layers;
		NextIncOnLevel[lvl] *= Layers;
		NextRowOnLevel[lvl] *= Layers;
		ArcLens[lvl] *= Layers;
	}
}

TBtrBlock::~TBtrBlock()
{
  if (ArcLens != NULL)
    delete[] ArcLens;
  if (FirstOnLevel != NULL)
    delete[] FirstOnLevel;
  if (NextIncOnLevel != NULL)
    delete[] NextIncOnLevel;
  if (NextRowOnLevel != NULL)
    delete[] NextRowOnLevel;
  if (RowSize != NULL)
    delete[] RowSize;
  if (RowCount != NULL)
    delete[] RowCount;
}

//////////////////////////////////////////////////////////////////////
// External usage methods
//////////////////////////////////////////////////////////////////////

int TBtrBlock::UnpackAll(uint8 *Points)
{
  return Unpack(Points, MaxTreeLevel);
}

