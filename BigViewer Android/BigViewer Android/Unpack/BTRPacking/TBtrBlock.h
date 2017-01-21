// TBtrBlock.h: interface for the TBtrBlock class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TBTRBLOCK_H__66D65B8E_3FDC_11D6_A242_00D0B7A7A8D4__INCLUDED_)
#define AFX_TBTRBLOCK_H__66D65B8E_3FDC_11D6_A242_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\StatCoding\_TAnyCoder.h"
#include "..\StatCoding\Errors.h"
#include <stdlib.h>

class TBtrBlock  
{
protected:
	bool Flat;
	int Layers;
  int MaxTreeLevel; // ������������ ������� ������, ���� 4 ������� ������� - 0� �������

	_TAnyCoder *Coder;

  int *ArcLens;
  int *FirstOnLevel; // ����� ������� ���� �� ������ ������
	int *NextIncOnLevel, *NextRowOnLevel; // �������� ����� ������� �� ������

  int *RowSize, *RowCount; // ����� ����� �� ������ (� ������ / �����)
  
	int PointsBufSize, PointsInRow, PointsInCol;
public:
	TBtrBlock(_TAnyCoder *_Coder, int LogSize, bool _Flat, int _Layers = 1);
  virtual ~TBtrBlock();

	virtual int UnpackAll(uint8 *Points);
		// ������������ ����
	virtual int Unpack(uint8 *Points, int UseLevels) = 0;
  	// ������������ ���� �� ������� UseLevels �������
	virtual int Pack(int err, int hierr, uint8 *Points, int *LevelSizes = NULL) = 0; 
		// ��������� ����
};

#endif // !defined(AFX_TBTRBLOCK_H__66D65B8E_3FDC_11D6_A242_00D0B7A7A8D4__INCLUDED_)
