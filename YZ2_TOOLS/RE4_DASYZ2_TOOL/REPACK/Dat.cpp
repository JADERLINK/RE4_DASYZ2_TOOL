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
        Dat(Stream^ stream, array<DatInfo^>^ dat, DatInfo^ ExtraRel, Int64 StartOffset, bool IS_E3_VERSION)
        {
            stream->Position = StartOffset;

            array<Byte>^ headerCont;

            if (IS_E3_VERSION == false) // versão final
            {
                headerCont = gcnew array<Byte>(16);
                EndianBitConverter::GetBytes((UInt32)dat->Length, Endianness::BigEndian)->CopyTo(headerCont, 0); //Amount

                if (ExtraRel != nullptr)
                {
                    EndianBitConverter::GetBytes(ExtraRel->Offset, Endianness::BigEndian)->CopyTo(headerCont, 4); //ExtraRelOffset
                }
            }
            else // é E3
            {
                headerCont = gcnew array<Byte>(4);
                EndianBitConverter::GetBytes((UInt32)dat->Length, Endianness::BigEndian)->CopyTo(headerCont, 0); //Amount

                if (ExtraRel != nullptr)
                {
                    EndianBitConverter::GetBytes(ExtraRel->Offset, Endianness::BigEndian)->CopyTo(headerCont, 0); //ExtraRelOffset, nesse caso, se tiver, sobrepõem o amount
                }
            }

            stream->Write(headerCont, 0, headerCont->Length);

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

            for (int i = 0; i < dat->Length; i++)
            {
                stream->Position = StartOffset + dat[i]->Offset;

                try
                {
                    if (dat[i]->FileExits)
                    {
                        auto reader = dat[i]->fileInfo->OpenRead();
                        reader->CopyTo(stream);
                        reader->Close();
                    }
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine("Error to read file: " + dat[i]->fileInfo->Name);
                    Console::WriteLine(ex);
                }
            }

            if (ExtraRel != nullptr)
            {
                stream->Position = StartOffset + ExtraRel->Offset;

                try
                {
                    if (ExtraRel->FileExits)
                    {
                        auto reader = ExtraRel->fileInfo->OpenRead();
                        reader->CopyTo(stream);
                        reader->Close();
                    }
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine("Error to read file: " + ExtraRel->fileInfo->Name);
                    Console::WriteLine(ex);
                }
            }

        }
    };
}

#endif