/*
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the UNPACK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// UNPACK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef UNPACK_EXPORTS
#define UNPACK_API __declspec(dllexport)
#else
#define UNPACK_API __declspec(dllimport)
#endif
*/

int decode (void * source, void * dest,	short int len);

int decode_x0 (void * source, void * dest,	short int len);
int decode_x1 (void * source, void * dest,	short int len);
int decode_x2 (void * source, void * dest,	short int len);
int decode_x4 (void * source, void * dest,	short int len);
int decode_x8 (void * source, void * dest,	short int len);

// scale: 0: 2:1 dest size 65536, row size 256 (!!!)
//				1: 1:1 dest size 16384, row size 128
//				2: 1:2 dest size 16384, row size 128
//				4: 1:4 dest size 16384, row size 128
//				8: 1:8 dest size 16384, row size 128
