// T256FreqModel.cpp: implementation of the T256FreqModel class.
//
//////////////////////////////////////////////////////////////////////
//#include <stdafx.h>

#include "T256FreqModel.h"
#include <memory.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

T256FreqModel::T256FreqModel(_TAnyCoder * _Coder):
	Coder(_Coder)
{
	CumFreq1 = CumFreq;
	CumFreq2 = CumFreq1 + 129;
	CumFreq3 = CumFreq2 + 65;
	CumFreq4 = CumFreq3 + 33;
	CumFreq5 = CumFreq4 + 17;
	CumFreq6 = CumFreq5 + 9;
	CumFreq7 = CumFreq6 + 5;

  Clear();
}

T256FreqModel::T256FreqModel()
{
	CumFreq1 = CumFreq;
	CumFreq2 = CumFreq1 + 129;
	CumFreq3 = CumFreq2 + 65;
	CumFreq4 = CumFreq3 + 33;
	CumFreq5 = CumFreq4 + 17;
	CumFreq6 = CumFreq5 + 9;
	CumFreq7 = CumFreq6 + 5;

  Clear();
}

T256FreqModel::~T256FreqModel()
{

}

//////////////////////////////////////////////////////////////////////
// Internal usage methods
//////////////////////////////////////////////////////////////////////

uint16 T256FreqModel::GetCF(uint16 val)
{
	uint16 cf = 0;

  if (val & 128)
  	cf += CumFreq7[val >> 7];
  if (val & 64)
  	cf += CumFreq6[val >> 6];
  if (val & 32)
  	cf += CumFreq5[val >> 5];
  if (val & 16)
  	cf += CumFreq4[val >> 4];
  if (val & 8)
  	cf += CumFreq3[val >> 3];
  if (val & 4)
  	cf += CumFreq2[val >> 2];
  if (val & 2)
  	cf += CumFreq1[val >> 1];
  if (val & 1)
  	cf += Freq[val - 1];
  return cf;
}

void T256FreqModel::UpdateModel(uint16 val)
{
	Freq[val] ++;
	CumFreq1[((val) >> 1) + 1] ++, CumFreq2[((val) >> 2) + 1] ++,
  CumFreq3[((val) >> 3) + 1] ++, CumFreq4[((val) >> 4) + 1] ++,
  CumFreq5[((val) >> 5) + 1] ++, CumFreq6[((val) >> 6) + 1] ++,
  CumFreq7[((val) >> 7) + 1] ++;

  Total ++;
  if (Total < 16384)
  	return;

  int cum = 1, i, ii;
  for (i = 0; i < 256; i ++)
  {
  	Freq[i] = (Freq[i] + 1) >> 1;
    cum += Freq[i];
  }
  Total = cum;

  ii = 0;
  for (i = 1; i <= 128; i ++)
  	CumFreq1[i] = Freq[ii] + Freq[ii + 1], ii += 2;
  ii = 1;
  for (i = 1; i <= 64; i ++)
  	CumFreq2[i] = CumFreq1[ii] + CumFreq1[ii + 1], ii += 2;
  ii = 1;
  for (i = 1; i <= 32; i ++)
  	CumFreq3[i] = CumFreq2[ii] + CumFreq2[ii + 1], ii += 2;
  ii = 1;
  for (i = 1; i <= 16; i ++)
  	CumFreq4[i] = CumFreq3[ii] + CumFreq3[ii + 1], ii += 2;
  ii = 1;
  for (i = 1; i <= 8; i ++)
  	CumFreq5[i] = CumFreq4[ii] + CumFreq4[ii + 1], ii += 2;
  ii = 1;
  for (i = 1; i <= 4; i ++)
  	CumFreq6[i] = CumFreq5[ii] + CumFreq5[ii + 1], ii += 2;
  ii = 1;
  for (i = 1; i <= 2; i ++)
  	CumFreq7[i] = CumFreq6[ii] + CumFreq6[ii + 1], ii += 2;
}

//////////////////////////////////////////////////////////////////////
// External usage methods
//////////////////////////////////////////////////////////////////////

void T256FreqModel::Clear(void)
{
	memset(Freq, 0, sizeof(Freq));
	memset(CumFreq, 0, sizeof(CumFreq));
  Total = 1;
}

void T256FreqModel::Encode(uint16 val)
{
	if (Freq[val])
		Coder->Encode(GetCF(val) + 1, Freq[val], Total);
  else
  {
  	Coder->Encode(0, 1, Total);
    Coder->Encode(val, 1, 256);
  }

  UpdateModel(val);
}

uint16 T256FreqModel::Decode(void)
{
	uint16 cl, rcf;

	uint16 cf = (uint16)(Coder->GetFreq(Total));
  int val;

  if (! cf)
  {
  	Coder->Decode(0, 1, Total);
    val = Coder->GetFreq(256);
    Coder->Decode(val, 1, 256);
  }
  else
  {
    rcf = cf;
    val = 0;
    cf --;

    if (cf >= (cl = CumFreq7[1]))
    	cf -= cl, val += 128;
    if (cf >= (cl = CumFreq6[(val >> 6) + 1]))
    	cf -= cl, val += 64;
    if (cf >= (cl = CumFreq5[(val >> 5) + 1]))
    	cf -= cl, val += 32;
    if (cf >= (cl = CumFreq4[(val >> 4) + 1]))
    	cf -= cl, val += 16;
    if (cf >= (cl = CumFreq3[(val >> 3) + 1]))
    	cf -= cl, val += 8;
    if (cf >= (cl = CumFreq2[(val >> 2) + 1]))
    	cf -= cl, val += 4;
    if (cf >= (cl = CumFreq1[(val >> 1) + 1]))
    	cf -= cl, val += 2;
    if (cf >= (cl = Freq[val]))
    	cf -= cl, val ++;

    rcf -= cf;
    while (! Freq[val]) val ++;

    Coder->Decode(/*GetCF(val)*/rcf, Freq[val], Total);
  }

  UpdateModel(val);
  return val;
}


