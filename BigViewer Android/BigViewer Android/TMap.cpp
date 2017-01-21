//---------------------------------------------------------------------------

#pragma hdrstop

#include "TMap.h"
#include "BigAccess\\BigAccess.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

//---------------------------------------------------------------------------
TMap::TMap(char* filename,int w,int h, bool ReadPlanshet)
        :TDrawable()
{
//str=NULL;
bmp=NULL;

bmp=new BITMAP();
if(bmp==NULL)
	{
	LastError=err_NoMem;
	return;
	}
else
	LastError=0;

bmp->bmBits=NULL;


//===File Open==============
LastError=OpenBigFile(filename,Handle);

/*
TCHAR buf[25];
int len=strlen(filename);
mbstowcs(buf,filename,len);
MessageBox(0,buf,TEXT("������ map"),MB_ICONEXCLAMATION);
*/

if(!LastError)
	LastError=SetZoom(1,0,0);
//==========================

if(ReadPlanshet)
	Planshet=new TPlanshet(Handle,this,w,h);
else
	Planshet=NULL;

memset(&bmi.bmiHeader,0,sizeof(BITMAPINFOHEADER));
bmi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
bmi.bmiHeader.biPlanes=1;
bmi.bmiHeader.biCompression=BI_RGB;

Cx=0;
Cy=0;

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
}

//---------------------------------------------------------------------------
TMap::~TMap()
{
if(bmp!=NULL)
	{
	if(bmp->bmBits!=NULL)
        delete(bmp->bmBits);
	delete(bmp);
	}

if(Planshet!=NULL)
  delete(Planshet);

//if(str!=NULL)
	//delete(str);

CloseSource(Handle);
}

//---------------------------------------------------------------------------
int TMap::OpenBigFile(char* path,int& HBig)
{
return OpenSource(path,&HBig);
}

//---------------------------------------------------------------------------
int TMap::Draw(HDC hdc,int w,int h)
{
unsigned char Type=0;
if(LastError=GetColorStruct(Handle,&Type))
	return LastError;

if(Type!=3)
	Type=1;

int nw=w,nh=h;

if(GetWidth()<w)
	nw=GetWidth();
if(GetHeight()<h)
	nh=GetHeight();

bmi.bmiHeader.biWidth=nw;
bmi.bmiHeader.biHeight=nh;

unsigned int Wmem=nw*Type;
while(Wmem%4!=0)
	Wmem++;

bmp->bmWidth=nw;
bmp->bmHeight=nh;
bmp->bmWidthBytes=Wmem;


bmp->bmBitsPixel=8*Type;
bmi.bmiHeader.biBitCount=8*Type;

if(bmp->bmBits!=NULL)
	delete(bmp->bmBits);

bmp->bmBits=NULL;
bmp->bmBits=new BYTE[Wmem*h];

if(bmp->bmBits==NULL)
	return err_NoMem;

if(W-Cx<nw)
        Cx=W-nw;

if(H-Cy<nh)
        Cy=H-nh;

SetCoord(Cx,Cy,nw,nh);

if(LastError=GetImage2(Handle,Cx,Cy,bmp))
		return LastError;

StretchDIBits(hdc,0,0,nw,nh,0,0,nw,nh,bmp->bmBits,(BITMAPINFO*)&bmi,DIB_RGB_COLORS,SRCCOPY);
return 0;
}

//---------------------------------------------------------------------------
void TMap::Scroll(int dx,int dy,int w,int h)
{
//====X==================
if(Cx+dx<0)
        Cx=Cx;//0;
else
if(Cx+dx+w>GetWidth()-1)
        Cx=GetWidth()-1-w;
else
        Cx=Cx+dx;

//====Y==================
if(Cy+dy<0)
        Cy=0;
else
if(Cy+dy+h>GetHeight()-1)
        Cy=GetHeight()-1-h;
else
        Cy=Cy+dy;
}

//---------------------------------------------------------------------------
void TMap::SetCoord(int x,int y,int w,int h)
{
//===X=====
if(x<0)
        Cx=0;
else
if(x>GetWidth()-1-w)
       Cx=GetWidth()-1-w;
else
        Cx=x;

//===Y=====
if(y<0)
        Cy=0;
else
if(y>GetHeight()-1-h)
        Cy=GetHeight()-1-h;
else
        Cy=y;
}

//---------------------------------------------------------------------------
int TMap::GetZoom( )
{
int zoom=0;
GetZoomOut(Handle,&zoom);
return zoom;
}

//---------------------------------------------------------------------------
int TMap::SetZoom(int zoom,int sw, int sh)
{
int oldzoom=GetZoom();

LastError=SetZoomOut(Handle,zoom);

if(!LastError)
	LastError=GetSourceSizes(Handle,&W,&H);
else
	MessageBox(0,TEXT("Error on set zoom!"),TEXT("MAP"),MB_ICONEXCLAMATION);

if((zoom!=oldzoom)&&(!LastError))
	{
	Cx+=sw/2;
	Cy+=sh/2;

	switch(oldzoom)
	{
	case -2:Cx/=2;Cy/=2;break;
	case 1:break;
	case 2:Cx*=2;Cy*=2;break;
	case 4:Cx*=4;Cy*=4;break;
	case 8:Cx*=8;Cy*=8;break;
	default:oldzoom=0;break;
	}

	if(oldzoom!=0)
	switch(zoom)
	{
	case -2:Cx*=2;Cy*=2;break;
	case 1:break;
	case 2:Cx/=2;Cy/=2;break;
	case 4:Cx/=4;Cy/=4;break;
	case 8:Cx/=8;Cy/=8;break;
	default:return err_InvalParam;
	}

	SetCoord(Cx-sw/2,Cy-sh/2,sw,sh);
	}

return LastError;
}

//---------------------------------------------------------------------------
int TMap::GetLayersCnt()
{
int cnt=0;
char names[15*15];
memset(names,0,sizeof(char)*15*15);
if(GetLayersList(Handle,&cnt,names))
	return 0;

return cnt;
}

//---------------------------------------------------------------------------
char* TMap::GetLayerName(int num)
{
int cnt=0;
char names[15*15];
memset(names,0,sizeof(char)*15*15);

//if(str==NULL)
	//str=new char[15];
//else
memset(str,0,sizeof(char)*15);

//if(str==NULL)
//	return 0;

if(num>=15)
	return str;

if(GetLayersList(Handle,&cnt,names))
	return str;

strcpy(str,names+num*15);
return str;
}

//---------------------------------------------------------------------------
bool TMap::GetLayerState(int num)
{
if(num>=15)
	return false;

unsigned int lay=0;
if(GetLayers(Handle,&lay))
	return false;

return ((lay>>num)&&1);
}

//---------------------------------------------------------------------------
void TMap::SetLayerOn(int num)
{
if(num>=15)
	return;

unsigned int lay=0;
if(GetLayers(Handle,&lay))
	return;

lay|=1<<num;
SetLayers(Handle,lay);
}

//---------------------------------------------------------------------------
void TMap::SetLayerOff(int num)
{
if(num>=15)
	return;

unsigned int lay=0;
if(GetLayers(Handle,&lay))
	return;

lay&=(~(1<<num));
SetLayers(Handle,lay);
}

//---------------------------------------------------------------------------