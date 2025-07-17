#pragma once
#ifndef __NewDasYZ2
#define __NewDasYZ2

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";
#include "Dat.cpp"
#include "Udas.cpp"
#include "YZ2.h"

using namespace System;
using namespace System::IO;
using namespace SimpleEndianBinaryIO;

namespace REPACK
{
    ref class NewDasYZ2
    {
    public:
        NewDasYZ2(Stream^ stream, array<DatInfo^>^ dat, UdasInfo^ udasGroup, DatInfo^ ExtraRel, bool IS_E3_VERSION)
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

            array<Byte>^ yz2Bytes;
            bool hasYZ2 = false;
            {
                MemoryStream^ datMS = gcnew MemoryStream();
                Dat^ datWriter = gcnew Dat(datMS, dat, ExtraRel, 0, IS_E3_VERSION);

                YZ2::YZ2Actions::YZ2Encode(datMS->ToArray(), yz2Bytes);
                datMS->Close();

                unsigned int aLength = (unsigned int)(((yz2Bytes->Length + 31) / 32) * 32);
                udasGroup->DatFileAlignedBytesLength = aLength;
                udasGroup->DatFileRealBytesLength = (unsigned int)yz2Bytes->Length;

                hasYZ2 = true;
            }

            array<Byte>^ TopBytes = Udas::MakeUdasTop(udasGroup, hasEnd, false, hasYZ2);

            stream->Write(TopBytes, 0, TopBytes->Length);

            stream->Write(yz2Bytes, 0, yz2Bytes->Length);

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
    };
}

#endif