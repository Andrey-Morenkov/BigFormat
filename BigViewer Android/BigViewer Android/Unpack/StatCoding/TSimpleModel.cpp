// TSimpleModel.cpp: implementation of the TSimpleModel class.
//
//////////////////////////////////////////////////////////////////////
//#include <stdafx.h>

#include "TSimpleModel.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TSimpleModel::TSimpleModel(_TAnyCoder * _Coder):Coder(_Coder)
{

}

TSimpleModel::~TSimpleModel()
{

}

void TSimpleModel::EncodeArray(void *buf, int size)
{
  for (int i = 0; i < size; i++)
    Encode(((uint8 *)buf)[i]);
}

void TSimpleModel::DecodeArray(void *buf, int size)
{
  for (int i = 0; i < size; i++)
    ((uint8 *)buf)[i] = Decode();
}

void TSimpleModel::EncodeZArray(void *buf, int size)
{
  int z = 0;

  for (int i = 0; i < size; i++)
    if (((uint8 *)buf)[i])
    {
      if (z)
        EncodeZUInt(z - 1);
      z = 0;
      Encode(((uint8 *)buf)[i]);
    }
    else
    {
      if (! z)
        Encode(((uint8 *)buf)[i]);
      z ++;
    }

  if (z)
    EncodeZUInt(z - 1);
}

void TSimpleModel::DecodeZArray(void *buf, int size)
{
  int z = 0;

  for (int i = 0; i < size; i++)
    if (z)
    {
      ((uint8 *)buf)[i] = 0;
      z --;
    }
    else
    {
      ((uint8 *)buf)[i] = Decode();
      if (! ((uint8 *)buf)[i])
        z = DecodeZUInt();
    }
}

void TSimpleModel::EncodeZUInt(unsigned long val)
{
  if (val > 0xFF)
  {
    EncodeBit(1);
    if (val > 0xFFFF)
    {
      EncodeBit(1);
      EncodeWide((unsigned short)val);
      EncodeWide((unsigned short)(val >> 16));
    }
    else
    {
      EncodeBit(0);
      EncodeWide((unsigned short)val);
    }
  }
  else
  {
    EncodeBit(0);
    Encode((unsigned char)val);
  }
}

unsigned long TSimpleModel::DecodeZUInt(void)
{
  unsigned long val;

  if (DecodeBit())
  {
    if (DecodeBit())
    {
      val = DecodeWide();
      val |= DecodeWide() << 16;
    }
    else
    {
      val = DecodeWide();
    }
  }
  else
  {
    val = Decode();
  }

  return val;
}

void TSimpleModel::EncodeZInt(signed long val)
{
  if (val < 0)
  {
    EncodeBit(1);
    EncodeZUInt(- val);
  }
  else
  {
    EncodeBit(0);
    EncodeZUInt(val);
  }
}

signed long TSimpleModel::DecodeZInt(void)
{
  if (DecodeBit())
    return -DecodeZUInt();
  else
    return DecodeZUInt();
}

