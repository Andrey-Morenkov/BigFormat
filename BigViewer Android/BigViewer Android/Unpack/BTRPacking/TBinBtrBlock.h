// TIndexedBtrBlock.h: interface for the TIndexedBtrBlock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(TBINBTRBLOCK__INCLUDED_)
#define TBINBTRBLOCK__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define IDX_USE_HS_MODEL 0 // использовать THS256FreqModel (пока нет)

#include "TBtrBlock.h"
#include "..\\StatCoding\\TSimpleModel.h"
#if IDX_USE_HS_MODEL
	#include "..\\StatCoding\\THS2FreqModel.h"
#else
	#include "..\\StatCoding\\T2FreqModel.h"
#endif

typedef TSimpleModel TBinDataModel;
#if IDX_USE_HS_MODEL
	typedef	THS2FreqModel TBinErrorModel;
#else
	typedef	T2FreqModel TBinErrorModel;
#endif

class TBinBtrBlock : public TBtrBlock
{
private:
	TBinDataModel SimpleModel;
	TBinErrorModel *ErrModel;

public:
  TBinBtrBlock(_TAnyCoder *_Coder, int LogSize, bool Flat, int _Layers = 1);
  virtual ~TBinBtrBlock();

	virtual int Unpack(uint8 *Points, int UseLevels);
  	// ¬осстановить блок по верхним UseLevels уровн€м
	virtual int Pack(int err, int hierr, uint8 *Points, int *LevelSizes = NULL); 
		// ”паковать блок
};

#endif // !defined(TBINBTRBLOCK__INCLUDED_)
