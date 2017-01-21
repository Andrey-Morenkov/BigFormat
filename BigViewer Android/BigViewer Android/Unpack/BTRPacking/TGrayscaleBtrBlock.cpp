// TGrayscaleBtrBlock.cpp: implementation of the TGrayscaleBtrBlock class.
//
//////////////////////////////////////////////////////////////////////

#define USE_HS_INTERP 0 // использовать интерполяцию по 2 точкам
#define USE_K_FUNC 0 // использовать интерполяцию по К-функциям

//#include <stdafx.h>

#include "TGrayscaleBtrBlock.h"
#include <math.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Internal usage definitions and functions
//////////////////////////////////////////////////////////////////////

#define GET_INTERP2 ((Points[p - arcl] + Points[p + arclen]) >> 1)

#if USE_HS_INTERP
	#define GET_INTERP GET_INTERP2
#else
	#define GET_INTERP \
		( ((! Flat) && (! (level % 2)) && (rpos > 0) && (rpos < (rowsize - 1))) ? \
		((9 * Points[p - arclen] + 9 * Points[p + arclen] + 10 * Points[p - posinc] + \
    2 * Points[p + posinc + arclen] + 2 * Points[p + posinc - arclen]) >> 5) \
		: \
    ((Points[p - arclen] + Points[p + arclen]) >> 1) )
#endif

#define MIN(a, b) (((a)<(b))?(a):(b))
#define MAX(a, b) (((a)>(b))?(a):(b))

int mabs(int x)
{
	if (x < 0)
  	return -x;
  else
  	return x;
}

int MyRound(double x)
{
	int a;

 	a = (int)fabs(x);
  if (fabs(x) - a >= 0.5)
  	a ++;

  if (x < 0.0)
  	a = -a;

  return a;
}

void FillArrays(int *v_div, int divisor)
{
  for (signed int i = 0; i < 1024; i++)
		v_div[i] = i / divisor;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TGrayscaleBtrBlock::TGrayscaleBtrBlock(_TAnyCoder *_Coder, int LogSize, bool Flat, int _Layers):
	TBtrBlock(_Coder, LogSize, Flat, _Layers), SimpleModel(_Coder), ErrModel(_Coder)
{
  div1 = div2 = -1;
} 

TGrayscaleBtrBlock::~TGrayscaleBtrBlock()
{

}

//////////////////////////////////////////////////////////////////////
// External usage methods
//////////////////////////////////////////////////////////////////////

int TGrayscaleBtrBlock::Unpack(uint8 *Points, int UseLevels)
{
  try
  {
    CHECK_USER(((UseLevels <= MaxTreeLevel) && (UseLevels > 1)),
      "Ошибка в параметре UseLevels");

    Coder->StartDecode();

		SimpleModel.Clear();
    ErrModel.Clear();

		int FullTreeLevels = SimpleModel.Decode();
    if (FullTreeLevels < UseLevels)
      UseLevels = FullTreeLevels;
    int err = SimpleModel.Decode(); // макс. ошибка
    int hierr = SimpleModel.Decode(); // ошибка на верхних уровнях (1 .. MaxTreeLevel - 2)
    Points[FirstOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextIncOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextRowOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]] = SimpleModel.Decode();

    int divisor, shift, shift2, * mm_div;

    divisor = 2 * hierr + 1;
    shift2 = hierr;
    shift = (255 + divisor) / divisor * divisor + hierr;
    if (! hierr) shift --;
    mm_div = mm_div1;
    if (divisor != div1)
      FillArrays(mm_div, divisor);
    div1 = divisor;

    int posinc, rowinc, rowsize, rowcount, arclen, p, row, rpos;
    int mine, maxe, ne, interp, level;

    // Распакуем первые UseLevels уровней
    for (level = 1; level <= UseLevels; level ++)
    {
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];

      if (level == FullTreeLevels - 1)
        if (hierr != err)
        {
          divisor = 2 * err + 1;
          shift2 = err;
          shift = (255 + divisor) / divisor * divisor + err;
          if (! err) shift --;
          ErrModel.Clear();
          mm_div = mm_div2;
          if (divisor != div2)
            FillArrays(mm_div, divisor);
          div2 = divisor;
        }

      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
          interp = shift - GET_INTERP;

          mine = mm_div[interp];
          maxe = mm_div[255 + interp];

          ne = ErrModel.DecodeIn(mine, maxe) * divisor -	interp + shift2;

          Points[p] = (ne > 255) ? 255 : ((ne < 0) ? 0 : ne);

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
          Points[p] = GET_INTERP;
          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row
    } // for level

    Coder->FinishDecode();
    return (int)(Coder->GetPassed());
  }
  catch(...)
  {
  	return -1;
  }
}

int TGrayscaleBtrBlock::ControlledPack(int err, int hierr, uint8 *Points, uint8 *Errs, int *LevelSizes)
{
	try
  {
		uint8 *PP = NULL;
		PP = new uint8[PointsBufSize];
    
		Coder->StartEncode();

		SimpleModel.Clear();
    ErrModel.Clear();

    SimpleModel.Encode(MaxTreeLevel);
    SimpleModel.Encode(err);       // макс. ошибка
    SimpleModel.Encode(hierr);     // ошибка на верхних уровнях (1 .. MaxTreeLevel - 2)
    SimpleModel.Encode(Points[FirstOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextIncOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]]);

    PP[FirstOnLevel[0]] = Points[FirstOnLevel[0]];
    PP[FirstOnLevel[0] + NextIncOnLevel[0]] = Points[FirstOnLevel[0] + NextIncOnLevel[0]];
    PP[FirstOnLevel[0] + NextRowOnLevel[0]] = Points[FirstOnLevel[0] + NextRowOnLevel[0]];
    PP[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]] = 
			Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]];

    int divisor, shift, shift2, * mm_div, * err_div;

    divisor = 2 * hierr + 1;
    shift2 = hierr;
    shift = (255 + divisor) / divisor * divisor + hierr;
    if (! hierr) shift --;
    mm_div = mm_div1, err_div = mm_div1;
    if (divisor != div1)
      FillArrays(mm_div, divisor);
    div1 = divisor;

    int posinc, rowinc, rowsize, rowcount, arclen, p, row, rpos;
    int mine, maxe, ne, interp;

    for (int level = 1; level <= MaxTreeLevel; level ++)
    {
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];

      if (level == MaxTreeLevel - 1)
        if (hierr != err)
        {
          divisor = 2 * err + 1;
          shift2 = err;
          shift = (255 + divisor) / divisor * divisor + err;
          if (! err) shift --;
          ErrModel.Clear();
          mm_div = mm_div2, err_div = mm_div2;
          if (divisor != div2)
            FillArrays(mm_div, divisor);
          div2 = divisor;
        }

      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
	  	    interp = shift - ((Points[p - arclen] + Points[p + arclen]) >> 1);

          mine = mm_div[interp];
          maxe = mm_div[255 + interp];
          ne = err_div[Points[p] + interp];

          ErrModel.EncodeIn(ne, mine, maxe);

          ne = ne * divisor - interp + shift2;

					PP[p] = Points[p];
          Points[p] = (ne > 255) ? 255 : ((ne < 0) ? 0 : ne);
					Errs[p] = abs(PP[p] - Points[p]);
					PP[p] = Points[p];

          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row
			int L;

			for (L = level + 1; L <= MaxTreeLevel; L ++)
			{
				posinc = NextIncOnLevel[L], rowinc = NextRowOnLevel[L];
				rowsize = RowSize[L], rowcount = RowCount[L];
				arclen = ArcLens[L];

				p = FirstOnLevel[L];
				rowinc -= posinc * rowsize;

				for (row = 0; row < rowcount; row ++)
				{
					for (rpos = 0; rpos < rowsize; rpos ++)
					{
						PP[p] = (PP[p - arclen] + PP[p + arclen]) >> 1;
						Errs[p] = abs(PP[p] - Points[p]);
						p += posinc;
					} // for rowpos
					p += rowinc;
				} // for row
			} // for L

			for (L = MaxTreeLevel; L > level; L --)
			{
				posinc = NextIncOnLevel[L], rowinc = NextRowOnLevel[L];
				rowsize = RowSize[L], rowcount = RowCount[L];
				arclen = ArcLens[L];

				p = FirstOnLevel[L];
				rowinc -= posinc * rowsize;

				for (row = 0; row < rowcount; row ++)
				{
					for (rpos = 0; rpos < rowsize; rpos ++)
					{
						Errs[p + arclen] = MAX(Errs[p], Errs[p + arclen]);
						p += posinc;
					} // for rowpos
					p += rowinc;
				} // for row
			} // for L

      if (LevelSizes != NULL)
        LevelSizes[level] = (int)(Coder->GetPassed() + 4);
    } // for level
    Coder->FinishEncode();

		if (PP)
			delete[] PP;
    
		return (int)(Coder->GetPassed());
  }
	catch(char *e)
	{
		return -1;
	}
  catch(...)
  {
  	return -1;
  }
}


int TGrayscaleBtrBlock::Pack(int err, int hierr, uint8 *Points, int *LevelSizes)
{
	try
  {
    Coder->StartEncode();

		SimpleModel.Clear();
    ErrModel.Clear();

    SimpleModel.Encode(MaxTreeLevel);
    SimpleModel.Encode(err);       // макс. ошибка
    SimpleModel.Encode(hierr);     // ошибка на верхних уровнях (1 .. MaxTreeLevel - 2)
    SimpleModel.Encode(Points[FirstOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextIncOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]]);

    int divisor, shift, shift2, * mm_div, * err_div;

    divisor = 2 * hierr + 1;
    shift2 = hierr;
    shift = (255 + divisor) / divisor * divisor + hierr;
    if (! hierr) shift --;
    mm_div = mm_div1, err_div = mm_div1;
    if (divisor != div1)
      FillArrays(mm_div, divisor);
    div1 = divisor;

    int posinc, rowinc, rowsize, rowcount, arclen, p, row, rpos;
    int mine, maxe, ne, interp;

    for (int level = 1; level <= MaxTreeLevel; level ++)
    {
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];

      if (level == MaxTreeLevel - 1)
        if (hierr != err)
        {
          divisor = 2 * err + 1;
          shift2 = err;
          shift = (255 + divisor) / divisor * divisor + err;
          if (! err) shift --;
          ErrModel.Clear();
          mm_div = mm_div2, err_div = mm_div2;
          if (divisor != div2)
            FillArrays(mm_div, divisor);
          div2 = divisor;
        }

      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

      for (row = 0; row < rowcount; row ++)
      {
#if USE_K_FUNC
				int kf = 0;
#endif
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
#if USE_K_FUNC
					/*const int c[3][6] =
						{
							{0, 0, 500, 623, -178, 55},
							{0, -123, 623, 623, -123, 0},
							{55, -178, 623, 500, 0, 0}
						};*/

					const int c[1][6] =
						{
							{0, -123, 623, 623, -123, 0},
						};

					int p1 = p - 5 * arclen, p2 = p - 3 * arclen, p3 = p - arclen,
						p4 = p + arclen, p5 = p + 3 * arclen, p6 = p + 5 * arclen;

					if (row == 0)
						p2 = p3;
					if (row < 2)
						p1 = p2;
					if (row == rowcount - 1)
						p5 = p4;
					if (row > rowcount - 3)
						p6 = p5;

					interp = (c[kf][0] * Points[p1] + c[kf][1] * Points[p2] + 
						c[kf][2] * Points[p3] + c[kf][3] * Points[p4] + c[kf][4] * Points[p5] + 
						c[kf][5] * Points[p6]) / 1000;
					if (interp < 0) 
						interp = 0;
					if (interp > 255)
						interp = 255;
					interp = shift - interp;
					kf = (kf + 1) % 1;//3;
#else
          interp = shift - GET_INTERP;
#endif
  //  	    interp = shift - ((Points[p - arclen] + Points[p + arclen]) >> 1);

          mine = mm_div[interp];
          maxe = mm_div[255 + interp];
          ne = err_div[Points[p] + interp];

          ErrModel.EncodeIn(ne, mine, maxe);

          ne = ne * divisor - interp + shift2;

          Points[p] = (ne > 255) ? 255 : ((ne < 0) ? 0 : ne);

          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row

      if (LevelSizes != NULL)
        LevelSizes[level] = (int)(Coder->GetPassed() + 4);
    } // for level
    Coder->FinishEncode();
    return (int)(Coder->GetPassed());
  }
	catch(char *e)
	{
		return -1;
	}
  catch(...)
  {
  	return -1;
  }
}



int TGrayscaleBtrBlock::PackK(int A, int err, int hierr, uint8 *Points, int *LevelSizes)
{
	try
  {
    Coder->StartEncode();

		SimpleModel.Clear();
    ErrModel.Clear();

    SimpleModel.Encode(MaxTreeLevel);
    SimpleModel.Encode(err);       // макс. ошибка
    SimpleModel.Encode(hierr);     // ошибка на верхних уровнях (1 .. MaxTreeLevel - 2)
    SimpleModel.Encode(Points[FirstOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextIncOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]]);

    int divisor, shift, shift2, * mm_div, * err_div;

    divisor = 2 * hierr + 1;
    shift2 = hierr;
    shift = (255 + divisor) / divisor * divisor + hierr;
    if (! hierr) shift --;
    mm_div = mm_div1, err_div = mm_div1;
    if (divisor != div1)
      FillArrays(mm_div, divisor);
    div1 = divisor;

    int posinc, rowinc, rowsize, rowcount, arclen, p, row, rpos;
    int mine, maxe, ne, interp;

    for (int level = 1; level <= MaxTreeLevel; level ++)
    {
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];

      if (level == MaxTreeLevel - 1)
        if (hierr != err)
        {
          divisor = 2 * err + 1;
          shift2 = err;
          shift = (255 + divisor) / divisor * divisor + err;
          if (! err) shift --;
          ErrModel.Clear();
          mm_div = mm_div2, err_div = mm_div2;
          if (divisor != div2)
            FillArrays(mm_div, divisor);
          div2 = divisor;
        }

      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
					int c[6];
					
					c[0] = c[5] = 0;
					c[2] = c[3] = A;
					c[1] = c[4] = 500 - A;

					int p1 = p - 5 * arclen, p2 = p - 3 * arclen, p3 = p - arclen,
						p4 = p + arclen, p5 = p + 3 * arclen, p6 = p + 5 * arclen;

					if (row == 0)
						p2 = p3;
					if (row < 2)
						p1 = p2;
					if (row == rowcount - 1)
						p5 = p4;
					if (row > rowcount - 3)
						p6 = p5;

					interp = (c[0] * Points[p1] + c[1] * Points[p2] + 
						c[2] * Points[p3] + c[3] * Points[p4] + c[4] * Points[p5] + 
						c[5] * Points[p6]) / 1000;

					if (interp < 0) 
						interp = 0;
					if (interp > 255)
						interp = 255;

					interp = shift - interp;

          mine = mm_div[interp];
          maxe = mm_div[255 + interp];
          ne = err_div[Points[p] + interp];

          ErrModel.EncodeIn(ne, mine, maxe);

          ne = ne * divisor - interp + shift2;

          Points[p] = (ne > 255) ? 255 : ((ne < 0) ? 0 : ne);

          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row

      if (LevelSizes != NULL)
        LevelSizes[level] = (int)(Coder->GetPassed() + 4);
    } // for level
    Coder->FinishEncode();
    return (int)(Coder->GetPassed());
  }
	catch(char *e)
	{
		return -1;
	}
  catch(...)
  {
  	return -1;
  }
}


int TGrayscaleBtrBlock::UnpackUni(uint8 *Points, int UseLevels)
{
	// TODO: !!!
  try
  {
    CHECK_USER(((UseLevels <= MaxTreeLevel) && (UseLevels > 1)),
      "Ошибка в параметре UseLevels");

    Coder->StartDecode();

		SimpleModel.Clear();
    ErrModel.Clear();

		int FullTreeLevels = SimpleModel.Decode();
    if (FullTreeLevels < UseLevels)
      UseLevels = FullTreeLevels;
    int err = SimpleModel.Decode(); // макс. ошибка
    int hierr = SimpleModel.Decode(); // ошибка на верхних уровнях (1 .. MaxTreeLevel - 2)
    Points[FirstOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextIncOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextRowOnLevel[0]] = SimpleModel.Decode();
    Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]] = SimpleModel.Decode();

    int divisor, shift, shift2, * mm_div;

    divisor = 2 * hierr + 1;
    shift2 = hierr;
    shift = (255 + divisor) / divisor * divisor + hierr;
    if (! hierr) shift --;
    mm_div = mm_div1;
    if (divisor != div1)
      FillArrays(mm_div, divisor);
    div1 = divisor;

    int posinc, rowinc, rowsize, rowcount, arclen, p, row, rpos;
    int mine, maxe, ne, interp, level;

    // Распакуем первые UseLevels уровней
    for (level = 1; level <= UseLevels; level ++)
    {
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];

      if (level == FullTreeLevels - 1)
        if (hierr != err)
        {
          divisor = 2 * err + 1;
          shift2 = err;
          shift = (255 + divisor) / divisor * divisor + err;
          if (! err) shift --;
          ErrModel.Clear();
          mm_div = mm_div2;
          if (divisor != div2)
            FillArrays(mm_div, divisor);
          div2 = divisor;
        }

      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
          interp = shift - GET_INTERP;

          mine = mm_div[interp];
          maxe = mm_div[255 + interp];

          ne = ErrModel.DecodeIn(mine, maxe) * divisor -	interp + shift2;

          Points[p] = (ne > 255) ? 255 : ((ne < 0) ? 0 : ne);

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
          Points[p] = GET_INTERP;
          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row
    } // for level

    Coder->FinishDecode();
    return (int)(Coder->GetPassed());
  }
  catch(...)
  {
  	return -1;
  }
}


int TGrayscaleBtrBlock::PackUni(int err, int hierr, uint8 *Points, short int *Coeff, int *LevelSizes)
// short int Coeff[3][9]; // коэфф. *1000 (сглаживание, интерполяция, восстановление)
{
	try
  {
    Coder->StartEncode();

		SimpleModel.Clear();
    ErrModel.Clear();

    SimpleModel.Encode(MaxTreeLevel);
    SimpleModel.Encode(err);       // макс. ошибка
    SimpleModel.Encode(hierr);     // ошибка на верхних уровнях (1 .. MaxTreeLevel - 2)
    
		for (int cc = 9; cc < 27; cc ++)
			SimpleModel.EncodeWide((uint16)Coeff[cc]);

		SimpleModel.Encode(Points[FirstOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextIncOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0]]);
    SimpleModel.Encode(Points[FirstOnLevel[0] + NextRowOnLevel[0] + NextIncOnLevel[0]]);

    int posinc, rowinc, rowsize, rowcount, arclen, p, row, rpos;
    int mine, maxe, ne, interp;

	  int BlockSize = (1L << MaxTreeLevel) + 1;
		uint8 *Copy = new uint8[BlockSize * BlockSize];
		memmove(Copy, Points, BlockSize * BlockSize);

		uint8 p0[10];
		int c, level;

		for (level = MaxTreeLevel; level > 1; level --)
		{
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];
			
      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

			// вычислить сглаженные существенные
      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
					if (row == 0)
						p0[0] = p0[1] = p0[2] = p0[3] = 0;
					else if (row  < 2)
					{
						p0[0] = p0[1] =0;
						p0[2] = Copy[p - arclen * 3];
						p0[3] = Copy[p - arclen * 2];
					}
					else
					{
						p0[0] = Copy[p - arclen * 5];
						p0[1] = Copy[p - arclen * 4];
						p0[2] = Copy[p - arclen * 3];
						p0[3] = Copy[p - arclen * 2];
					}
					p0[4] = Copy[p - arclen];
					p0[5] = Copy[p];
					p0[6] = Copy[p + arclen];
					if (row == rowcount - 1)
					{
						p0[7] = 0;
						p0[8] = 0;
					}
					else
					{
						p0[7] = Copy[p + arclen * 2];
						p0[8] = Copy[p + arclen * 3];
					}

					c = 0;
					for (int k = 0; k < 9; k ++)
						c += Coeff[k] * p0[k];
					c /= 1000;
          Points[p - arclen] = (c > 255) ? 255 : ((c < 0) ? 0 : c);

          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row
		}

		delete[] Copy; 

    int divisor, shift, shift2, * mm_div, * err_div;

    divisor = 2 * hierr + 1;
    shift2 = hierr;
    shift = (255 + divisor) / divisor * divisor + hierr;
    if (! hierr) shift --;
    mm_div = mm_div1, err_div = mm_div1;
    if (divisor != div1)
      FillArrays(mm_div, divisor);
    div1 = divisor;

    for (level = 1; level <= MaxTreeLevel; level ++)
    {
      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];

      if (level == MaxTreeLevel - 1)
        if (hierr != err)
        {
          divisor = 2 * err + 1;
          shift2 = err;
          shift = (255 + divisor) / divisor * divisor + err;
          if (! err) shift --;
          ErrModel.Clear();
          mm_div = mm_div2, err_div = mm_div2;
          if (divisor != div2)
            FillArrays(mm_div, divisor);
          div2 = divisor;
        }

      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

			// интерполировать и кодировать несущественные
      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
					if (row == 0)
						p0[0] = p0[1] = p0[2] = 0;
					else if (row  < 2)
					{
						p0[0] = 0;
						p0[1] = Points[p - arclen * 3];
						p0[2] = Points[p - arclen * 2];
					}
					else
					{
						p0[0] = Points[p - arclen * 4];
						p0[1] = Points[p - arclen * 3];
						p0[2] = Points[p - arclen * 2];
					}
					p0[3] = Points[p - arclen];
					p0[4] = Points[p];
					p0[5] = Points[p + arclen];
					if (row == rowcount - 1)
					{
						p0[6] = 0;
						p0[7] = 0;
						p0[9] = 0;
					}
					else if (row == rowcount - 2)
					{
						p0[6] = Points[p + arclen * 2];
						p0[7] = Points[p + arclen * 3];
						p0[8] = 0;
					}
					else
					{
						p0[6] = Points[p + arclen * 2];
						p0[7] = Points[p + arclen * 3];
						p0[8] = Points[p + arclen * 3];
					}


					interp = 0;
					for (int k = 0; k < 9; k ++)
						interp += Coeff[9 + k] * p0[k];
					interp /= 1000;

					if (interp < 0) 
						interp = 0;
					if (interp > 255)
						interp = 255;
					interp = shift - interp;
  //  	    interp = shift - ((Points[p - arclen] + Points[p + arclen]) >> 1);

          mine = mm_div[interp];
          maxe = mm_div[255 + interp];
          ne = err_div[Points[p] + interp];

          ErrModel.EncodeIn(ne, mine, maxe);

          ne = ne * divisor - interp + shift2;

          Points[p] = (ne > 255) ? 255 : ((ne < 0) ? 0 : ne);

          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row

      posinc = NextIncOnLevel[level], rowinc = NextRowOnLevel[level];
      rowsize = RowSize[level], rowcount = RowCount[level];
      arclen = ArcLens[level];
			
      p = FirstOnLevel[level];
      rowinc -= posinc * rowsize;

			// восстановить существенные из сглаженных (для последующих уровней)
      for (row = 0; row < rowcount; row ++)
      {
        for (rpos = 0; rpos < rowsize; rpos ++)
        {
					if (row == 0)
						p0[0] = p0[1] = p0[2] = p0[3] = 0;
					else if (row  < 2)
					{
						p0[0] = p0[1] =0;
						p0[2] = Copy[p - arclen * 3];
						p0[3] = Copy[p - arclen * 2];
					}
					else
					{
						p0[0] = Copy[p - arclen * 5];
						p0[1] = Copy[p - arclen * 4];
						p0[2] = Copy[p - arclen * 3];
						p0[3] = Copy[p - arclen * 2];
					}
					p0[4] = Copy[p - arclen];
					p0[5] = Copy[p];
					p0[6] = Copy[p + arclen];
					if (row == rowcount - 1)
					{
						p0[7] = 0;
						p0[8] = 0;
					}
					else
					{
						p0[7] = Copy[p + arclen * 2];
						p0[8] = Copy[p + arclen * 3];
					}

					c = 0;
					for (int k = 0; k < 9; k ++)
						c += Coeff[18 + k] * p0[k];
					c /= 1000;
          Points[p - arclen] = (c > 255) ? 255 : ((c < 0) ? 0 : c);

          p += posinc;
        } // for rowpos
        p += rowinc;
      } // for row

      if (LevelSizes != NULL)
        LevelSizes[level] = (int)(Coder->GetPassed() + 4);
    } // for level
    Coder->FinishEncode();
    return (int)(Coder->GetPassed());
  }
	catch(char *e)
	{
		return -1;
	}
  catch(...)
  {
  	return -1;
  }
}


