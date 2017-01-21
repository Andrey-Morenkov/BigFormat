// T256FreqModel.h: interface for the T256FreqModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_T256FREQMODEL_H__009F40E1_3FE9_11D6_A242_00D0B7A7A8D4__INCLUDED_)
#define AFX_T256FREQMODEL_H__009F40E1_3FE9_11D6_A242_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "_TAnyCoder.h"

class T256FreqModel  
{
protected:
	_TAnyCoder * Coder;

	uint16 Freq[256];
  uint16 CumFreq[270];
  uint16 *CumFreq1, *CumFreq2, *CumFreq3, *CumFreq4, *CumFreq5,	*CumFreq6,
  	*CumFreq7;
  uint16 Total;

  uint16 GetCF(uint16 val);
	void UpdateModel(uint16 val);
public:
	void SetCoder(_TAnyCoder * _Coder)
	{
		Coder = _Coder;
	}

	T256FreqModel(_TAnyCoder * _Coder);
	T256FreqModel();
	virtual ~T256FreqModel();

	void Clear(void);
  void Encode(uint16 val);
  uint16 Decode();
};

#endif // !defined(AFX_T256FREQMODEL_H__009F40E1_3FE9_11D6_A242_00D0B7A7A8D4__INCLUDED_)
