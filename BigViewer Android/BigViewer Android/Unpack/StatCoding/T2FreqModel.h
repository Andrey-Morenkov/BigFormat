//---------------------------------------------------------------------------

#ifndef T2FreqModelH
#define T2FreqModelH
//---------------------------------------------------------------------------
#include "_TAnyCoder.h"

class T2FreqModel
{
protected:
	_TAnyCoder * Coder;

	uint16 Freq[2];
  uint16 Total;

  uint16 GetCF(uint16 val);
	void UpdateModel(uint16 val);
public:
	void SetCoder(_TAnyCoder * _Coder)
	{
		Coder = _Coder;
	}

	T2FreqModel(_TAnyCoder * _Coder);
	T2FreqModel();
	virtual ~T2FreqModel();

	void Clear(void);
  void Encode(uint16 val);
  uint16 Decode();
};

#endif
