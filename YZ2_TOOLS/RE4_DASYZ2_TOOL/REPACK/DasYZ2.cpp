#pragma once
#ifndef __DasYZ2
#define __DasYZ2

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";
#include "Dat.cpp"
#include "Udas.cpp"

using namespace System;
using namespace System::IO;
using namespace SimpleEndianBinaryIO;

namespace REPACK
{
    ref class DasYZ2
    {
    public:
        DasYZ2(Stream^ stream, DatInfo^ yz2, UdasInfo^ udasGroup)
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
                    Console::WriteLine("Error to read file: " + udasGroup->End->fileInfo->Name + Environment::NewLine + " ex: " + ex);
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
                    Console::WriteLine("Error to read file: " + udasGroup->Middle->fileInfo->Name + Environment::NewLine + " ex: " + ex);
                }
            }

            array<Byte>^ yz2Bytes = gcnew array<Byte>(yz2->Length);
            bool hasYZ2 = false;

            if (yz2->FileExits)
            {
                try
                {
                    BinaryReader^ br = gcnew BinaryReader(yz2->fileInfo->OpenRead());
                    br->BaseStream->Read(yz2Bytes, 0, (int)yz2->fileInfo->Length);
                    br->Close();
                    hasYZ2 = true;
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine("Error to read file: " + udasGroup->Middle->fileInfo->Name + Environment::NewLine + " ex: " + ex);
                }
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