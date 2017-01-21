//---------------------------------------------------------------------------

#ifndef TPlanshetH
#define TPlanshetH

#include "TDrawable.h"

//---------------------------------------------------------------------------
class TPlanshet : public TDrawable
{
private:

		TDrawable* map;         //Main Big Map
		int LastError;          //LastError

protected:
		void CurPos(int& mCx,int& mCy,int& mW,int& mH);

public:
		BITMAP* bmp;            //Current planshet
		BITMAPINFO_256 bmi;     //Current planshet info

		TPlanshet(int Handle,TDrawable* BigMap,int w,int h);
		~TPlanshet();

		int Draw(HDC hdc,int w,int h);

		void Scroll(int dx,int dy,int w,int h);
		void SetCoord(int x,int y,int w,int h);
		void GoToCurPos(int w,int h);
		int GetWidth(){return bmi.bmiHeader.biWidth;};
		int GetHeight(){return bmi.bmiHeader.biHeight;};
		int GetError(){return LastError;};
};
#endif

