#pragma once
#ifndef __EXTRACT_DAT
#define __EXTRACT_DAT

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Text;
using namespace SimpleEndianBinaryIO;

namespace EXTRACT
{
    ref class Dat
    {
    public:
        Int32 DatAmount = 0;
        array<String^>^ DatFiles = nullptr;

        Dat(StreamWriter^ idxj, Stream^ readStream, UInt32 offsetStart, UInt32 length, String^ directory, String^ baseName, bool isDecmp)
        {
            EndianBinaryReader^ br = gcnew EndianBinaryReader(readStream, Endianness::BigEndian);
            br->BaseStream->Position = offsetStart;
            UInt32 amount = br->ReadUInt32();
            if (amount >= 0x010000)
            {
                Console::WriteLine("Invalid dat file!");
                return;
            }

            if (idxj != nullptr)
            {
                idxj->WriteLine("DAT_AMOUNT:" + amount);
            }
            DatAmount = static_cast<int>(amount);

            int blocklength = static_cast<int>(amount * 4u);

            array<Byte>^ offsetblock = gcnew array<Byte>(blocklength);
            array<Byte>^ nameblock = gcnew array<Byte>(blocklength);

            br->BaseStream->Position = offsetStart + 16;

            br->Read(offsetblock, 0, blocklength);
            br->Read(nameblock, 0, blocklength);

            array<KeyValuePair<int, String^>^>^ fileList = gcnew array<KeyValuePair<int, String^>^>(amount);

            int Temp = 0;
            for (int i = 0; i < amount; i++)
            {
                int offset = EndianBitConverter::ToInt32(offsetblock, Temp, Endianness::BigEndian);
                String^ format = Encoding::ASCII->GetString(nameblock, Temp, 4);
                format = ValidateFormat(format)->ToUpperInvariant();

                String^ FileFullName = Path::Combine(baseName, baseName + "_" + i.ToString("D3"));
                if (isDecmp)
                {
                    FileFullName = baseName + "_" + i.ToString("D3");
                }
                if (format->Length > 0)
                {
                    FileFullName += "." + format;
                }

                fileList[i] = gcnew KeyValuePair<int, String^>(offset, FileFullName);

                Temp += 4;
            }

            if (!Directory::Exists(Path::Combine(directory, baseName)) && !isDecmp)
            {
                try
                {
                    Directory::CreateDirectory(Path::Combine(directory, baseName));
                }
                catch (Exception^)
                {
                    Console::WriteLine("Failed to create directory: " + Path::Combine(directory, baseName));
                }
            }

            DatFiles = gcnew array<String^>(amount);

            for (int i = 0; i < fileList->Length; i++)
            {
                DatFiles[i] = fileList[i]->Value;

                int subFileLength;
                if (i < fileList->Length - 1)
                {
                    subFileLength = fileList[i + 1]->Key - fileList[i]->Key;
                }
                else
                {
                    subFileLength = static_cast<int>(length - fileList[i]->Key);
                }

                br->BaseStream->Position = offsetStart + fileList[i]->Key;

                array<Byte>^ endfile = gcnew array<Byte>(subFileLength);
                br->Read(endfile, 0, subFileLength);
                if (subFileLength > 0)
                {
                    try
                    {
                        File::WriteAllBytes(Path::Combine(directory, fileList[i]->Value), endfile);
                    }
                    catch (Exception^ ex)
                    {
                        Console::WriteLine(fileList[i]->Value + ": " + ex->Message);
                    }
                }

                String^ Line = "DAT_" + i.ToString("D3") + ":" + fileList[i]->Value;
                if (idxj != nullptr)
                {
                    idxj->WriteLine(Line);
                }
            }
        }

    private:
        String^ ValidateFormat(String^ source)
        {
            String^ res = "";
            for (int i = 0; i < source->Length; i++)
            {
                if ((source[i] >= 'A' && source[i] <= 'Z') ||
                    (source[i] >= 'a' && source[i] <= 'z') ||
                    (source[i] >= '0' && source[i] <= '9'))
                {
                    res += source[i];
                }
            }
            return res;
        }
    };
}

#endif