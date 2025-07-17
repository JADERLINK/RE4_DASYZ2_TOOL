#pragma once
#ifndef __REPACK_UDAS
#define __REPACK_UDAS

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";
#include "Dat.cpp";

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace SimpleEndianBinaryIO;

namespace REPACK
{
    public ref class UdasInfo
    {
    public:
        UInt32 DatFileAlignedBytesLength = 0;
        UInt32 DatFileRealBytesLength = 0;
        int SoundFlag = 4;
        DatInfo^ Top = gcnew DatInfo();
        DatInfo^ Middle = gcnew DatInfo();
        DatInfo^ End = gcnew DatInfo();
    };

    ref class Udas
    {
    public:
        Udas(Stream^ stream, array<DatInfo^>^ dat, UdasInfo^ udasGroup, bool isDRS, DatInfo^ ExtraRel, bool IS_E3_VERSION)
        {
            array<Byte>^ EndBytes = gcnew array<Byte>(udasGroup->End->Length);
            array<Byte>^ MiddleBytes = gcnew array<Byte>(udasGroup->Middle->Length);
            bool hasEnd = false;

            if (udasGroup->End->FileExits)
            {
                try
                {
                    BinaryReader^ br = gcnew BinaryReader(udasGroup->End->fileInfo->OpenRead());
                    br->BaseStream->Read(EndBytes, 0, (int)udasGroup->End->fileInfo->Length);
                    br->Close();
                    hasEnd = true;
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine("Error to read file: " + udasGroup->End->fileInfo->Name);
                    Console::WriteLine(ex);
                }
            }

            if (udasGroup->Middle->FileExits)
            {
                try
                {
                    BinaryReader^ br = gcnew BinaryReader(udasGroup->Middle->fileInfo->OpenRead());
                    br->BaseStream->Read(MiddleBytes, 0, (int)udasGroup->Middle->fileInfo->Length);
                    br->Close();
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine("Error to read file: " + udasGroup->Middle->fileInfo->Name);
                    Console::WriteLine(ex);
                }
            }

            array<Byte>^ TopBytes = MakeUdasTop(udasGroup, hasEnd, isDRS, dat->Length > 0);

            stream->Write(TopBytes, 0, TopBytes->Length);

            Dat^ datWriter = gcnew Dat(stream, dat, ExtraRel, TopBytes->Length, IS_E3_VERSION);

            if (MiddleBytes->Length != 0)
            {
                stream->Position = udasGroup->Middle->Offset;
                stream->Write(MiddleBytes, 0, MiddleBytes->Length);
            }

            if (EndBytes->Length != 0)
            {
                stream->Position = udasGroup->End->Offset;
                stream->Write(EndBytes, 0, EndBytes->Length);
            }
        }

        static array<Byte>^ MakeUdasTop(UdasInfo^ udasGroup, bool hasEnd, bool isDRS, bool hasDat)
        {
            array<Byte>^ TopBytes;

            if (udasGroup->Top->FileExits)
            {
                try
                {
                    TopBytes = gcnew array<Byte>(udasGroup->Top->Length);

                    BinaryReader^ br = gcnew BinaryReader(udasGroup->Top->fileInfo->OpenRead());
                    br->BaseStream->Read(TopBytes, 0, (int)udasGroup->Top->fileInfo->Length);
                    br->Close();

                    if (TopBytes->Length < 0x80)
                    {
                        TopBytes = MakerNewTopBytes(hasEnd, hasDat, udasGroup->SoundFlag, isDRS);
                        Console::WriteLine("Top file is less than 0x80 in size. It was replaced with a new one.");
                    }
                }
                catch (Exception^ ex)
                {
                    TopBytes = MakerNewTopBytes(hasEnd, hasDat, udasGroup->SoundFlag, isDRS);
                    Console::WriteLine("Error to read file: " + udasGroup->Top->fileInfo->Name);
                    Console::WriteLine(ex);
                }
            }
            else
            {
                TopBytes = MakerNewTopBytes(hasEnd, hasDat, udasGroup->SoundFlag, isDRS);
            }

            UInt32 firtPosition = EndianBitConverter::ToUInt32(TopBytes, 0x2C, Endianness::BigEndian);
            if (firtPosition != TopBytes->Length)
            {
                array<Byte>^ b = EndianBitConverter::GetBytes((UInt32)TopBytes->Length, Endianness::BigEndian);
                TopBytes[0x2C] = b[0];
                TopBytes[0x2D] = b[1];
                TopBytes[0x2E] = b[2];
                TopBytes[0x2F] = b[3];
                firtPosition = (UInt32)TopBytes->Length;
            }

            udasGroup->Middle->Offset = firtPosition + udasGroup->DatFileAlignedBytesLength;
            udasGroup->End->Offset = udasGroup->Middle->Offset + (UInt32)udasGroup->Middle->Length;

            if (hasDat)
            {
                UInt32 firstType = EndianBitConverter::ToUInt32(TopBytes, 0x20, Endianness::BigEndian);
                if (firstType != 0)
                {
                    TopBytes[0x20] = 0;
                    TopBytes[0x21] = 0;
                    TopBytes[0x22] = 0;
                    TopBytes[0x23] = 0;
                }

                array<Byte>^ datlength = EndianBitConverter::GetBytes((UInt32)udasGroup->DatFileRealBytesLength, Endianness::BigEndian);
                TopBytes[0x24] = datlength[0];
                TopBytes[0x25] = datlength[1];
                TopBytes[0x26] = datlength[2];
                TopBytes[0x27] = datlength[3];

                if (hasEnd)
                {
                    array<Byte>^ endOffset = EndianBitConverter::GetBytes((UInt32)udasGroup->End->Offset, Endianness::BigEndian);
                    TopBytes[0x4C] = endOffset[0];
                    TopBytes[0x4D] = endOffset[1];
                    TopBytes[0x4E] = endOffset[2];
                    TopBytes[0x4F] = endOffset[3];

                    array<Byte>^ endLength = EndianBitConverter::GetBytes((UInt32)udasGroup->End->Length, Endianness::BigEndian);
                    TopBytes[0x44] = endLength[0];
                    TopBytes[0x45] = endLength[1];
                    TopBytes[0x46] = endLength[2];
                    TopBytes[0x47] = endLength[3];

                    UInt32 secondType = EndianBitConverter::ToUInt32(TopBytes, 0x40, Endianness::BigEndian);
                    if (secondType == 0xFFFFFFFF)
                    {
                        array<Byte>^ SoundFlag = EndianBitConverter::GetBytes((Int32)udasGroup->SoundFlag, Endianness::BigEndian);
                        TopBytes[0x40] = SoundFlag[0];
                        TopBytes[0x41] = SoundFlag[1];
                        TopBytes[0x42] = SoundFlag[2];
                        TopBytes[0x43] = SoundFlag[3];
                    }

                    TopBytes[0x60] = 0xFF;
                    TopBytes[0x61] = 0xFF;
                    TopBytes[0x62] = 0xFF;
                    TopBytes[0x63] = 0xFF;
                }
                else
                {
                    TopBytes[0x40] = 0xFF;
                    TopBytes[0x41] = 0xFF;
                    TopBytes[0x42] = 0xFF;
                    TopBytes[0x43] = 0xFF;
                }
            }
            else
            {
                if (hasEnd)
                {
                    array<Byte>^ endOffset = EndianBitConverter::GetBytes((UInt32)udasGroup->End->Offset, Endianness::BigEndian);
                    TopBytes[0x2C] = endOffset[0];
                    TopBytes[0x2D] = endOffset[1];
                    TopBytes[0x2E] = endOffset[2];
                    TopBytes[0x2F] = endOffset[3];

                    array<Byte>^ endLength = EndianBitConverter::GetBytes((UInt32)udasGroup->End->Length, Endianness::BigEndian);
                    TopBytes[0x24] = endLength[0];
                    TopBytes[0x25] = endLength[1];
                    TopBytes[0x26] = endLength[2];
                    TopBytes[0x27] = endLength[3];

                    UInt32 secondType = EndianBitConverter::ToUInt32(TopBytes, 0x20, Endianness::BigEndian);
                    if (secondType == 0xFFFFFFFF || secondType == 0)
                    {
                        array<Byte>^ SoundFlag = EndianBitConverter::GetBytes((Int32)udasGroup->SoundFlag, Endianness::BigEndian);
                        TopBytes[0x20] = SoundFlag[0];
                        TopBytes[0x21] = SoundFlag[1];
                        TopBytes[0x22] = SoundFlag[2];
                        TopBytes[0x23] = SoundFlag[3];
                    }

                    TopBytes[0x40] = 0xFF;
                    TopBytes[0x41] = 0xFF;
                    TopBytes[0x42] = 0xFF;
                    TopBytes[0x43] = 0xFF;
                }
                else
                {
                    TopBytes[0x20] = 0xFF;
                    TopBytes[0x21] = 0xFF;
                    TopBytes[0x22] = 0xFF;
                    TopBytes[0x23] = 0xFF;
                }
            }

            return TopBytes;
        }

    private:
        static array<Byte>^ MakerNewTopBytes(bool hasEnd, bool hasDat, int SoundFlag, bool isDRS)
        {
            array<Byte>^ top = gcnew array<Byte>(0x400);

            if (isDRS)
            {
                array<Byte>^ data = {
                    0x83, 0x6E, 0x83, 0x4A, 0x83, 0x5A, 0x82, 0xCC,
                    0x83, 0x41, 0x83, 0x7A, 0x81, 0x5B, 0x81, 0x5B,
                    0x81, 0x5B, 0x81, 0x5B, 0x81, 0x5B, 0x81, 0x5B,
                    0x81, 0x5B, 0x81, 0x49, 0x81, 0x49, 0x81, 0x49
                };
                Array::Copy(data, 0, top, 0, data->Length);
            }
            else
            {
                int temp = 0;
                for (int i = 0; i < 8; i++)
                {
                    top[temp++] = 0xCA;
                    top[temp++] = 0xB6;
                    top[temp++] = 0xBE;
                    top[temp++] = 0x20;
                }
            }

            top[0x2E] = 0x04;

            if (hasDat && hasEnd)
            {
                array<Byte>^ soundFlag = EndianBitConverter::GetBytes((Int32)SoundFlag, Endianness::BigEndian);
                top[0x40] = soundFlag[0];
                top[0x41] = soundFlag[1];
                top[0x42] = soundFlag[2];
                top[0x43] = soundFlag[3];
                top[0x60] = top[0x61] = top[0x62] = top[0x63] = 0xFF;
            }
            else if (hasDat && !hasEnd)
            {
                top[0x40] = top[0x41] = top[0x42] = top[0x43] = 0xFF;
            }
            else if (!hasDat && hasEnd)
            {
                array<Byte>^ soundFlag = EndianBitConverter::GetBytes((Int32)SoundFlag, Endianness::BigEndian);
                top[0x20] = soundFlag[0];
                top[0x21] = soundFlag[1];
                top[0x22] = soundFlag[2];
                top[0x23] = soundFlag[3];
                top[0x40] = top[0x41] = top[0x42] = top[0x43] = 0xFF;
            }
            else
            {
                top[0x20] = top[0x21] = top[0x22] = top[0x23] = 0xFF;
            }

            return top;
        }
    };

}

#endif