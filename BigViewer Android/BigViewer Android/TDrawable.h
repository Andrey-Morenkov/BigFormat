//---------------------------------------------------------------------------

#ifndef TDrawableH
#define TDrawableH

#include <System.Classes.hpp>
#include <FMX.Graphics.hpp>

//---------------------------------------------------------------------------
struct BITMAPINFO_256
{
BITMAPINFOHEADER bmiHeader;
RGBQUAD bmiColors[256];
};

struct BITMAPINFO_MY
{
BITMAPINFOHEADER bmiHeader;
RGBQUAD* bmiColors;
};

class TDrawable
{
protected:
	int Cx;
	int Cy;

public:
		int virtual Draw(HDC hdc,int w,int h)=0;
        void virtual Scroll(int dx,int dy,int w,int h)=0;
        void virtual SetCoord(int x,int y,int w,int h)=0;

        int GetCx(){return Cx;};
		int GetCy(){return Cy;};

		void Draw(Fmx::Graphics::TBitmap * fmxbmp);
};

const int
        err_Ok=0,                       //No errors
        err_InvalParam=1001,            //Invalid parameters
		err_NoMem=1002;                  //No memory
#endif
