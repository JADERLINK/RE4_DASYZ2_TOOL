#pragma once
#ifndef __REPACK_DAT
#define __REPACK_DAT

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";

using namespace System;
using namespace System::IO;
using namespace System::Text;
using namespace SimpleEndianBinaryIO;

namespace REPACK
{
    ref class DatInfo
    {
    public:
        String^ Path = nullptr;
        FileInfo^ fileInfo = nullptr;
        String^ Extension = nullptr;
        int Length = 0;
        UInt32 Offset = 0;
        bool FileExits = false;
    };

    ref class Dat
    {
    public:
        Dat(Stream^ stream, UInt32 DatHeaderLength, array<DatInfo^>^ dat)
        {
            array<Byte>^ headerCont = gcnew array<Byte>(16);
            array<Byte>^ Amount = EndianBitConverter::GetBytes(dat->Length, Endianness::BigEndian);
            headerCont[0] = Amount[0];
            headerCont[1] = Amount[1];
            headerCont[2] = Amount[2];
            headerCont[3] = Amount[3];
            stream->Write(headerCont, 0, 16);

            for (int i = 0; i < dat->Length; i++)
            {
                array<Byte>^ offset = EndianBitConverter::GetBytes(dat[i]->Offset, Endianness::BigEndian);
                stream->Write(offset, 0, 4);
            }

            for (int i = 0; i < dat->Length; i++)
            {
                array<Byte>^ name = Encoding::ASCII->GetBytes(dat[i]->Extension);
                stream->Write(name, 0, 4);
            }

            array<Byte>^ complete = gcnew array<Byte>(DatHeaderLength - (16 + (4 * dat->Length * 2)));

            if (complete->Length > 0)
            {
                stream->Write(complete, 0, complete->Length);
            }

            for (int i = 0; i < dat->Length; i++)
            {
                array<Byte>^ archive = gcnew array<Byte>(dat[i]->Length);
                try
                {
                    if (dat[i]->FileExits)
                    {
                        BinaryReader^ br = gcnew BinaryReader(dat[i]->fileInfo->OpenRead());
                        br->BaseStream->Read(archive, 0, (int)dat[i]->fileInfo->Length);
                        br->Close();
                    }
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine("Error to read file: " + dat[i]->fileInfo->Name + Environment::NewLine + " ex: " + ex);
                }
                stream->Write(archive, 0, archive->Length);
            }
        }
    };
}

#endif