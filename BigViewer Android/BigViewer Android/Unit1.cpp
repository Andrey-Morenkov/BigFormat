//---------------------------------------------------------------------------

#include <fmx.h>
#include <FMX.Platform.Win.hpp>
#pragma hdrstop

#include "Unit1.h"
#include "BigAccess\\BigAccess.h"
//#include <Vcl.Graphics.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
map=NULL;
}

//---------------------------------------------------------------------------
void __fastcall TForm1::MenuItem2Click(TObject *Sender)
{
Close();
}

//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
LibAttach();
if(InitBigAccess(1024*1024))
	ShowMessage(TEXT("������ ������������� BigAccess!"));
}

//---------------------------------------------------------------------------
void __fastcall TForm1::FormDestroy(TObject *Sender)
{
if(map!=NULL)
	delete(map);
LibDetach();
}

//---------------------------------------------------------------------------
bool __fastcall TForm1::OpenBIG(AnsiString fn)
{
	filepath = fn;
if(map!=NULL)
	delete(map);

map=new TMap(filepath.c_str(),Width,Height);

if(map==NULL)
	{
	MessageBox(0,TEXT("������ ��������� ������ ��� BIG!"),TEXT("Map"),MB_ICONEXCLAMATION);
	return false;
	}
else
if(map->GetError())
	{
	wchar_t buf[25];
	_itow(map->GetError(),buf,10);
	MessageBox(0,buf,TEXT("������ �������� BIG"),MB_ICONEXCLAMATION);
	delete(map);
	map=NULL;
	return false;
	}
else
if(map->Planshet->GetError())
	{
	MessageBox(0,TEXT("������ �������� ��������!"),TEXT("Planshet"),MB_ICONEXCLAMATION);
	delete(map);
	map=NULL;
	return false;
	}

//Set MapView Mode
MapViewMode=1;
drawer=map->Planshet;

/*
//Layers menu creation
ClearMenu();
int lc=map->GetLayersCnt();
for(int k=0;k<lc;k++)
		{
		NewMenuItem(map->GetLayerName(k),5000+k);
		if(map->GetLayerState(k))
		CheckMenu(5000+k);
		}

//Zoom menu initialization
CheckZoomMenu();
*/

return true;
}

//---------------------------------------------------------------------------
void __fastcall TForm1::MenuItem3Click(TObject *Sender)
{
//Open BIG file
if(!OpenDialog1->Execute())
	return;

if(!OpenBIG(OpenDialog1->FileName))
	ShowMessage(TEXT("������ �������� .Big!"));
else
	Invalidate();
}

//---------------------------------------------------------------------------

void __fastcall TForm1::FormPaint(TObject *Sender, TCanvas *Canvas, const TRectF &ARect)
{
	if (drawer)
	{
		Fmx::Graphics::TBitmap * bmp = new Fmx::Graphics::TBitmap(ImageViewer1->Width, ImageViewer1->Height);
		drawer->Draw(bmp);
		ImageViewer1->Bitmap = bmp;
	}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::FormResize(TObject *Sender)
{
	//

}
//---------------------------------------------------------------------------

