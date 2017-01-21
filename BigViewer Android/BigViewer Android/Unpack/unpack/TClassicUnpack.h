// TClassicUnpack.h: interface for the TClassicUnpack class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TCLASSICUNPACK_H__51A6E204_5838_11D6_A24C_00D0B7A7A8D4__INCLUDED_)
#define AFX_TCLASSICUNPACK_H__51A6E204_5838_11D6_A24C_00D0B7A7A8D4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define MC 192

const short int p1[8]={8,16,24,32,48,64,96,128};
const float gamma=0.0;
//---------------------------------------------------------------------------

class TClassicUnpack  
{
private:
	int dePIRH(void);
  int deRRE(void);

  void DeTree		( void * , void * ) ;
  void DePackNBN	(unsigned char * ,unsigned char * ,unsigned char * ) ;
  void DePackPC	( void * , void * ) ;
  void PC ( short int, short int, short int, short int, short int );

  short int UnPressRRE ( void * , void * ) ;
	short int SUB ( void );
	long unRRE(unsigned char *from,unsigned char *to,unsigned char *flags,
  			unsigned char *buf);
	void readtree(void);
	void loadHaf(short int n,short int *buf);
	short int readHaf(short int *buf);
	short int findC(short int c);
  void unLevensht(long *k/*,signed long buf*/);

  unsigned char F[MC][MC];
  unsigned char A16Kb [128*128] ;
  unsigned char *B16Kb ;
  short int iB;
  unsigned char Ctabl [MC] ;
  unsigned char kc ;
  unsigned char treeinfo [14] ;
  unsigned char metod ;

  short int hb[4][300];

	short int elemlen , *lns;
	unsigned long *cds;
	unsigned long c;
	short int l;

	short int nodes, *buff, nn;
	unsigned char table[7][256];

	short int pg,oc;

  long ptr;
	int byte;
	int bit;
	char *buf;

  void initBS(void * mbuf);
	void readBS(long *lc,short int len);
	void preloadBS(signed long *lc);
  long getBSlen(void);

public:
	void __fastcall Execute();
  int size,result;
  unsigned char *from,*to;
};

#endif // !defined(AFX_TCLASSICUNPACK_H__51A6E204_5838_11D6_A24C_00D0B7A7A8D4__INCLUDED_)
