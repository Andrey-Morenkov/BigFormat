//---------------------------------------------------------------------------

#pragma hdrstop

#include "TPlanshet.h"
#include "BigAccess\\BigAccess.h"
#include "TMap.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

//---------------------------------------------------------------------------
TPlanshet::TPlanshet(int Handle,TDrawable* BigMap,int w,int h)
        : TDrawable()
{
map=BigMap;
LastError=0;
Cx=0;
Cy=0;
bmp=NULL;

if((w==0)||(h==0))
	return;

//Drawing new MAP
unsigned int Wmem=0,W=0,H=0;
if(LastError=GetSmallMapSizes(Handle,&W,&H))
		{
		double Wcoef=(double)((TMap*)BigMap)->GetWidth()/(double)w;
		double Hcoef=(double)((TMap*)BigMap)->GetHeight()/(double)h;

		if(Wcoef>Hcoef)
			{
			W=((TMap*)BigMap)->GetWidth()/Wcoef;
			H=((TMap*)BigMap)->GetHeight()/Wcoef;
			}
		else
			{
			W=((TMap*)BigMap)->GetWidth()/Hcoef;
			H=((TMap*)BigMap)->GetHeight()/Hcoef;
			}
		}

Wmem=W;
while(Wmem%4!=0)
	Wmem++;

//----------------------------------------------------------
bmp=new BITMAP();
memset(bmp,0,sizeof(BITMAP));

bmp->bmWidth=W;
bmp->bmHeight=H;
bmp->bmWidthBytes=Wmem;
bmp->bmBitsPixel=8;
bmp->bmBits=new BYTE[Wmem*H];
if(bmp->bmBits==NULL)
        {
        LastError=err_NoMem;
        return;
        }

if(LastError=GetSmallMap(Handle,bmp))
		{
		for(int j=0;j<H;j++)
			for(int i=0;i<Wmem;i++)
				{
				if((j/10)%2==0)
					((BYTE*)bmp->bmBits)[j*Wmem+i]=((i/10)%2==0)?0x11:0x95;
				else
					((BYTE*)bmp->bmBits)[j*Wmem+i]=((i/10)%2==0)?0x95:0x11;
				}
		}

memset(&bmi.bmiHeader,0,sizeof(BITMAPINFOHEADER));
bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
bmi.bmiHeader.biWidth=W;
bmi.bmiHeader.biHeight=H;
bmi.bmiHeader.biPlanes=1;
bmi.bmiHeader.biBitCount=8;
bmi.bmiHeader.biCompression=BI_RGB;

HPALETTE pal;
GetPalette(Handle,&pal);

GetPaletteEntries(pal,0,256,(PALETTEENTRY*)bmi.bmiColors);
for(int i=0;i<256;i++)
	{
	BYTE tmp=bmi.bmiColors[i].rgbRed;
	bmi.bmiColors[i].rgbRed=bmi.bmiColors[i].rgbBlue;
	bmi.bmiColors[i].rgbBlue=tmp;
	bmi.bmiColors[i].rgbReserved=0;
	}

LastError=0;
}

//---------------------------------------------------------------------------
TPlanshet::~TPlanshet()
{
if(bmp!=NULL)
	{
	if(bmp->bmBits!=NULL)
        delete(bmp->bmBits);
	delete(bmp);
	}
}

//---------------------------------------------------------------------------
int TPlanshet::Draw(HDC hdc,int w,int h)
{
int srcX,srcY;

if(bmi.bmiHeader.biWidth-Cx<w)
        srcX=bmi.bmiHeader.biWidth-w;
else
        srcX=Cx;

if(bmi.bmiHeader.biHeight-Cy<h)
        srcY=bmi.bmiHeader.biHeight-h;
else
        srcY=Cy;

if(srcX<0)
	srcX=0;

if(srcY<0)
	srcY=0;

SetCoord(srcX,srcY,w,h);

StretchDIBits(hdc,0,0,bmi.bmiHeader.biWidth,bmi.bmiHeader.biHeight,srcX,-srcY,bmi.bmiHeader.biWidth,bmi.bmiHeader.biHeight,bmp->bmBits,(BITMAPINFO*)&bmi,DIB_RGB_COLORS,SRCCOPY);

//=========CUR-POS=========
int PosX,PosY,mW=w,mH=h;
RECT rt;

CurPos(PosX,PosY,mW,mH);
PosX-=1;
PosY-=1;
mW+=2;
mH+=2;

SelectObject(hdc,CreatePen(PS_SOLID,0,0x0000FF));

rt.top=PosY+mH/2-2-Cy;
rt.bottom=PosY+mH/2+2-Cy;

rt.left=0-Cx;
rt.right=PosX-Cx;
Rectangle(hdc,rt.left,rt.top,rt.right,rt.bottom);

rt.left=PosX+mW-Cx;
rt.right=bmi.bmiHeader.biWidth;
Rectangle(hdc,rt.left,rt.top,rt.right,rt.bottom);

rt.left=PosX+mW/2-2-Cx;
rt.right=PosX+mW/2+2-Cx;

rt.top=0-Cy;
rt.bottom=PosY-Cy;
Rectangle(hdc,rt.left,rt.top,rt.right,rt.bottom);

rt.top=PosY+mH-Cy;
rt.bottom=bmi.bmiHeader.biHeight;
Rectangle(hdc,rt.left,rt.top,rt.right,rt.bottom);

MoveToEx(hdc,PosX-Cx,PosY-Cy,0);
LineTo(hdc,PosX-Cx+mW,PosY-Cy);
LineTo(hdc,PosX-Cx+mW,PosY-Cy+mH);
LineTo(hdc,PosX-Cx,PosY-Cy+mH);
LineTo(hdc,PosX-Cx,PosY-Cy);

PosX++;
PosY++;
mW-=2;
mH-=2;
SelectObject(hdc,CreatePen(PS_SOLID,0,0xFFFFFF));
MoveToEx(hdc,PosX-Cx,PosY-Cy,0);
LineTo(hdc,PosX-Cx+mW,PosY-Cy);
LineTo(hdc,PosX-Cx+mW,PosY-Cy+mH);
LineTo(hdc,PosX-Cx,PosY-Cy+mH);
LineTo(hdc,PosX-Cx,PosY-Cy);

PosX++;
PosY++;
mW-=2;
mH-=2;
SelectObject(hdc,CreatePen(PS_SOLID,0,0x0000FF));
MoveToEx(hdc,PosX-Cx,PosY-Cy,0);
LineTo(hdc,PosX-Cx+mW,PosY-Cy);
LineTo(hdc,PosX-Cx+mW,PosY-Cy+mH);
LineTo(hdc,PosX-Cx,PosY-Cy+mH);
LineTo(hdc,PosX-Cx,PosY-Cy);

if((mW>=w/2)||(mH>=h/2))
	{
	SelectObject(hdc,CreatePen(PS_SOLID,2,0x0000FF));
	MoveToEx(hdc,PosX-Cx,PosY-Cy,0);
	LineTo(hdc,PosX-Cx+mW,PosY-Cy+mH);

	MoveToEx(hdc,PosX-Cx+mW,PosY-Cy,0);
	LineTo(hdc,PosX-Cx,PosY-Cy+mH);
	}

LastError=GetLastError();
return LastError;
}

//---------------------------------------------------------------------------
void TPlanshet::Scroll(int dx,int dy,int w,int h)
{
//====X==================
if((Cx+dx)<0)
        Cx=0;
else
if((Cx+dx)>(GetWidth()-1))
        Cx=GetWidth()-1;
else
        Cx=Cx+dx;

//====Y==================
if((Cy+dy)<0)
        Cy=0;
else
if((Cy+dy)>GetHeight()-1)
        Cy=GetHeight()-1;
else
        Cy=Cy+dy;
}

//---------------------------------------------------------------------------
void TPlanshet::SetCoord(int x,int y,int w,int h)
{
//===X=====
if(x<0)
        Cx=0;
else
if(x>GetWidth()-1)
        Cx=GetWidth()-1;
else
        Cx=x;

//===Y=====
if(y<0)
        Cy=0;
else
if(y>GetHeight()-1)
        Cy=GetHeight()-1;
else
        Cy=y;
}

//---------------------------------------------------------------------------

void TPlanshet::CurPos(int& mCx,int& mCy,int& mW,int& mH)
{
double dx=0,dy=0;

if(bmi.bmiHeader.biWidth>0)
	dx=(double)((TMap*)map)->GetWidth()/(double)bmi.bmiHeader.biWidth;
if(bmi.bmiHeader.biHeight)
	dy=(double)((TMap*)map)->GetHeight()/(double)bmi.bmiHeader.biHeight;

if((dx==0)||(dy==0))
	{
	mCx=Cx;
	mCy=Cy;
	}
else
	{
	mCx=int((double)((TMap*)map)->GetCx()/(double)dx);
	mCy=int((double)((TMap*)map)->GetCy()/(double)dy);
	mW=int((double)mW/(double)dx);
	mH=int((double)mH/(double)dy);
	}
}

//---------------------------------------------------------------------------
void TPlanshet::GoToCurPos(int w,int h)
{
int x,y,nw=w,nh=h;
CurPos(x,y,nw,nh);

SetCoord(x+nw/2-w/2,y+nh/2-h/2,w,h);
}

//---------------------------------------------------------------------------

