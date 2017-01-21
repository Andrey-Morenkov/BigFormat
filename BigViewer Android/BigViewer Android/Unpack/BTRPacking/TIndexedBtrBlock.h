// TIndexedBtrBlock.h: interface for the TIndexedBtrBlock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TINDEXEDBTRBLOCK_H__51A6E1F6_5838_11D6_A24C_00D0B7A7A8D4__INCLUDED_)
#define AFX_TINDEXEDBTRBLOCK_H__51A6E1F6_5838_11D6_A24C_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define IDX_USE_HS_MODEL 0 // использовать THS256FreqModel (пока нет)

#include "TBtrBlock.h"
#include "..\\StatCoding\\TSimpleModel.h"
#if IDX_USE_HS_MODEL
	#include "..\\StatCoding\\THS256FreqModel.h"
#else
	#include "..\\StatCoding\\T256FreqModel.h"
#endif

typedef TSimpleModel TIDXDataModel;
#if IDX_USE_HS_MODEL
	typedef	THS256FreqModel TIDXErrorModel; 
#else
	typedef	T256FreqModel TIDXErrorModel; 
#endif

class TIndexedBtrBlock : public TBtrBlock  
{
private:
	TIDXDataModel SimpleModel;
	TIDXErrorModel *ErrModel;

public:
  TIndexedBtrBlock(_TAnyCoder *_Coder, int LogSize, bool Flat, int _Layers = 1);
  virtual ~TIndexedBtrBlock();

	virtual int Unpack(uint8 *Points, int UseLevels);
  	// ¬осстановить блок по верхним UseLevels уровн€м
	virtual int Pack(int err, int hierr, uint8 *Points, int *LevelSizes = NULL); 
		// ”паковать блок
};

#endif // !defined(AFX_TINDEXEDBTRBLOCK_H__51A6E1F6_5838_11D6_A24C_00D0B7A7A8D4__INCLUDED_)
