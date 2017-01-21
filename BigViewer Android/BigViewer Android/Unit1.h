//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Menus.hpp>
#include <FMX.Types.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Dialogs.hpp>

#include "TMap.h"
#include <FMX.ExtCtrls.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Objects.hpp>

//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
	TMainMenu *MainMenu1;
	TMenuItem *MenuItem1;
	TMenuItem *MenuItem2;
	TMenuItem *MenuItem3;
	TOpenDialog *OpenDialog1;
	TImageViewer *ImageViewer1;
	void __fastcall MenuItem2Click(TObject *Sender);
	void __fastcall MenuItem3Click(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall FormPaint(TObject *Sender, TCanvas *Canvas, const TRectF &ARect);
	void __fastcall FormResize(TObject *Sender);


private:	// User declarations
	TMap* map;
	TDrawable*      drawer; //map or planshet drawer
	BYTE MapViewMode;   	//0=Nothing; 1=Planshet; 2=M0ap;
	AnsiString filepath;
protected:
	bool __fastcall OpenBIG(AnsiString fn);
public:		// User declarations
	__fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
