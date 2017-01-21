//---------------------------------------------------------------------------

#pragma hdrstop

#include "TDrawable.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

#include <Vcl.Graphics.hpp>
#include <System.Classes.hpp>

void TDrawable::Draw(Fmx::Graphics::TBitmap * fmxbmp)
{
	HDC srcDC = GetDC(NULL);
	HDC memDC = CreateCompatibleDC(srcDC);
	HBITMAP bmp = CreateCompatibleBitmap(srcDC, fmxbmp->Width, fmxbmp->Height);
	ReleaseDC(NULL, srcDC);
	HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, bmp);

	Draw(memDC, fmxbmp->Width, fmxbmp->Height);

	SelectObject(memDC, oldBitmap);

	{
		Vcl::Graphics::TBitmap * vclbmp = new Vcl::Graphics::TBitmap();
		vclbmp->Width = fmxbmp->Width;
		vclbmp->Height = fmxbmp->Height;
		vclbmp->Handle = bmp;

		System::Classes::TMemoryStream * memstream = new System::Classes::TMemoryStream();
		vclbmp->SaveToStream(memstream);

		fmxbmp->LoadFromStream(memstream);

		delete memstream;
		delete vclbmp;
	}

	ReleaseDC(NULL, memDC);
}

