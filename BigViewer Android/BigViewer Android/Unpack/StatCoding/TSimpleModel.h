// TSimpleModel.h: interface for the TSimpleModel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TSIMPLEMODEL_H__66D65B8C_3FDC_11D6_A242_00D0B7A7A8D4__INCLUDED_)
#define AFX_TSIMPLEMODEL_H__66D65B8C_3FDC_11D6_A242_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "_TAnyCoder.h"

class TSimpleModel  
{
private:
	_TAnyCoder * Coder;

public:
	TSimpleModel(_TAnyCoder * _Coder);
	TSimpleModel() {}
	virtual ~TSimpleModel();

	void SetCoder(_TAnyCoder * _Coder)
	{
		Coder = _Coder;
	}

	void Clear(void)
	{
	}
	void EncodeBit(uint8 val)
  {
  	Coder->EncodeShift(val & 0x01, 1, 1);
  }
	void Encode(uint8 val)
  {
  	Coder->EncodeShift(val, 1, 8);
  }
	void EncodeWide(uint16 val)
  {
  	Coder->EncodeShift(val, 1, 16);
  }
	void EncodeIn(int val, int min, int max)
  {
  	Coder->Encode(val - min, 1, max - min + 1);
  }
	int DecodeIn(int min, int max)
  {
    int val = Coder->GetFreq(max - min + 1);
    Coder->Decode(val, 1, max - min + 1);
    return val + min;
  }
  uint8 DecodeBit()
  {
  	int cf = Coder->GetFreqShift(1);
    Coder->Decode(cf, 1, 2);
    return cf ? 0xFF : 0x00;
  }
  uint8 Decode()
  {
  	int cf = Coder->GetFreqShift(8);
    Coder->Decode(cf, 1, 256);
    return cf;
  }
  uint16 DecodeWide()
  {
  	int cf = Coder->GetFreqShift(16);
    Coder->Decode(cf, 1, 65536);
    return cf;
  }

  void EncodeZUInt(unsigned long val);
  unsigned long DecodeZUInt(void);
  void EncodeZInt(signed long val);
  signed long DecodeZInt(void);

  void EncodeArray(void *buf, int size);
  void DecodeArray(void *buf, int size);

  void EncodeZArray(void *buf, int size);
  void DecodeZArray(void *buf, int size);
};

#endif // !defined(AFX_TSIMPLEMODEL_H__66D65B8C_3FDC_11D6_A242_00D0B7A7A8D4__INCLUDED_)
