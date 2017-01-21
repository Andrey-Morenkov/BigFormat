//---------------------------------------------------------------------------

#ifndef TMapH
#define TMapH

#include "TPlanshet.h"

//---------------------------------------------------------------------------
class TMap : public TDrawable
{
private:
		unsigned int W;			//Full Image Width and Height
		unsigned int H;

		int Handle;     //BigFile Handle
		int LastError;  //is Big File was Opened sucessfully

		char str[15];
protected:
		int OpenBigFile(char* path,int& HBig);

public:
		BITMAP* bmp;			//Current image
		BITMAPINFO_256 bmi;     //Current image info

		TPlanshet* Planshet;

		TMap(char* filename,int w,int h, bool ReadPlanshet=true);
		~TMap();

		int Draw(HDC hdc,int w,int h);
		int SetZoom(int zoom,int sw, int sh);
		int GetZoom();

        void Scroll(int dx,int dy,int w,int h);
        void SetCoord(int x,int y,int w,int h);

        int GetWidth(){return W;};
        int GetHeight(){return H;};
        int GetError(){return LastError;};

		int GetLayersCnt();
		char* GetLayerName(int num);
		bool GetLayerState(int num);
		void SetLayerOn(int num);
		void SetLayerOff(int num);
		int GetHandle(){return Handle;};

};
#endif
