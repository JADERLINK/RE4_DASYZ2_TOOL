using namespace System;
using namespace System::IO;

#include <iostream>;
#include "YZ2.h";

static void Continue(String^ file);
static void ContinueDec(String^ filePath);
static void ContinueEnc(String^ filePath);

int main(array<System::String^> ^args)
{
    System::Globalization::CultureInfo::CurrentCulture = System::Globalization::CultureInfo::InvariantCulture;
    Console::OutputEncoding = System::Text::Encoding::UTF8;

    Console::WriteLine("# RE4_YZ2_COMPRESSION_TOOL");
    Console::WriteLine("# Tool by: JADERLINK");
    Console::WriteLine("# YZ2 compression by: Yamazaki Satoshi");
    Console::WriteLine("# Thanks for OAleex and Krisp");
    Console::WriteLine("# Version 2025-05-01");
    Console::WriteLine("");

    bool usingBatFile = false;
    int start = 0;
    if (args != nullptr && args->Length > 0 && args[0]->ToLowerInvariant() == "-bat")
    {
        usingBatFile = true;
        start = 1;
    }

    if (args != nullptr && args->Length > 0)
    {
        for (int i = start; i < args->Length; i++)
        {
            if (File::Exists(args[i]))
            {
                try
                {
                    Continue(args[i]);
                }
                catch (Exception^ ex)
                {
                    Console::WriteLine("Error: " + args[i]);
                    Console::WriteLine(ex);
                }
                catch (const std::exception& ex) {
                    Console::WriteLine("Error: " + args[i]);
                    std::cout << "Caught native C++ exception: " << ex.what() << std::endl;
                }
                catch (...) {
                    Console::WriteLine("Error: " + args[i]);
                    std::cout << "Unknown (unhandled) exception!" << std::endl;
                }
            }
        }
    }

    if (args == nullptr || args->Length == 0)
    {
        Console::WriteLine("How to use: drag the file to the executable.");
        Console::WriteLine("Press any key to close the console.");
        Console::ReadKey();
    }
    else
    {
        Console::WriteLine("Finished!!!");
        if (!usingBatFile)
        {
            Console::WriteLine("Press any key to close the console.");
            Console::ReadKey();
        }
    }

    return 0;
}


static void Continue(String^ file)
{
    FileInfo^ fileInfo = gcnew FileInfo(file);
    Console::WriteLine("File: " + fileInfo->Name);
    String^ Extension = Path::GetExtension(fileInfo->Name)->ToUpperInvariant();

    if (Extension == ".YZ2")
    {
        Console::WriteLine("Decode Mode!");
        Console::WriteLine("Wait for processing to finish.");
        System::Diagnostics::Stopwatch^ sw = System::Diagnostics::Stopwatch::StartNew();

        ContinueDec(fileInfo->FullName);

        sw->Stop();
        Console::WriteLine("Taken time in Milliseconds: " + sw->ElapsedMilliseconds);
    }
    else
    {
        Console::WriteLine("Encode Mode!");
        Console::WriteLine("Wait for processing to finish.");
        System::Diagnostics::Stopwatch^ sw = System::Diagnostics::Stopwatch::StartNew();

        ContinueEnc(fileInfo->FullName);

        sw->Stop();
        Console::WriteLine("Taken time in Milliseconds: " + sw->ElapsedMilliseconds);
    }
}

static void ContinueDec(String^ filePath)
{
    FileInfo^ fileInfo = gcnew FileInfo(filePath);
    FileStream^ stream = fileInfo->OpenRead();

    array<Byte>^ in_header = gcnew array<Byte>(32);
    stream->Read(in_header, 0, 32);

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
        array<unsigned char>^ in_content = gcnew array<unsigned char>(stream->Length);

        stream->Position = 0;
        stream->Read(in_content, 0, fileInfo->Length);
        stream->Close();

        array<unsigned char>^ out_content;

        YZ2::YZ2Actions::YZ2Decode(in_content, out_content);

        String^ outFilePath = Path::Combine(Path::GetDirectoryName(filePath), Path::GetFileNameWithoutExtension(filePath));
        String^ subExtension = Path::GetExtension(Path::GetFileNameWithoutExtension(filePath));
        if (subExtension == nullptr || subExtension == "")
        {
            outFilePath += ".decmp";
        }

        File::WriteAllBytes(outFilePath, out_content);
    }
    else
    {
        stream->Close();
        Console::WriteLine("The yz2 file is invalid!");
    }
}


static void ContinueEnc(String^ filePath)
{
    FileInfo^ fileInfo = gcnew FileInfo(filePath);
    FileStream^ stream = fileInfo->OpenRead();

    array<unsigned char>^ in_content = gcnew array<unsigned char>(fileInfo->Length);

    stream->Read(in_content, 0, fileInfo->Length);
    stream->Close();

    array<unsigned char>^ out_content;

    YZ2::YZ2Actions::YZ2Encode(in_content, out_content);

    String^ finalFile = filePath;
    if (Path::GetExtension(finalFile)->ToLowerInvariant() == ".decmp")
    {
        finalFile = Path::ChangeExtension(finalFile, ".yz2");
    }
    else
    {
        finalFile += ".yz2";
    }

    File::WriteAllBytes(finalFile, out_content);
}
