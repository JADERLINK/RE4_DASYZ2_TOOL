#pragma once
#ifndef __EXTRACT_UDAS
#define __EXTRACT_UDAS

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";
#include "CheckYZ2.cpp"
#include "Dat.cpp"

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace SimpleEndianBinaryIO;

namespace EXTRACT
{
    private value struct UdasData
    {
        UInt32 type;
        UInt32 offset;
        UInt32 length;
    };

    ref class Udas
    {
    public:
        int SoundFlag = -1;
        int DatAmount = 0;
        array<String^>^ DatFiles = nullptr;
        String^ ExtraRel = nullptr;
        String^ SndPath = nullptr;
        bool hasYZ2 = false;
        String^ YZ2Path = nullptr;
        bool IsE3Version = false;

        Udas(StreamWriter^ idxj, Stream^ readStream, String^ directory, String^ baseName, FileFormat fileFormat)
        {
            EndianBinaryReader^ br = gcnew EndianBinaryReader(readStream, Endianness::BigEndian);

            List<UdasData>^ UdasList = gcnew List<UdasData>();

            UInt32 temp = 0x20;
            for (int i = 0; i < 2; i++)
            {
                br->Position = temp;

                UInt32 u_Type = br->ReadUInt32();
                UInt32 u_DataSize = br->ReadUInt32();
                br->ReadUInt32(); // Unused
                UInt32 u_DataOffset = br->ReadUInt32();

                if (u_Type == 0xFFFFFFFF)
                {
                    break;
                }

                // Adiciona � lista
                UdasList->Add(UdasData{ u_Type, u_DataOffset, u_DataSize });

                temp += 32;
            }

            if (UdasList->Count == 0 || UdasList[0].offset >= readStream->Length || UdasList[0].offset >= 0x010000)
            {
                Console::WriteLine("Error extracting file, first offset is invalid!");
                return;
            }

            if (UdasList->Count >= 1) // UDAS_TOP
            {
                if (!Directory::Exists(Path::Combine(directory, baseName)))
                {
                    try
                    {
                        Directory::CreateDirectory(Path::Combine(directory, baseName));
                    }
                    catch (Exception^ ex)
                    {
                        Console::WriteLine("Failed to create directory: " + Path::Combine(directory, baseName));
                        Console::WriteLine(ex);
                        return;
                    }
                }

                int udasTopLength = (int)UdasList[0].offset;
                array<Byte>^ udasTop = gcnew array<Byte>(udasTopLength);

                br->Position = 0;
                br->Read(udasTop, 0, udasTopLength);

                String^ fileFullName = Path::Combine(baseName, baseName + "_TOP.HEX");
                idxj->WriteLine("!UDAS_TOP:" + fileFullName);

                try
                {
                    File::WriteAllBytes(Path::Combine(directory, fileFullName), udasTop);
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine(fileFullName + ": " + ex);
                }
            }

            if (UdasList->Count == 1) // UDAS_MIDDLE
            {
                int length = (int)UdasList[0].length;
                int startoffset = (int)UdasList[0].offset;
                int maxlength = (int)br->Length;
                int newOffset = startoffset + length;
                int newlength = maxlength - newOffset;

                if (newlength > 0 && newOffset < readStream->Length)
                {
                    array<Byte>^ udasMiddle = gcnew array<Byte>(newlength);

                    br->Position = newOffset;
                    br->Read(udasMiddle, 0, newlength);

                    String^ fileFullName = Path::Combine(baseName, baseName + "_MIDDLE.HEX");
                    idxj->WriteLine("!UDAS_MIDDLE:" + fileFullName);

                    try
                    {
                        File::WriteAllBytes(Path::Combine(directory, fileFullName), udasMiddle);
                    }
                    catch (Exception^ ex)
                    {
                        Console::WriteLine(fileFullName + ": " + ex);
                    }
                }
            }

            bool readedDat = false;
            bool readedSnd = false;

            for (int i = 0; i < UdasList->Count; i++)
            {
                UInt32 type = UdasList[i].type;

                // type == 0xFFFFFFFF : none

                if (type == 0x0 && !readedDat)
                {
                    // DAT
                    UInt32 length = UdasList[i].length;
                    UInt32 startOffset = UdasList[i].offset;

                    //checar YZ2
                    CheckYZ2^ yz2 = gcnew CheckYZ2(idxj, readStream, startOffset, length, directory, baseName, fileFormat);
                    hasYZ2 = yz2->hasYZ2;
                    YZ2Path = yz2->YZ2Path;
                    DatAmount = yz2->DatAmount;
                    DatFiles = yz2->DatFiles;
                    ExtraRel = yz2->ExtraRel;
                    IsE3Version = yz2->IsE3Version;

                    if (!yz2->hasYZ2)
                    {
                        Dat^ a = gcnew Dat(idxj, readStream, startOffset, length, directory, baseName, fileFormat);
                        DatAmount = a->DatAmount;
                        DatFiles = a->DatFiles;
                        ExtraRel = a->ExtraRel;
                        IsE3Version = a->IsE3Version;
                    }

                    readedDat = true;
                }
                else if (type != 0x0 && type != 0xFFFFFFFF && !readedSnd)
                {
                    // SND

                    SoundFlag = (int)type;
                    idxj->WriteLine("UDAS_SOUNDFLAG:" + ((int)type).ToString());

                    int startOffset = (int)UdasList[i].offset;
                    int length = (int)(br->Length - startOffset);

                    //middle
                    if (i >= 1)
                    {
                        int M_Length = (int)UdasList[i - 1].length;
                        int M_startOffset = (int)UdasList[i - 1].offset;
                        int subOffset = M_startOffset + M_Length;
                        int subLength = startOffset - subOffset;

                        if (subLength > 0)
                        {
                            array<Byte>^ udasMiddle = gcnew array<Byte>(subLength);

                            br->Position = subOffset;
                            br->Read(udasMiddle, 0, subLength);

                            String^ fileFullName = Path::Combine(baseName, baseName + "_MIDDLE.HEX");
                            idxj->WriteLine("!UDAS_MIDDLE:" + fileFullName);

                            try
                            {
                                File::WriteAllBytes(Path::Combine(directory, fileFullName), udasMiddle);
                            }
                            catch (Exception^ ex)
                            {
                                Console::WriteLine(fileFullName + ": " + ex);
                            }
                        }
                    }

                    //end
                    {
                        array<Byte>^ udasEnd = gcnew array<Byte>(length);

                        br->Position = startOffset;
                        br->Read(udasEnd, 0, length);

                        String^ fileFullNameEnd;

                        if (length > 0 && type == 0x04) // 4
                        {
                            fileFullNameEnd = Path::Combine(baseName, baseName + "_END.SND");
                            idxj->WriteLine("UDAS_END:" + fileFullNameEnd);
                        }
                        else // type == 0xFF_FF_FF_FE // -2
                        {
                            fileFullNameEnd = Path::Combine(baseName, baseName + "_END.EMPTY");
                            idxj->WriteLine("UDAS_END:" + fileFullNameEnd);
                        }

                        SndPath = fileFullNameEnd;

                        try
                        {
                            File::WriteAllBytes(Path::Combine(directory, fileFullNameEnd), udasEnd);
                        }
                        catch (Exception^ ex)
                        {
                            Console::WriteLine(fileFullNameEnd + ": " + ex);
                        }
                    }

                    readedSnd = true;
                }
                else if (type != 0xFFFFFFFF)
                {
                    idxj->WriteLine("# ERROR_FLAG" + i.ToString("D1") + ":" + ((int)type).ToString());

                    int startOffset = (int)UdasList[i].offset;
                    int length = (int)(br->Length - startOffset);

                    if (length > 0)
                    {
                        array<Byte>^ udasError = gcnew array<Byte>(length);

                        br->Position = startOffset;
                        br->Read(udasError, 0, length);

                        String^ FileFullName = Path::Combine(baseName, baseName + String::Format("_ERROR{0:D1}.HEX", i));
                        idxj->WriteLine("# ERROR_FILE" + i.ToString("D1") + ":" + FileFullName);

                        try
                        {
                            File::WriteAllBytes(Path::Combine(directory, FileFullName), udasError);
                        }
                        catch (Exception^ ex)
                        {
                            Console::WriteLine(FileFullName + ": " + ex);
                        }
                    }
                }
            }
        }
    };
}

#endif