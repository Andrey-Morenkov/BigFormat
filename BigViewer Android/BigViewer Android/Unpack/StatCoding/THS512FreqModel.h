// T512FreqModel.h: interface for the T512FreqModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_T512FREQMODEL_H__009F40DE_3FE9_11D6_A242_00D0B7A7A8D4__INCLUDED_)
#define AFX_T512FREQMODEL_H__009F40DE_3FE9_11D6_A242_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <memory.h>
#include "_TAnyCoder.h"

class THS512FreqModel  
{
protected:
	_TAnyCoder *Coder;

	uint16 Freq[512];
  uint16 CumFreq[1024];
  uint16 *CF0, *CF1, *CF2, *CF3, *CF4, *CF5,	*CF6,	*CF7, *CF8;
  uint16 *CFu0, *CFu1, *CFu2, *CFu3, *CFu4, *CFu5,	*CFu6,	*CFu7, *CFu8;
  uint16 Total;

  uint16 GetCF(int val)
  {
  	return CF0[val] + CF1[val >> 1] + CF2[val >> 2] + CF3[val >> 3] +
    	CF4[val >> 4] + CF5[val >> 5] + CF6[val >> 6] + CF7[val >> 7] +
      CF8[val >> 8];
  }

	void UpdateModel(int val)
  {
    int ival = ~val;
		Freq[val] ++;
    Total ++;

    if (ival & 1) CFu0[val] ++;
    if (ival & 2) CFu1[val >> 1] ++;
    if (ival & 4) CFu2[val >> 2] ++;
    if (ival & 8) CFu3[val >> 3] ++;
    if (ival & 16) CFu4[val >> 4] ++;
    if (ival & 32) CFu5[val >> 5] ++;
    if (ival & 64) CFu6[val >> 6] ++;
    if (ival & 128) CFu7[val >> 7] ++;
    if (ival & 256) CFu8[val >> 8] ++;

    if (Total < 16384)
      return;

    int cum = 1, i, ii;
    memset(CumFreq, 0, sizeof(CumFreq));
    for (i = 0; i < 512; i ++)
    {
    	ii = ~i;
      Freq[i] = (Freq[i] + 1) >> 1;
      cum += Freq[i];

      if (ii & 1) CFu0[i] += Freq[i];
      if (ii & 2) CFu1[i >> 1]  += Freq[i];
      if (ii & 4) CFu2[i >> 2]  += Freq[i];
      if (ii & 8) CFu3[i >> 3]  += Freq[i];
      if (ii & 16) CFu4[i >> 4] += Freq[i];
      if (ii & 32) CFu5[i >> 5] += Freq[i];
      if (ii & 64) CFu6[i >> 6] += Freq[i];
      if (ii & 128) CFu7[i >> 7] += Freq[i];
      if (ii & 256) CFu8[i >> 8] += Freq[i];
    }
    Total = cum;
    // DONE: доделать update
  }

public:
	void SetCoder(_TAnyCoder * _Coder)
	{
		Coder = _Coder;
	}

	THS512FreqModel(_TAnyCoder *_Coder):
			Coder(_Coder)
  {
		CF0 = CumFreq;
    CF1 = CF0 + 512;
    CF2 = CF1 + 256;
    CF3 = CF2 + 128;
    CF4 = CF3 + 64;
    CF5 = CF4 + 32;
    CF6 = CF5 + 16;
    CF7 = CF6 + 8;
    CF8 = CF7 + 4;
    CFu0 = CF0 + 1;
    CFu1 = CF1 + 1;
    CFu2 = CF2 + 1;
    CFu3 = CF3 + 1;
    CFu4 = CF4 + 1;
    CFu5 = CF5 + 1;
    CFu6 = CF6 + 1;
    CFu7 = CF7 + 1;
    CFu8 = CF8 + 1;
    Clear();
  }

	THS512FreqModel()
  {
		CF0 = CumFreq;
    CF1 = CF0 + 512;
    CF2 = CF1 + 256;
    CF3 = CF2 + 128;
    CF4 = CF3 + 64;
    CF5 = CF4 + 32;
    CF6 = CF5 + 16;
    CF7 = CF6 + 8;
    CF8 = CF7 + 4;
    CFu0 = CF0 + 1;
    CFu1 = CF1 + 1;
    CFu2 = CF2 + 1;
    CFu3 = CF3 + 1;
    CFu4 = CF4 + 1;
    CFu5 = CF5 + 1;
    CFu6 = CF6 + 1;
    CFu7 = CF7 + 1;
    CFu8 = CF8 + 1;
    Clear();
  }

	void Clear(void)
  {
    memset(Freq, 0, sizeof(Freq));
    memset(CumFreq, 0, sizeof(CumFreq));
    Total = 1;
  }

  void EncodeIn(int val, int min, int max)
  {
    uint16 cfmax, cfval, cfmin;
    cfmin = GetCF(min);
    cfmax = GetCF(max) + Freq[max] + 1 - cfmin;
    cfval = GetCF(val) - cfmin;

    if (Freq[val])
      Coder->Encode(cfval + 1, Freq[val],	cfmax);
    else
    {
      Coder->Encode(0, 1, cfmax);
      Coder->Encode(val - min, 1, max - min + 1);
    }

    UpdateModel(val);
  }

  int DecodeIn(int min, int max)
  {
  	// TODO: оптимизировать decode

    uint16 cfmax, cfmin, cl, rcf;
    cfmin = GetCF(min);
    cfmax = GetCF(max) + Freq[max] + 1 - cfmin;

    uint16 cf = Coder->GetFreq(cfmax);
    int val;

    if (! cf)
    {
      Coder->Decode(0, 1, cfmax);
      val = Coder->GetFreq(max - min + 1);
      Coder->Decode(val, 1, max - min + 1);
      val += min;
    }
    else
    {
      rcf = cf;
      cf += cfmin - 1;
      val = 0;

      if (cf >= (cl = CFu8[0]))
        cf -= cl, val |= 256;
      if (cf >= (cl = CFu7[val >> 7]))
        cf -= cl, val |= 128;
      if (cf >= (cl = CFu6[val >> 6]))
        cf -= cl, val |= 64;
      if (cf >= (cl = CFu5[val >> 5]))
        cf -= cl, val |= 32;
      if (cf >= (cl = CFu4[val >> 4]))
        cf -= cl, val |= 16;
      if (cf >= (cl = CFu3[val >> 3]))
        cf -= cl, val |= 8;
      if (cf >= (cl = CFu2[val >> 2]))
        cf -= cl, val |= 4;
      if (cf >= (cl = CFu1[val >> 1]))
        cf -= cl, val |= 2;
      if (cf >= (cl = CFu0[val]))
        cf -= cl, val |= 1;

      rcf -= cf;

      Coder->Decode(rcf, Freq[val], cfmax);
    }

    UpdateModel(val);
    return val;
  }
};

#endif // !defined(AFX_T512FREQMODEL_H__009F40DE_3FE9_11D6_A242_00D0B7A7A8D4__INCLUDED_)
