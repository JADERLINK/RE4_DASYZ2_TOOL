#pragma once
#ifndef __CheckYZ2
#define __CheckYZ2

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";

#include "YZ2.h"
#include "Dat.cpp"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace SimpleEndianBinaryIO;

namespace EXTRACT
{
    ref class CheckYZ2
    {
    public:
        bool hasYZ2 = false;
        String^ YZ2Path = nullptr;
        Int32 DatAmount = 0;
        array<String^>^ DatFiles = nullptr;
        String^ ExtraRel = nullptr;

        CheckYZ2(StreamWriter^ idxj, Stream^ readStream, UInt32 offsetStart, UInt32 length, String^ directory, String^ baseName)
        {
            EndianBinaryReader^ br = gcnew EndianBinaryReader(readStream, System::Text::Encoding::UTF8, Endianness::BigEndian);
            br->BaseStream->Position = offsetStart;

            UInt32 amount = br->ReadUInt32();
            if (amount >= 0x010000)
            {
                hasYZ2 = true;
            }

            if (hasYZ2)
            {
                br->BaseStream->Position = offsetStart;
                array<Byte>^ in_header = gcnew array<Byte>(32);
                br->BaseStream->Read(in_header, 0, 32);

                bool HasInvalidChar = false;
                bool Has0x09 = false;
                bool Has0x0A = false;

                for (size_t i = 0; i < in_header->Length; i++)
                {
                    if (in_header[i] == 0x09 && Has0x09 == false)
                    {
                        Has0x09 = true;
                    }
                    else if (in_header[i] == 0x0A && Has0x0A == false)
                    {
                        Has0x0A = true;
                    }
                    else if (
                        (in_header[i] >= 0x01 && in_header[i] <= 0x2F)
                        || (in_header[i] >= 0x3A && in_header[i] <= 0x40)
                        || (in_header[i] >= 0x47 && in_header[i] <= 0x60)
                        || (in_header[i] >= 0x67 && in_header[i] <= 0xFF)
                        )
                    {
                        HasInvalidChar = true;
                    }
                }

                if (!HasInvalidChar && Has0x09 && Has0x0A) // valid YZ2
                {
                    br->BaseStream->Position = offsetStart;
                    array<Byte>^ udasYz2 = gcnew array<Byte>(length);
                    br->Read(udasYz2, 0, static_cast<int>(length));

                    array<Byte>^ datArr;

                    YZ2::YZ2Actions::YZ2Decode(udasYz2, datArr);

                    if (idxj != nullptr)
                    {
                        idxj->WriteLine("IS_DAT_COMPRESSED:true");
                    }

                    MemoryStream^ ms = gcnew MemoryStream(datArr);

                    Dat^ a = gcnew Dat(idxj, ms, 0, datArr->Length, directory, baseName, false);
                    DatAmount = a->DatAmount;
                    DatFiles = a->DatFiles;
                    ExtraRel = a->ExtraRel;
                    ms->Close();
                }
                else
                {
                    br->BaseStream->Position = offsetStart;
                    array<Byte>^ udasYz2 = gcnew array<Byte>(length);
                    br->Read(udasYz2, 0, static_cast<int>(length));

                    String^ FileFullName = Path::Combine(baseName, baseName + ".YZ2");
                    if (idxj != nullptr)
                    {
                        idxj->WriteLine("HAS_YZ2:true");
                        idxj->WriteLine("YZ2_PATH:" + FileFullName);
                    }

                    YZ2Path = FileFullName;

                    try
                    {
                        File::WriteAllBytes(Path::Combine(directory, FileFullName), udasYz2);
                    }
                    catch (Exception^ ex)
                    {
                        Console::WriteLine(FileFullName + ": " + ex->ToString());
                    }

                }
            }
        }
    };
}

#endif