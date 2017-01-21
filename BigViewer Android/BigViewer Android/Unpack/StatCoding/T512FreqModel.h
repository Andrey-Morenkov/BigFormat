// T512FreqModel.h: interface for the T512FreqModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_T512FREQMODEL_H__009F40DE_3FE9_11D6_A242_00D0B7A7A8D4__INCLUDED_)
#define AFX_T512FREQMODEL_H__009F40DE_3FE9_11D6_A242_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "_TAnyCoder.h"

class T512FreqModel  
{
protected:
	_TAnyCoder * Coder;

	uint16 Freq[512];
  uint16 CumFreq[518];
  uint16 *CumFreq1, *CumFreq2, *CumFreq3, *CumFreq4, *CumFreq5,	*CumFreq6,
  	*CumFreq7, *CumFreq8;
  uint16 Total;

  uint16 GetCF(uint16 val);
	void UpdateModel(uint16 val);
public:
	void SetCoder(_TAnyCoder * _Coder)
	{
		Coder = _Coder;
	}

	T512FreqModel(_TAnyCoder * _Coder);
	T512FreqModel();
	virtual ~T512FreqModel();

	void Clear(void);
  void EncodeIn(uint16 val, uint16 min, uint16 max);
  uint16 DecodeIn(uint16 min, uint16 max);
};

#endif // !defined(AFX_T512FREQMODEL_H__009F40DE_3FE9_11D6_A242_00D0B7A7A8D4__INCLUDED_)
