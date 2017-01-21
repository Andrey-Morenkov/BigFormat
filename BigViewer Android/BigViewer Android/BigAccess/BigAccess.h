#ifndef BigAccessH
#define BigAccessH
//������� ���������� BigAccess.dll
//���������� ������������ ���������� BigAccess.lib � ������
//��� ������ ���������� ������� BigAccess.dll � unpack.dll

//#define SPEC extern "C" __declspec(dllimport)

void LibAttach();
void LibDetach();

//------------------------------------------------------------------------------------
int InitBigAccess(int MaxCacheSize);
//�������������� ������� BigAccess.
//
//	MaxCacheSize - ����������� ���������� ������ ���� (� ������).

int OpenSource(char *SourceName, int *_Handle);
//��������� �������� (BIG-���� ��� INI-����). �������� ��� ����.
//
//	SourceName - ��� ��������� � ��������� ������� ���� (��������� ��
//		0-terminated ASCII ������).
//	_Handle - ��������� �� ����������, � ������� ����� ������� ����������
//		��������� ���������.

int CloseSource(int Handle);
//��������� �������� ����� �������� OpenSource ��������.
//
//	Handle - ���������� ������������ ���������.

int GetSourceSizes(int Handle, unsigned int *_BigPixelWidth, unsigned int *_BigPixelHeight);
//���������� �������� ������������ ������ ��������� � �������� � ������ ��������
//�������� (� ������������� �����������).
//
//	Handle - ���������� ���������.
//	_BigPixelWidth - ��������� �� ����������, � ������� ����� ������� ��������������
//		������ ������������ ������ ��������� (� ������������� �����������).
//	_BigPixelHeight - ��������� �� ����������, � ������� ����� ������� ������������
//		������ ������������ ������ ��������� (� ������������� �����������).

int SetZoomOut(int Handle, int _ZoomOut);
//������������� ������� ������� ��� ���������.
//
//	Handle - ���������� ���������.
//	ZoomOut - ����������� ����������������� ������ (�������).
//		��������� ��������: 1, 2, 4, 8, -2(����������).

int GetZoomOut(int Handle, int *_ZoomOut);
//���������� ������� ������� ��� ���������.
//
//	Handle - ���������� ���������.
//	_ZoomOut - ��������� �� ����������, � ������� ����� ������� ������� �����������
//		����������������� ������ (�������).
//		��������� ��������: 1, 2, 4, 8, -2(����������).

int GetImage(int Handle, int X0, int Y0, BITMAP *Bitmap);
// ! ����������, ������������� ������������ GetImage2
//��������� ������������ ������� ������������ ������ ��������� � �������� �����
//� ������� Windows Bitmap.
//
//	Handle - ���������� ���������.
//	X0 - �������� ������ �������� ���� ������� ������������ ������ ���������
//		(� ��������, ������� � 0, � ������������� �����������).
//	Y0 - �������� ������ �������� ���� ������� ������������ ������ ���������
//		(� ��������, ������� � 0, � ������������� �����������).
//	Bitmap - ������ �� ����������� �������� �������� ��������� ������� ������������
//		������. ��������� BITMAP ���������� � ������������ ����� wingdi.h ���������
//		�������:
//
//		typedef struct tagBITMAP {
//   			LONG   bmType;		// ���� �� ����������� ��� ����������
//   			LONG   bmWidth; 	// �������������� ������ �������
//						// (� ���������� �����������)
//   			LONG   bmHeight; 	// ������������ ������ �������
//						// (� ���������� �����������)
//   			LONG   bmWidthBytes; 	// ���������� ���������� ����
//						// �� ������ ����������� (������������
//						// �� ��������� 4)
//   			WORD   bmPlanes;	// ���� �� ����������� ��� ����������
//   			WORD   bmBitsPixel;	// ���� �� ����������� ��� ����������
//   			LPVOID bmBits; 		// ��������� �� ����������� �����,
//						// ������ �������� ������ ���� ��
//						// ����� bmWidthBytes*bmHeight ����
//		} BITMAP;

int GetImage2(int Handle, int X0, int Y0, BITMAP *Bitmap);
//����� ������, ������������� ������������ ������ GetImage
//��������� �� ��, ��
//
//	���� Bitmap->bmBitsPixel ������ ���� ����������� ����� �������
//	  8 ��� 256 ������,
//		24 ��� 16 ��� ������
//
//	� ������, ���� Bitmap->bmBitsPixel = 24, ����� Bitmap->bmBits ������ ����
//		������� �� ������� 3 ����� �� ������, � ������������� ������� ������ �� ��������
//		4 ����.


int GetStdImage(int Handle, int Row, int Col, char *AData);
//������������� ����������� ����� ������� ���������. ������� ������� 128x128 ��������,
//��������� ������ �������� ���� ������� (Col*128, Row*128).
//
//	Handle - ���������� ���������.
//	Row - ���������� �������� ������ �������� ���� ������� ������������ ������ ���������
//		(� ��������, ������� � 0, � ���������� �����������): Y = Row*128.
//	Col - ���������� �������� ������ �������� ���� ������� ������������ ������ ���������
//		(� ��������, ������� � 0, � ���������� �����������): X = Col*128.
//	AData - ��������� �� ���������� ������� ������ - ��������� ����� ������������ ������.
//		������ ���������� ������� ������ ���� 16384 ����, ���� �������� �� ������������,
//		� 16384*3 ����, ���� ������������. ���� �������� ������������, �� �������������
//		��������� ����� ������� ������� � �������: �����, �������, �������.

HPALETTE CreatePalette332(void);
//������� � ���������� handle ������� 3-3-2.

HPALETTE CreateGrayscalePalette(void);
//������� � ���������� handle ������� � 256 ���������� ������.

int GetPaletteData(int Handle, unsigned char *ColorTable);
//������� ������ ������� ��� ����������� ���������. ��� �������, �
//����������� �� �������� � idx-�����, ������� 3-3-2, Grayscale ��� �����������
//�������.
//
//	Handle - ���������� ���������.
//	ColorTable - ������ �� 256 ��������� �� 4 ����� (��� R, G, B, Z ���������
//		�������. ���� �������� ������������, �� ��������� ������� 3-3-2.

int GetPalette(int Handle, HPALETTE *HPalette);
//������� handle ������� ��� ����������� ���������. ��� �������, �
//����������� �� �������� � idx-�����, �������� ���� CreatePalette332, ����
//CreateGrayscalePalette.
//
//	Handle - ���������� ���������.
//	HPalette - ��������� �� ����������, � ������� ����� ������� handle
//		��������� �������. ���� �������� ������������, �� ��������� ������� 3-3-2.

int GetColorStruct(int Handle, unsigned char *Type);
//���������� ��� �������� ������ ���������.
//
//	Handle - ���������� ���������.
//	Type - ��������� �� ����������, � ������� ����� ������� ���
//		�������� ������:
//		0x01 - 8 bpp, ������� 3-3-2,
//		0x02 - 8 bpp, ������� - 256 �������� ������,
//		0x03 - 24 bpp, true color,
//    0x04 - 8bpp, ������������ �������
//  (����������� GetPalette ��� ���� �������)

int GetColorsNum(int Handle, int *ColorsNum);
//���������� ����� ������ � ������� (2^24 ��� truecolor) 
                                                                                       
int GetBaseCoords(int Handle, int X, int Y, double *BaseX, double *BaseY);
//��������� ������������� ���������� � �������� � ���������� ����.
//
//	Handle - ���������� ���������.
//	X - �������� ����� � �������� (������ � ���� �� ������ ���� � �������. �����.).
//	Y - �������� ����� � �������� (������ � ���� �� �������� ���� � �������. �����.).
//	BaseX - ��������� �� ����������, � ������� ����� �������� ��������
//		����� � ����������� ����.
//	BaseY - ��������� �� ����������, � ������� ����� �������� ��������
//		����� � ����������� ����.

int GetPixelCoords(int Handle, int *X, int *Y, double BaseX, double BaseY);
//��������� ���������� ���� � ���������� � ��������.
//
//	Handle - ���������� ���������.
//	X - ��������� �� ����������, � ������� ����� �������� ��������
//		����� � �������� (������ � ���� �� ������ ���� � �������. �����.).
//	Y - ��������� �� ����������, � ������� ����� �������� ��������
//		����� � �������� (������ � ���� �� �������� ���� � �������. �����.).
//	BaseX - �������� ����� � ����������� ����.
//	BaseY - �������� ����� � ����������� ����.

int GetSmallMapSizes(int Handle, unsigned int *_Width, unsigned int *_Height);
//���������� �������� ����������� �������� ���������.
//
//	Handle - ���������� ���������.
//	_Width - ��������� �� ����������, � ������� ����� ������� ��������������
//		������ ����������� �������� (� ��������).
//	_Height - ��������� �� ����������, � ������� ����� ������� ������������
//		������ ����������� �������� (� ��������).

int GetLargeMapSizes(int Handle, unsigned int *_Width, unsigned int *_Height);
//���������� �������� ����������� �������� ���������.
//
//	Handle - ���������� ���������.
//	_Width - ��������� �� ����������, � ������� ����� ������� ��������������
//		������ ����������� �������� (� ��������).
//	_Height - ��������� �� ����������, � ������� ����� ������� ������������
//		������ ����������� �������� (� ��������).

int GetSmallMap(int Handle, BITMAP *Bitmap);
//��������� ����������� ����� ����������� �������� � �������� �����
//� ������� Windows Bitmap.
//
//	Handle - ���������� ���������.
//	Bitmap - ������ �� ����������� �������� �������� ��������� ������������
//		������. ��������� BITMAP ���������� � ������������ �����
//		wingdi.h (��. ������� GetImage):

int GetLargeMap(int Handle, BITMAP *Bitmap);
//��������� ����������� ����� ����������� �������� � �������� �����
//� ������� Windows Bitmap.
//
//	Handle - ���������� ���������.
//	Bitmap - ������ �� ����������� �������� �������� ��������� ������������
//		������. ��������� BITMAP ���������� � ������������ �����
//		wingdi.h (��. ������� GetImage):

int GetLargeMapBlock(int Handle, int Row, int Col, char *PackedImage);
//������������ ��������� �������������� ������ � ����������� ��������. ������ ������
//128x128 ��������.
//
//	Handle - ���������� ���������.
//	Row - ����� ������ ������ (������ �� ����).
//	Col - ����� ������� ������ (������ �� ����).
//	PackedImage - ��������� �� ���������� ������� ������, ���� ����� �������
//		����������� ����. ������ ������������ ����� ����� �������� � �������
//		������� GetLargeMapBlockSize. ���������� ����� ����������� � �������
//		������� UnpackBlock.

int GetLargeMapBlockSize(int Handle, int Row, int Col, int *PackedSize);
//�������� ������ ������������ ����� ����������� ����������� ��������.
//
//	Handle - ���������� ���������.
//	Row - ����� ������ ������ (������ �� ����).
//	Col - ����� ������� ������ (������ �� ����).
//	PackedSize - ��������� �� ����������, ���� ����� ������� ������ ����������������
//		������������ ����� �����������.

int GetLayersList(int Handle, int *_LayerCount, char *LayerNames);
//���������� ���������� ����� � �������� ������� ���� � ���������.
//
//	Handle - ���������� ���������.
//	_LayerCount - ��������� �� ����������, � ������� ����� �������� ���������� �����.
//		���� � ��������� ������������ ������ �������� ����, �� � *_LayerCount �����
//		������� ����.
//	LayerNames - ������ �� 15*15=225 ��������� ���� char (��������� �������������).
//		� ������ ��������� ���������� LayerNames+i*15 - ��������� ��
//		NULL-terminated ASCII ������ � ��������� ���� ����� i (��������� � ����).

int GetLayersPriority(int Handle, int *_Priorities);
//���������� ���������� � ����������� �����.
//
//	Handle - ���������� ���������.
//	_Priorities - ������ ��������� ���� int, � ������� ����� �������� ����������
//		��������������� �����.

int GetLayerMixInfo(int Handle, int LayerIndex, char *Style, char *UnicColor);
//���������� ���������� � ����, ������� ������������ ��� ������������.
//
//	Handle - ���������� ���������.
//	LayerIndex - ����� ����, ������� � ����.
//	Style - ��������� �� ����������, � ������� ����� �������
//		������ �������� ������������.
//	UnicColor - ��������� �� ����������, � ������� ����� �������
//		������ �������� ������������.

int MixTwoLayers(unsigned char *Dest, unsigned char *Src, char Style, char UnicColor);
//��������� ��� ����, �������������� � ���� ������ 128x128 �������� ��������.
//
//	Dest - ������ 16384 ����, ������������ ����� ����������� ������� ���� (128x128),
//		�� ������� ��������� ����������� ������� ���� (Src). ������ ���� ����������
//		���� ������� - ������ � ������� ������ (�������).
//	Src - ������ 16384 ����, ������������ ����� ����������� ������� ���� (128x128),
//		������� ��������� �� ����������� ������� ���� (Dest). ������ ���� ����������
//		���� ������� - ������ � ������� ������ (�������).
//	Style - ������ �������� ������������, ������������ ��� ������� ����.
//	UnicColor - ������ �������� ������������, ������������ ���� ��� ������� ����.

int SetLayers(int Handle, unsigned int Layers);
//������������� ����� ������������ ����� ��� ���������.
//
//	Handle - ���������� ���������.
//	Layers - ������� ����� ������������ �����. ��� ����� i ������������� ���� �����
//		i (��������� � ����). �������� �����:
//			0 - ���� �� ������������;
//			1 - ���� ������������.
//		����, ��������������� �������������� �����, ������ ���� ��������. ��������
//		����� 10 �����, ����� �������� 0, 1, 7, 9 ����, � ��������� ���������, �����
//		Layers = 00000000 00000000 00000010 10000011.
//		����
//		Layers = 00000000 00000000 00000110 10000011, �� ������� ��������� ���
//		������ rcLayerNotPresent.

int GetLayers(int Handle, unsigned int *Layers);
//���������� ����� ������������ ����� ��� ���������.
//
//	Handle - ���������� ���������.
//	Layers - ��������� �� ����������, � ������� ����� ������� ������� �����
//		������������ ����� (� ������� SetLayers).

int UnpackBlock(char *Dst, char *Src, int PackedSize);
//������������ ���������� ����� �����������.
//
//	Dst - ��������� �� ���������� ������� ������, ���� ����� ������� �������������
//		����. ������ ������� - 16384 �����.
//	Src - ��������� ������� ������, ��� ���������� ����������� ����. ������ ������� -
//		PackedSize ����.
//	PackedSize - ������ ������������ �����.


//------------------------------------------------------------------------------------

//���� ��������� �������.

const int
	rcOk = 0,			// �������� ���������� ������ �������
	rcSrcAlreadyOpened = -101,	// ������� ������� ��������, 
					// �� ������ ����� ��������
					// (� ������ ������ ���������� �� ������������)
	rcSrcAlreadyClosed = -102,	// ������� ������ � ���������� ����������
	rcCantOpenIdxFile = -103,	// ���������� ������� ��������� idx-����
	rcCantOpenBigFile = -104,	// ���������� ������� ��������� big-����
	rcInvalidIniFile = -105,	// ini-���� ���������
	rcMemoryError = -106,		// ������ � ������ � �������
	rcCantIncludeUnpackDll = -107,	// �� ������� ���������� unpack.dll
	rcInvalidUnpackDll = -108,	// � unpack.dll �� ���������� ������� decode
	rcDiskError = -109,		// ������ � �����
	rcUnpackError = -110,		// ������ ������������� ������
	rcZoomNotImplemented = -111,	// �������� �������� ZoomOut
	rcIniNotOpened = -112,		// ������� ��������������� ��������
					// GetBaseCoords ��� GetPixelCoords,
					// �� ������ ��������������� ini-����
	rcNoMapPresent = -113,		// ������������ ��������� ������� �
					// � ���������, ���� �������� �������� 
					// �� �������� ���������
	rcLayerNotPresent = -114,	// ������� �������� �������������� ����
	rcSystemAlreadyInited = -115,	// ������� ������������ ������������� �������
	rcSystemNotInited = -116,	// ������� ������ � �������������������� �������
	rcTooManyOpenedSources = -117,	// ������� ��������� ���������� ���������� 
					// ������������ �������� ���������� (� ������ 
					// ������ - 50)
	rcFatalError = -118,		// ������ ������
	rcBppNotImplemented = -119,	// ������ ������ ������������� �������� �� 
					// ��������������
	rcInvalidBpp = -120,		// idx-���� ���������
	rcNoPalette = -121;			// ������� �������� ������ ���������, ���� 
					// ������� �� ������������ (�������� � ������ True Color)
#endif