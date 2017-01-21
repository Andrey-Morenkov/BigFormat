// TGrayscaleBtrBlock.h: interface for the TGrayscaleBtrBlock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TGRAYSCALEBTRBLOCK_H__9CFDF461_3FF3_11D6_A242_00D0B7A7A8D4__INCLUDED_)
#define AFX_TGRAYSCALEBTRBLOCK_H__9CFDF461_3FF3_11D6_A242_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define GS_USE_HS_MODEL 1 // использовать THS512FreqModel 

#include "TBtrBlock.h"

#include "..\\StatCoding\\TSimpleModel.h"
#if GS_USE_HS_MODEL
	#include "..\\StatCoding\\THS512FreqModel.h"
#else
	#include "..\\StatCoding\\T512FreqModel.h"
#endif

typedef TSimpleModel TGSDataModel;
#if GS_USE_HS_MODEL
	typedef	THS512FreqModel TGSErrorModel; 
#else
	typedef	T512FreqModel TGSErrorModel; 
#endif

class TGrayscaleBtrBlock : public TBtrBlock  
{
private:
	TGSDataModel SimpleModel;
	TGSErrorModel ErrModel;

  int mm_div1[1024], mm_div2[1024];
  int div1, div2;
public:
  TGrayscaleBtrBlock(_TAnyCoder *_Coder, int LogSize, bool Flat, int _Layers = 1);
  virtual ~TGrayscaleBtrBlock();

	virtual int Unpack(uint8 *Points, int UseLevels);
  	// Восстановить блок по верхним UseLevels уровням
	virtual int Pack(int err, int hierr, uint8 *Points, int *LevelSizes = NULL); 
		// Упаковать блок
	virtual int ControlledPack(int err, int hierr, uint8 *Points, uint8 *Errs, int *LevelSizes = NULL); 
		// Упаковать блок и вычислить максимальные ошибки для узлов (для решения задач)
	virtual int PackK(int A, int err, int hierr, uint8 *Points, int *LevelSizes = NULL); 
		// Упаковать блок c K-функцией (500-A, A, A, 500-A)
	virtual int UnpackUni(uint8 *Points, int UseLevels);
  	// Восстановить блок по верхним UseLevels уровням
	virtual int PackUni(int err, int hierr, uint8 *Points, short int *Coeff, int *LevelSizes = NULL); 
		// Упаковать блок
};

#endif // !defined(AFX_TGRAYSCALEBTRBLOCK_H__9CFDF461_3FF3_11D6_A242_00D0B7A7A8D4__INCLUDED_)
