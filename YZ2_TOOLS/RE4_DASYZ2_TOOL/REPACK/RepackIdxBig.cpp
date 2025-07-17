#pragma once
#ifndef __REPACK_RepackIdxBig
#define __REPACK_RepackIdxBig

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";
#include "Dat.cpp";
#include "Udas.cpp";
#include "DasYZ2.cpp";
#include "NewDasYZ2.cpp";

using namespace System;
using namespace System::IO;
using namespace System::Collections::Generic;
using namespace System::Globalization;

namespace REPACK
{
    ref class RepackIdxBig
    {
    public:
        RepackIdxBig(FileInfo^ info)
        {
            StreamReader^ idxj = nullptr;

            try
            {
                idxj = info->OpenText();
            }
            catch (Exception^ ex)
            {
                Console::WriteLine("Error: " + ex);
                return;
            }

            // continua só se idxj != nullptr

            String^ TOOL_VERSION = nullptr;
            String^ FILE_FORMAT = nullptr;
            UInt32 DAT_AMOUNT = 0;
            Dictionary<String^, String^>^ DatFiles = gcnew Dictionary<String^, String^>();
            String^ UDAS_TOP = nullptr;
            Int32 UDAS_SOUNDFLAG = -1;
            String^ UDAS_END = nullptr;
            String^ EXTRA_REL = nullptr;
            String^ UDAS_MIDDLE = nullptr;
            String^ YZ2_PATH = nullptr;
            bool HAS_YZ2 = false;
            bool IS_DAT_COMPRESSED = false;
            bool IS_E3_VERSION = false;

            while (!idxj->EndOfStream)
            {
                String^ line = idxj->ReadLine();
                if (line != nullptr)
                {
                    line = line->Trim();
                }

                if (!(String::IsNullOrEmpty(line)
                    || line->StartsWith("#")
                    || line->StartsWith("\\")
                    || line->StartsWith("/")
                    || line->StartsWith(":")
                    || line->StartsWith("!")
                    || line->StartsWith("@")
                    ))
                {
                    array<String^>^ split = line->Split(gcnew array<wchar_t>{ ':' });
                    if (split->Length >= 2)
                    {
                        String^ key = split[0]->ToUpperInvariant()->Trim();
                        String^ value = split[1]->Trim()->Replace('\\', Path::DirectorySeparatorChar)->Replace('/', Path::DirectorySeparatorChar);

                        if (key->Contains("FILE_FORMAT"))
                        {
                            FILE_FORMAT = value;
                        }
                        else if (key->Contains("TOOL_VERSION"))
                        {
                            TOOL_VERSION = value;
                        }
                        else if (key->Contains("UDAS_TOP"))
                        {
                            UDAS_TOP = value;
                        }
                        else if (key->Contains("UDAS_END"))
                        {
                            UDAS_END = value;
                        }
                        else if (key->Contains("EXTRA_REL"))
                        {
                            EXTRA_REL = value;
                        }
                        else if (key->Contains("UDAS_MIDDLE"))
                        {
                            UDAS_MIDDLE = value;
                        }
                        else if (key->Contains("YZ2_PATH"))
                        {
                            YZ2_PATH = value;
                        }
                        else if (key->Contains("UDAS_SOUNDFLAG"))
                        {
                            Int32::TryParse(value, NumberStyles::Integer, CultureInfo::InvariantCulture, UDAS_SOUNDFLAG);
                        }
                        else if (key->Contains("DAT_AMOUNT"))
                        {
                            UInt32::TryParse(value, NumberStyles::Integer, CultureInfo::InvariantCulture, DAT_AMOUNT);
                        }
                        else if (key->Contains("HAS_YZ2"))
                        {
                            Boolean::TryParse(value, HAS_YZ2);
                        }
                        else if (key->Contains("IS_DAT_COMPRESSED"))
                        {
                            Boolean::TryParse(value, IS_DAT_COMPRESSED);
                        }
                        else if (key->Contains("IS_E3_VERSION"))
                        {
                            Boolean::TryParse(value, IS_E3_VERSION);
                        }
                        else if (key->StartsWith("DAT_"))
                        {
                            if (!DatFiles->ContainsKey(key))
                            {
                                DatFiles->Add(key, value);
                            }
                        }
                    }
                }
            }

            idxj->Close();

            if (FILE_FORMAT == nullptr
                || !(FILE_FORMAT == "UDAS" || FILE_FORMAT == "DAT" || FILE_FORMAT == "MAP" || FILE_FORMAT == "DAS" || FILE_FORMAT == "DRS" || FILE_FORMAT == "DECMP"))
            {
                Console::WriteLine("Invalid FILE_FORMAT!");
                return;
            }

            if (DAT_AMOUNT == 0 && HAS_YZ2 == false)
            {
                Console::WriteLine("DAT_AMOUNT cannot be 0!");
                return;
            }

            if (IS_DAT_COMPRESSED && HAS_YZ2)
            {
                Console::WriteLine("IS_DAT_COMPRESSED and HAS_YZ2 cannot be true at the same time!");
                return;
            }


            if (TOOL_VERSION != nullptr)
            {
                Console::WriteLine("TOOL_VERSION: " + TOOL_VERSION);
            }

            Console::WriteLine("FILE_FORMAT: " + FILE_FORMAT);

            if (DAT_AMOUNT != 0)
            {
                Console::WriteLine("DAT_AMOUNT: " + DAT_AMOUNT);
            }

            FileStream^ stream = nullptr;

            try
            {
                String^ endFileName = Path::ChangeExtension(info->FullName, FILE_FORMAT->ToLowerInvariant());
                FileInfo^ endFileInfo = gcnew FileInfo(endFileName);
                stream = endFileInfo->Create();
            }
            catch (Exception^ ex)
            {
                Console::WriteLine("Error: " + ex);
                return;
            }

            // continua só se stream != nullptr

            array<DatInfo^>^ datGroup = gcnew array<DatInfo^>(DAT_AMOUNT);
            DatInfo^ ExtraRel = nullptr;

            //calc
            UInt32 headerLength = IS_E3_VERSION ? 4u : 16u; // no E3 o header é menor
            UInt32 fullDatHeaderLength = headerLength + (4 * DAT_AMOUNT * 2);
            fullDatHeaderLength = (((fullDatHeaderLength + 31) / 32) * 32);
            UInt32 datFileBytesLength = fullDatHeaderLength;

            // get files
            for (int i = 0; i < DAT_AMOUNT; i++)
            {
                DatInfo^ dat = gcnew DatInfo();
                String^ key = "DAT_" + i.ToString("D3");
                if (DatFiles->ContainsKey(key))
                {
                    dat->Path = DatFiles[key];
                }
                else
                {
                    dat->Path = "null";
                }
                datGroup[i] = dat;
            }

            UInt32 tempOffset = fullDatHeaderLength;
            for (int i = 0; i < DAT_AMOUNT; i++)
            {
                FileInfo^ a = gcnew FileInfo(Path::Combine(info->Directory->FullName, datGroup[i]->Path));
                datGroup[i]->fileInfo = a;
                datGroup[i]->Extension = a->Extension->ToUpperInvariant()->Replace(".", "")->PadRight(4, (char)0x0)->Substring(0, 4);
                datGroup[i]->Offset = tempOffset;

                if (a->Exists)
                {
                    int aLength = (int)(((a->Length + 31) / 32) * 32);

                    datGroup[i]->FileExits = true;
                    datFileBytesLength += (UInt32)aLength;
                    datGroup[i]->Length = aLength;
                    tempOffset += (UInt32)aLength;

                    Console::WriteLine("DAT_" + i.ToString("D3") + ": " + datGroup[i]->Path);
                }
                else
                {
                    Console::WriteLine("DAT_" + i.ToString("D3") + ": " + datGroup[i]->Path + "   (File does not exist!)");
                }
            }

            //get extraRel
            if (EXTRA_REL != nullptr)
            {
                DatInfo^ extraRel = gcnew DatInfo();
                FileInfo^ a = gcnew FileInfo(Path::Combine(info->Directory->FullName, EXTRA_REL));
                extraRel->fileInfo = a;
                extraRel->Extension = a->Extension->ToUpperInvariant()->Replace(".", "")->PadRight(4, (char)0x0)->Substring(0, 4);
                extraRel->Offset = tempOffset;

                if (a->Exists)
                {
                    int aLength = (int)(((a->Length + 31) / 32) * 32);

                    extraRel->FileExits = true;
                    datFileBytesLength += (UInt32)aLength;
                    extraRel->Length = aLength;
                    tempOffset += (UInt32)aLength;

                    Console::WriteLine("EXTRA_REL: " + EXTRA_REL);
                    ExtraRel = extraRel;
                }
                else
                {
                    Console::WriteLine("EXTRA_REL: " + EXTRA_REL + "   (File does not exist!)");
                }
            }

            if (FILE_FORMAT == "DAT" || FILE_FORMAT == "MAP" || FILE_FORMAT == "DECMP")
            {
                Dat^ _ = gcnew Dat(stream, datGroup, ExtraRel, 0, IS_E3_VERSION);
            }
            else if (FILE_FORMAT == "UDAS" || FILE_FORMAT == "DAS" || FILE_FORMAT == "DRS")
            {
                bool isDRS = FILE_FORMAT == "DRS";

                UdasInfo^ udasGroup = gcnew UdasInfo();
                udasGroup->DatFileAlignedBytesLength = datFileBytesLength;
                udasGroup->DatFileRealBytesLength = datFileBytesLength;
                udasGroup->SoundFlag = UDAS_SOUNDFLAG;

                Console::WriteLine("UDAS_SOUNDFLAG: " + UDAS_SOUNDFLAG.ToString("d"));

                if (UDAS_END != nullptr)
                {
                    udasGroup->End->Path = UDAS_END;
                    FileInfo^ a = gcnew FileInfo(Path::Combine(info->Directory->FullName, udasGroup->End->Path));
                    udasGroup->End->fileInfo = a;

                    if (a->Exists)
                    {
                        int aLength = (int)(((a->Length + 31) / 32) * 32);

                        udasGroup->End->FileExits = true;
                        udasGroup->End->Length = aLength;

                        Console::WriteLine("UDAS_END: " + udasGroup->End->Path);
                    }
                    else
                    {
                        Console::WriteLine("UDAS_END: " + udasGroup->End->Path + "   (File does not exist!)");
                    }
                }

                if (UDAS_MIDDLE != nullptr)
                {
                    udasGroup->Middle->Path = UDAS_MIDDLE;
                    FileInfo^ a = gcnew FileInfo(Path::Combine(info->Directory->FullName, udasGroup->Middle->Path));
                    udasGroup->Middle->fileInfo = a;

                    if (a->Exists)
                    {
                        int aLength = (int)(((a->Length + 31) / 32) * 32);

                        udasGroup->Middle->FileExits = true;
                        udasGroup->Middle->Length = aLength;

                        Console::WriteLine("UDAS_MIDDLE: " + udasGroup->Middle->Path);
                    }
                    else
                    {
                        Console::WriteLine("UDAS_MIDDLE: " + udasGroup->Middle->Path + "   (File does not exist!)");
                    }
                }

                if (UDAS_TOP != nullptr)
                {
                    udasGroup->Top->Path = UDAS_TOP;
                    FileInfo^ a = gcnew FileInfo(Path::Combine(info->Directory->FullName, udasGroup->Top->Path));
                    udasGroup->Top->fileInfo = a;

                    if (a->Exists)
                    {
                        int aLength = (int)(((a->Length + 31) / 32) * 32);

                        udasGroup->Top->FileExits = true;
                        udasGroup->Top->Length = aLength;

                        Console::WriteLine("UDAS_TOP: " + udasGroup->Top->Path);
                    }
                    else
                    {
                        Console::WriteLine("UDAS_TOP: " + udasGroup->Top->Path + "   (File does not exist!)");
                    }
                }

                if (IS_DAT_COMPRESSED)
                {
                    NewDasYZ2^ _ = gcnew NewDasYZ2(stream, datGroup, udasGroup, ExtraRel, IS_E3_VERSION);
                }
                else if (HAS_YZ2)
                {
                    DatInfo^ yz2 = gcnew DatInfo();

                    yz2->Path = YZ2_PATH;
                    FileInfo^ a = gcnew FileInfo(Path::Combine(info->Directory->FullName, yz2->Path));
                    yz2->fileInfo = a;

                    if (a->Exists)
                    {
                        int aLength = (int)(((a->Length + 31) / 32) * 32);

                        yz2->FileExits = true;
                        yz2->Length = aLength;
                        udasGroup->DatFileAlignedBytesLength = (UInt32)aLength;
                        udasGroup->DatFileRealBytesLength = (UInt32)a->Length;

                        Console::WriteLine("YZ2_PATH: " + yz2->Path);
                    }
                    else
                    {
                        Console::WriteLine("YZ2_PATH: " + yz2->Path + "   (File does not exist!)");
                    }

                    DasYZ2^ _ = gcnew DasYZ2(stream, yz2, udasGroup);
                }
                else
                {
                    Udas^ _ = gcnew Udas(stream, datGroup, udasGroup, isDRS, ExtraRel, IS_E3_VERSION);
                }
            }

            stream->Close();

        }
    };
}

#endif