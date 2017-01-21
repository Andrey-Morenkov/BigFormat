// TUniIndexedBtrBlock.h: interface for the TUniIndexedBtrBlock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TUNIINDEXEDBTRBLOCK_H__51A6E1FA_5838_11D6_A24C_00D0B7A7A8D4__INCLUDED_)
#define AFX_TUNIINDEXEDBTRBLOCK_H__51A6E1FA_5838_11D6_A24C_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define UNI_USE_HS_MODEL 0 // ������������ THS256FreqModel (���� ���)

#include "TBtrBlock.h"
#include "..\\StatCoding\\TSimpleModel.h"
#if UNI_USE_HS_MODEL
	#include "..\\StatCoding\\THS256FreqModel.h"
#else
	#include "..\\StatCoding\\T256FreqModel.h"
#endif

typedef TSimpleModel TUNIDataModel;
#if UNI_USE_HS_MODEL
	typedef	THS256FreqModel TUNIErrorModel; 
#else
	typedef	T256FreqModel TUNIErrorModel; 
#endif

class TUniIndexedBtrBlock : public TBtrBlock  
{
private:
  uint8 ErrModelIdx[16][16], ColorIdx[256], GoodColors[15];
  signed int ColorCount[256];
  TUNIDataModel SimpleModel;
	TUNIErrorModel *ErrModel;
public:
  TUniIndexedBtrBlock(_TAnyCoder *_Coder, int LogSize, bool Flat, int _Layers = 1);
  virtual ~TUniIndexedBtrBlock();

	virtual int Unpack(uint8 *Points, int UseLevels);
  	// ������������ ���� �� ������� UseLevels �������
	virtual int Pack(int err, int hierr, uint8 *Points, int *LevelSizes = NULL); 
		// ��������� ����
};

#endif // !defined(AFX_TUNIINDEXEDBTRBLOCK_H__51A6E1FA_5838_11D6_A24C_00D0B7A7A8D4__INCLUDED_)
