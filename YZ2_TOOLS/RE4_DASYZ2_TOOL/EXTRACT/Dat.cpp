#pragma once
#ifndef __EXTRACT_DAT
#define __EXTRACT_DAT

#include "EndianBinaryReader.cpp";
#include "EndianBinaryWriter.cpp";
#include "EndianBitConverter.cpp";
#include "Endianness.h";
#include "FileFormat.h"

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
        String^ ExtraRel = nullptr;
        bool IsE3Version = false;

        Dat(StreamWriter^ idxj, Stream^ readStream, UInt32 offsetStart, UInt32 fullLength, String^ directory, String^ baseName, FileFormat fileFormat)
        {
            UInt32 tableOffset = 0x10; // na versão final do jogo, a tabela começa no offset 0x10, na E3 em 0x4;

            UInt32 endDatOffset = fullLength; // define o final real dos arquivos com formatos, no DRS tem mais um arquivo no final (REL);

            bool hasExtraDRS = false; // booleano que indica se existe o arquivo adicional do DRS;

            UInt32 extraDrsOffset = 0; // o offset caso tenha o arquivo REL do DRS;

            EndianBinaryReader^ br = gcnew EndianBinaryReader(readStream, Endianness::BigEndian);
            br->BaseStream->Position = offsetStart;

            //o primeiro uint, em geral é um amount, exceto na versão GC E3 nos arquivos DRS no qual pode ser um offset;
            UInt32 amount = br->ReadUInt32();

            if (amount >= 0x010000 && fileFormat != FileFormat::DRS) // só é inválido se o valor for maior de 0x10000 e não for DRS;
            {
                Console::WriteLine("Invalid file!");
                return;
            }

            //os 3 próximos uint são usados para verificar o tipo de conteúdo;
            UInt32 u2 = br->ReadUInt32();
            UInt32 u3 = br->ReadUInt32();
            UInt32 u4 = br->ReadUInt32();

            if (fileFormat != FileFormat::DRS) // caso não for um DRS;
            {
                // verifica se é da versão final ou do E3;
                if (u2 != 0 || u3 != 0 || u4 != 0)
                {
                    tableOffset = 0x04; // versão E3
                    IsE3Version = true;
                    idxj->WriteLine("IS_E3_VERSION:true");
                }
            }
            else // é um DRS
            {
                if (u3 == 0 && u4 == 0) // se for 0, esse é do tipo final;
                {
                    if (u2 != 0) // u2 é a posição do extraDrsOffset;
                    {
                        extraDrsOffset = u2;
                        hasExtraDRS = true;
                        if (extraDrsOffset < endDatOffset)
                        {
                            endDatOffset = extraDrsOffset;
                        }
                    }

                }
                else // senão tipo E3
                {
                    tableOffset = 0x04; // versão E3
                    IsE3Version = true;
                    idxj->WriteLine("IS_E3_VERSION:true");

                    //no tipo E3 o primeiro campo pode ser uma quantidade ou o extraDrsOffset;
                    if (amount >= u2) // se for maior que isso, o campo é um offset
                    {
                        UInt32 realAmount = 0;
                        UInt32 lastValidOffset = u2;

                        br->Position = offsetStart + 0x4;

                        while (true) // verificação da quantidade real
                        {
                            UInt32 val = br->ReadUInt32();

                            if (val > fullLength && val > 0x30000000) // 30 = 0 or 41 = A
                            {
                                break;
                            }

                            if (br->Position >= offsetStart + u2)
                            {
                                Console::WriteLine("Invalid file! E3 DRS LOOP");
                                return;
                            }

                            if (val > lastValidOffset)
                            {
                                lastValidOffset = val;
                            }

                            realAmount++;
                        }

                        if (amount > lastValidOffset && amount < endDatOffset)
                        {
                            hasExtraDRS = true;
                            extraDrsOffset = amount;
                            endDatOffset = amount;
                        }

                        amount = realAmount;
                    }

                }

            }

            //-----------------------------

            if (!Directory::Exists(Path::Combine(directory, baseName)) && fileFormat != FileFormat::DECMP)
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

            idxj->WriteLine("DAT_AMOUNT:" + amount);
            DatAmount = static_cast<int>(amount);

            //-----------------------------

            int blocklength = static_cast<int>(amount * 4u);

            array<Byte>^ offsetblock = gcnew array<Byte>(blocklength);
            array<Byte>^ nameblock = gcnew array<Byte>(blocklength);

            br->BaseStream->Position = offsetStart + tableOffset;

            br->Read(offsetblock, 0, blocklength);
            br->Read(nameblock, 0, blocklength);

            array<KeyValuePair<int, String^>^>^ fileList = gcnew array<KeyValuePair<int, String^>^>(amount);

            int Temp = 0;
            for (int i = 0; i < amount; i++)
            {
                int offset = EndianBitConverter::ToInt32(offsetblock, Temp, Endianness::BigEndian);
                String^ format = Encoding::ASCII->GetString(nameblock, Temp, 4);
                format = ValidateFormat(format)->ToUpperInvariant();

                String^ fileFullName = Path::Combine(baseName, baseName + "_" + i.ToString("D3"));
                if (fileFormat == FileFormat::DECMP)
                {
                    fileFullName = baseName + "_" + i.ToString("D3");
                }
                if (format->Length > 0)
                {
                    fileFullName += "." + format;
                }

                fileList[i] = gcnew KeyValuePair<int, String^>(offset, fileFullName);

                Temp += 4;
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
                    subFileLength = static_cast<int>(endDatOffset - fileList[i]->Key);
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
                idxj->WriteLine(Line);
            }

            //---
            if (hasExtraDRS)
            {
                String^ fileFullName = Path::Combine(baseName, baseName + "_EXTRA.REL");

                int subFileLength = static_cast<int>(fullLength - extraDrsOffset);
                br->BaseStream->Position = offsetStart + extraDrsOffset;

                array<Byte>^ endfile = gcnew array<Byte>(subFileLength);
                br->Read(endfile, 0, subFileLength);
                if (subFileLength > 0)
                {
                    try
                    {
                        File::WriteAllBytes(Path::Combine(directory, fileFullName), endfile);
                    }
                    catch (Exception^ ex)
                    {
                        Console::WriteLine(fileFullName + ": " + ex);
                    }

                }

                String^ Line = "EXTRA_REL:" + fileFullName;
                idxj->WriteLine(Line);

                ExtraRel = fileFullName;
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