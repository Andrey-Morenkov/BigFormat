// TClassicUnpack.cpp: implementation of the TClassicUnpack class.
//
//////////////////////////////////////////////////////////////////////
//#include "stdafx.h"

#include "TClassicUnpack.h"
#include <memory.h>

//////////////////////////////////////////////////////////////////////
// Methods
//////////////////////////////////////////////////////////////////////

void __fastcall TClassicUnpack::Execute()
{
	//---- Place thread code here ----
  if (size == 0)
  {
  	memset(to,0xFF,16384);
    result=0;
  }
  else if ((size==1)&&(from[0]==0))
  {
  	memset(to,0,16384);
    result=0;
  }
  else if ((size==1)&&(from[0]==0xFF))
  {
  	memset(to,0xFF,16384);
    result=0;
  }
  else if ((size==2)&&(from[0]==1))
  {
  	memset(to,from[1],16384);
    result=0;
  }
  else if (size==16384)
  {
  	memcpy(to,from,16384);
    result=0;
  }
  else if ((size>1)&&(from[0]==0))
  {
	// rre
		result=deRRE();
  }
  else
  {
	// pirh
  	if (from[0]>MC) result=1;
  	else result=dePIRH();
  }
}
//---------------------------------------------------------------------------
#define fastRead(len) {	ptr+=(len);	byte=(short int)(ptr/8);\
	bit =(short int)(ptr%8); }

#define fastPreload(lcc) { ((char *)(&lcc))[3]=buf[byte];\
	((char *)(&lcc))[2]=buf[byte+1];\
	((char *)(&lcc))[1]=buf[byte+2]; \
	((char *)(&lcc))[0]=buf[byte+3];  \
	lcc<<=bit; }

#define readBit(lc) { (*(lc))=(long)buf[byte];(*(lc))<<=(24+bit);ptr++;\
 byte=(short int)(ptr/8);	bit =(short int)(ptr%8);}

//---------------------------------------------------------------------------
int TClassicUnpack::dePIRH()
{
  B16Kb=from;
	if ( SUB() ) return 1 ;

	if ( kc > 1 )
  {
		metod = treeinfo[0] >> 6 ;
		if ( metod > 2 ) return 1 ;
			treeinfo[0] &= 0x3F ;
  } else return 1;
  B16Kb=to;
  DeTree ( &B16Kb[0] , &A16Kb[0] ) ;
  DePackNBN ( &Ctabl[0] , &A16Kb[0] , &B16Kb[0] ) ;
 	DePackPC ( &B16Kb[0] , &A16Kb[0] ) ;
  return 0;
}


int TClassicUnpack::deRRE()
{
	if ( size!=UnPressRRE(from, to) ) return 1 ;
  else return 0;
}

short int TClassicUnpack::SUB ( void )
{
  kc = B16Kb [ 0 ] ;
  short int k=size;
	short int i , j , s ;

  for ( i=1 ; i<=kc ; i++ ) Ctabl [i-1] = B16Kb [i] ;
  j = 0 ;
  s = - ( B16Kb [kc+1] & 0xC0 ) ;
  for ( i=kc+1 ; i<kc+1+14 ; i++ )
  	if ( s != kc )
  	{
  		s += ( treeinfo [i-kc-1] = B16Kb [i] ) ;
    	j ++ ;
  	}
  	else
  		treeinfo [i-kc-1] = 0 ;
  if ( s != kc ) return 1 ;
  memcpy ( &A16Kb[0] , &B16Kb[kc+1+j] , k-kc-1-j ) ;
  return 0 ;
}


short int TClassicUnpack::UnPressRRE ( void * src, void * dst)
{
	unsigned char *bbb1=A16Kb;
	unsigned char bbb2[4500];
	short int f;
	long res;

	res=unRRE(from,to,bbb1,bbb2);
	f=(short int)(res%8);

	return (short int)( f ? (res/8)+1 : (res/8) );
}

long TClassicUnpack::unRRE(unsigned char *from,unsigned char *to,unsigned char *flags,
			unsigned char *buf)
{
	unsigned long lc;
	short int par1,par2,obflag;
	short int max;
  unsigned char *addr;
	short int i,j,w,h,c,x,y;
	short int *wb=(short int*)buf,*hb=(short int*)(buf+512),
  			*cb=(short int*)(buf+1024);	/* buf+2224 */
	short int www,seg;

	initBS(from);
	readBS((long*)&lc,8);
	/* must be lc==0 */
	readBS((long*)&lc,8);
	lc>>=24;
	obflag=(short int)lc&64;
	par2=(short int)lc&56;
	par2>>=3;
	par1=(short int)lc&7;

	max=p1[par1];
	pg=par2;

	loadHaf(max,wb);
	loadHaf(max,hb);
	loadHaf(256+par2,cb);

  memset(flags,0,16384);
  memset(table,0xFF,1792);
  for (i=0;i<256;i++)
  	table[0][i]=i;

	i=j=0;
	while ((i<128)&&(j<128))
	{
	if (j>0) oc=to[(i<<7)+j-1];
	else if (i>0) oc=to[((i-1)<<7)+j];
	else oc=255;
		/* read */
		if (obflag)
		{
			//readBit((long*)&lc);
      readBS((long*)&lc,1);
			lc>>=31;
			lc&=1L;
			if (!lc)
			{
				w=h=1;
			}
			else
			{
				/* read w,h */
				w=readHaf(wb)+1;
				h=readHaf(hb)+1;
			}
		}
		else
		{
			/* read w,h */
			w=readHaf(wb)+1;
			h=readHaf(hb)+1;
		}
		/* read c */
		c=readHaf(cb);

		c=findC(c);

		www=w;
		seg=((i+h-1)<<7)+j;
		for (w=0;www>0;w++)
			if (!flags[seg+w])
				www--;

    seg=(i<<7)+j;
    for (y=i;y<i+h;y++,seg+=128)
		{
		 	for (x=0;x<w;x++)
				if (!flags[seg+x]) to[seg+x]=c;
			memset(flags+seg,1,w);
		}

    seg=(i<<7)+j;
    addr=(unsigned char *)memchr(flags+seg,0,16384-seg);
    if (addr==NULL)
    	i=128;
    else
    	i=(addr-flags)/128,j=(addr-flags)%128;
	}

	return getBSlen();
}

void TClassicUnpack::readtree(void)
{
	unsigned long lc;
	short int n=nodes;
	nodes++;

	readBS((long*)&lc,2);
	lc>>=30;
	if (lc&2)
		buff[n*2]=nn,nn++;
	else
	{
		buff[n*2]=nodes+1000;
		readtree();
	}
	if (lc&1)
		buff[n*2+1]=nn,nn++;
	else
	{
		buff[n*2+1]=nodes+1000;
		readtree();
	}
}

void TClassicUnpack::loadHaf(short int n,short int *buf)
{
	short int i;
	unsigned long lc;

	nodes=0;
	nn=0;
	buff=buf;

  if (n>256) elemlen=9;
	else if (n>128) elemlen=8;
	else if (n>64)  elemlen=7;
	else if (n>32)  elemlen=6;
	else if (n>16)  elemlen=5;
	else if (n>8)   elemlen=4;
	else if (n>4)   elemlen=3;
	else if (n>2)   elemlen=2;
	else            elemlen=1;

	readtree();
	for (i=0;i<nn;i++)
	{
		readBS((long*)&lc,elemlen);
		lc>>=(32-elemlen);
		lc&=((1L<<elemlen)-1L);
		hb[0][i]=(short int)lc;
	}
	for (i=0;i<nodes*2;i++)
		if (buff[i]<1000)
		{
			buff[i]=hb[0][buff[i]];
		}
}

short int TClassicUnpack::readHaf(short int *buf)
{
	signed long lc;
	short int l=0,n=1000;

  preloadBS(&lc);

	while (n>=1000)
	{
		if (lc<0)
			n=buf[(n-1000)*2+1];
		else
			n=buf[(n-1000)*2];
		lc<<=1;
		l++;
	}
  readBS(&lc,l);
	return n;
}


short int TClassicUnpack::findC(short int c)
{
  ////!!!!!!!!!!!!!!!! table rotate
	short int i,j;
	if (c<256)
	{
		for (j=pg-1;j>0;j--)
			table[j][oc]=table[j-1][oc];
		table[0][oc]=c;
		return c;
	}
	else
	{
		i=c-256;
		c=table[i][oc];
		for (j=i;j>0;j--)
			table[j][oc]=table[j-1][oc];
		table[0][oc]=c;
		return c;
	}
}

void TClassicUnpack::initBS(void * mbuf)
{
	ptr=0;
	bit=0;
	byte=0;
	buf=(char *)mbuf;
}

void TClassicUnpack::readBS(long *lc,short int len)
{
	char *c=(char *)lc;

	c[3]=buf[byte];
	c[2]=buf[byte+1];
	c[1]=buf[byte+2];
	c[0]=buf[byte+3];
	(*lc)<<=bit;

	ptr+=len;
	byte=(short int)(ptr/8);
	bit =(short int)(ptr%8);
}


void TClassicUnpack::preloadBS(signed long *lc)
{
	char *c=(char *)lc;

	c[3]=buf[byte];
	c[2]=buf[byte+1];
	c[1]=buf[byte+2];
	c[0]=buf[byte+3];
	(*lc)<<=bit;
}

long TClassicUnpack::getBSlen(void)
{
	if (ptr>=131072L) return 1L;
	return ptr;
}
/**********************************************************************/

void TClassicUnpack::unLevensht(long *k/*,signed long buf*/)
{
	long kk;
  signed long buf;

      readBS(&buf,1);
      if (buf<0) {(*k)=1L;return;}
			//k>1
      readBS(&buf,1);
      if (buf<0)
      {
	      readBS(&buf,1);
        if (buf<0)
        	(*k)=2L;
	      else
        	(*k)=0L;
	      readBS(&buf,1);
        if (buf<0) (*k)++;
        (*k)+=2L;
        return;
      }
      //k>5
      (*k)=0L;

      readBS(&buf,1);
			while ( buf >= 0 )
          {
						(*k) += 32L ;
             readBS(&buf,1);
          }
      readBS(&buf,5);
      kk=(buf>>27)&(0x0000001fL);
			(*k) += (kk +1L);

      (*k)+=5L;
}


void TClassicUnpack::DeTree		( void * dst, void * src )
{
  short int HTree[MC][2];
  short int c,i,j,b;
  long d,k;
  unsigned long lc;

	i=0;d=0;

  for (c=1;c<15;c++)
  {
  	b=treeinfo[c-1];
    d<<=1;
    for (j=0;j<b;j++)
    {
    	HTree[i][0]=c;
      HTree[i][1]=d;
      i++;
      d++;
    }
  }

  initBS(src);
  c=0;

  while (c<16383)
  {
		preloadBS((long *)&lc);
    for (i=0;i<MC;i++)
    	if ((lc>>(32-HTree[i][0]))==HTree[i][1])
      	break;
    readBS((long *)&lc,HTree[i][0]);
    if (i) ((char *)dst)[c]=i,c++;
    else
    {
      unLevensht((long *)&k/*,lc*/);
      memset((char *)dst+c,0,k);
      c+=k;
    }
  }
}

void TClassicUnpack::DePackNBN	(unsigned char * tbl,unsigned char * dst,unsigned char * src)
{
	short int i,j,fl,fl2;
  short int indexA,indexB;
  unsigned char spr;

  for ( i=0 ; i<MC ; i++ )
    for ( j=0 ; j<MC ; j++ )
      F [i][j] = j ;
  for ( i=1 ; i<MC ; i++ ) {
    F [i][0] = i ;
    F [i][i] = 0 ; }


	indexA = indexB = spr = 0;//kc = 0 ;

	dst [indexA] = tbl [0];

  while ( ++ indexA < 16384 )
  {
    fl2=src[indexB++];
		fl=F[spr][fl2];
    j=fl2>>1;
		for (i=fl2;i>j;i--)
    	F[spr][i]=F[spr][i-1];
    F[spr][j]=fl;
  	spr=fl;
    dst [indexA]=tbl[fl] ;
  }

}

void TClassicUnpack::DePackPC	( void * dst, void * src )
{
  short int i=0,j,k;

	if (!metod)
  	memcpy(dst,src,16384);
  if (metod==1)
    for (j=0;j<128;j++)
    	for (k=j;k<16384;k+=128)
      	((char *)dst)[i++]=((char *)src)[k];
	if (metod==2)
    iB=0,PC(0,0,64,0,0);
}

void TClassicUnpack::PC ( short int x, short int y, short int level, short int dir, short int st )
{
if ( level == 1 )
	switch ( dir * 2 + st ) {
		case 0 :	B16Kb [  x  + (   y   << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [ x+1 + (   y   << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [ x+1 + ( (y+1) << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [  x  + ( (y+1) << 7 ) ] = A16Kb [ iB ++ ] ;
					break ;
		case 1 :	B16Kb [ x+1 + ( (y+1) << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [  x  + ( (y+1) << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [  x  + (   y   << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [ x+1 + (   y   << 7 ) ] = A16Kb [ iB ++ ] ;
					break ;
		case 2 :	B16Kb [  x  + (   y   << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [  x  + ( (y+1) << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [ x+1 + ( (y+1) << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [ x+1 + (   y   << 7 ) ] = A16Kb [ iB ++ ] ;
					break ;
		case 3 :	B16Kb [ x+1 + ( (y+1) << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [ x+1 + (   y   << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [  x  + (   y   << 7 ) ] = A16Kb [ iB ++ ] ;
					B16Kb [  x  + ( (y+1) << 7 ) ] = A16Kb [ iB ++ ] ;
					break ; }
else
	switch ( dir * 2 + st ) {
		case 0 : PC ( x,       y,       level>>1, 1, 0 );
					PC ( x+level, y,       level>>1, 0, 0 );
					PC ( x+level, y+level, level>>1, 0, 0 );
					PC ( x,       y+level, level>>1, 1, 1 );
					break;
		case 1 : PC ( x+level, y+level, level>>1, 1, 1 );
					PC ( x      , y+level, level>>1, 0, 1 );
					PC ( x      , y      , level>>1, 0, 1 );
					PC ( x+level, y      , level>>1, 1, 0 );
					break;
		case 2 : PC ( x,       y,       level>>1, 0, 0 );
					PC ( x      , y+level, level>>1, 1, 0 );
					PC ( x+level, y+level, level>>1, 1, 0 );
					PC ( x+level, y      , level>>1, 0, 1 );
					break;
		case 3 : PC ( x+level, y+level, level>>1, 0, 1 );
					PC ( x+level, y,       level>>1, 1, 1 );
					PC ( x      , y      , level>>1, 1, 1 );
					PC ( x,       y+level, level>>1, 0, 0 );
					break; }
}
