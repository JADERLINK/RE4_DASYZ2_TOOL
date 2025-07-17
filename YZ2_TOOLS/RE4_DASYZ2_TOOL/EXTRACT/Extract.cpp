#pragma once
#ifndef __Extract
#define __Extract

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";
#include "FileFormat.h"
#include "Dat.cpp"
#include "Udas.cpp"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;

namespace EXTRACT
{
    public ref class Extract
    {
    public:
        Extract(FileInfo^ info, FileFormat fileFormat)
        {
            FileStream^ stream = nullptr;
            StreamWriter^ idxj = nullptr;

            try
            {
                stream = info->OpenRead();
            }
            catch (Exception^ ex)
            {
                Console::WriteLine("Error: " + ex);
                return;
            }

            try
            {
                FileInfo^ idxjInfo = gcnew FileInfo(Path::ChangeExtension(info->FullName, ".idxbig"));
                idxj = idxjInfo->CreateText();
            }
            catch (Exception^ ex)
            {
                stream->Close();
                Console::WriteLine("Error: " + ex);
                return;
            }

            idxj->WriteLine("# github.com/JADERLINK");
            idxj->WriteLine("# youtube.com/@JADERLINK");
            idxj->WriteLine("# RE4 DASYZ2 TOOL By JADERLINK");
            idxj->WriteLine("TOOL_VERSION:V06");

            switch (fileFormat)
            {
            case FileFormat::DAT:
                idxj->WriteLine("FILE_FORMAT:DAT");
                break;
            case FileFormat::MAP:
                idxj->WriteLine("FILE_FORMAT:MAP");
                break;
            case FileFormat::UDAS:
                idxj->WriteLine("FILE_FORMAT:UDAS");
                break;
            case FileFormat::DAS:
                idxj->WriteLine("FILE_FORMAT:DAS");
                break;
            case FileFormat::DRS:
                idxj->WriteLine("FILE_FORMAT:DRS");
                break;
            case FileFormat::DECMP:
                idxj->WriteLine("FILE_FORMAT:DECMP");
                break;
            default:
                idxj->WriteLine("FILE_FORMAT:NULL");
                break;
            }

            String^ directory = info->Directory->FullName;
            String^ baseName = Path::GetFileNameWithoutExtension(info->Name);
            if (baseName->Length == 0)
            {
                baseName = "NULL";
            }

            if (fileFormat == FileFormat::DAT || fileFormat == FileFormat::MAP || fileFormat == FileFormat::DECMP)
            {
                try
                {
                    Dat^ a = gcnew Dat(idxj, stream, 0, (UInt32)info->Length, directory, baseName, fileFormat);

                    // Console
                    Console::WriteLine("FileCount = " + a->DatAmount);
                    if (a->DatFiles != nullptr)
                    {
                        for (int i = 0; i < a->DatFiles->Length; i++)
                        {
                            Console::WriteLine("File_" + i + " = " + a->DatFiles[i]);
                        }
                    }

                    if (a->ExtraRel != nullptr)
                    {
                        Console::WriteLine("ExtraRel = " + a->ExtraRel);
                    }

                    if (a->IsE3Version)
                    {
                        Console::WriteLine("IsE3Version = true");
                    }
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine("Error: " + ex);
                }
            }
            else if (fileFormat == FileFormat::UDAS || fileFormat == FileFormat::DAS || fileFormat == FileFormat::DRS)
            {
                try
                {
                    Udas^ a = gcnew Udas(idxj, stream, directory, baseName, fileFormat);

                    // Console
                    Console::WriteLine("FileCount = " + a->DatAmount);
                    if (a->DatFiles != nullptr)
                    {
                        for (int i = 0; i < a->DatFiles->Length; i++)
                        {
                            Console::WriteLine("File_" + i + " = " + a->DatFiles[i]);
                        }
                    }

                    if (a->ExtraRel != nullptr)
                    {
                        Console::WriteLine("ExtraRel = " + a->ExtraRel);
                    }

                    if (a->IsE3Version)
                    {
                        Console::WriteLine("IsE3Version = true");
                    }

                    if (a->hasYZ2 && a->YZ2Path != nullptr)
                    {
                        Console::WriteLine("Unable to extract YZ2 compression!!");
                        Console::WriteLine("YZ2DAT = " + a->YZ2Path);
                    }

                    Console::WriteLine("SoundFlag = " + a->SoundFlag);

                    if (a->SndPath != nullptr)
                    {
                        Console::WriteLine("File_SND = " + a->SndPath);
                    }
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine("Error: " + ex);
                }
            }

            stream->Close();
            idxj->Close();
        }

    };
}

#endif