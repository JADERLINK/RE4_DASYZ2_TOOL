using namespace System;
using namespace System::IO;

#include <iostream>;
#include "EXTRACT/FileFormat.h";
#include "EXTRACT/Extract.cpp";
#include "REPACK/RepackIdxBig.cpp"

static void Continue(String^ file);

int main(array<System::String^>^ args)
{
    System::Globalization::CultureInfo::CurrentCulture = System::Globalization::CultureInfo::InvariantCulture;
    Console::OutputEncoding = System::Text::Encoding::UTF8;

    Console::WriteLine("# RE4_DASYZ2_TOOL");
    Console::WriteLine("# Tool by: JADERLINK");
    Console::WriteLine("# YZ2 compression by: Yamazaki Satoshi");
    Console::WriteLine("# Thanks for OAleex and Krisp");
    Console::WriteLine("# Version 2025-07-17");
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
    Console::WriteLine();
    Console::WriteLine("File: " + fileInfo->Name);
    String^ Extension = fileInfo->Extension->ToUpperInvariant();

    if (Extension == ".DAT" || Extension == ".MAP" || Extension == ".UDAS" || Extension == ".DAS" || Extension == ".DRS" || Extension == ".DECMP")
    {
        Console::WriteLine("Extract Mode!");
        Console::WriteLine("Wait for processing to finish.");
        System::Diagnostics::Stopwatch^ sw = System::Diagnostics::Stopwatch::StartNew();

        EXTRACT::FileFormat fileFormat = EXTRACT::FileFormat::Null;
        if (Extension == ".DAT") fileFormat = EXTRACT::FileFormat::DAT;
        else if (Extension == ".MAP") fileFormat = EXTRACT::FileFormat::MAP;
        else if (Extension == ".UDAS") fileFormat = EXTRACT::FileFormat::UDAS;
        else if (Extension == ".DAS") fileFormat = EXTRACT::FileFormat::DAS;
        else if (Extension == ".DRS") fileFormat = EXTRACT::FileFormat::DRS;
        else if (Extension == ".DECMP") fileFormat = EXTRACT::FileFormat::DECMP;

        if (fileFormat != EXTRACT::FileFormat::Null)
        {
           gcnew EXTRACT::Extract(fileInfo, fileFormat);
        }

        sw->Stop();
        Console::WriteLine("Taken time in Milliseconds: " + sw->ElapsedMilliseconds);
    }
    else if (Extension == ".IDXBIG")
    {
        Console::WriteLine("Repack Mode!");
        Console::WriteLine("Wait for processing to finish.");
        System::Diagnostics::Stopwatch^ sw = System::Diagnostics::Stopwatch::StartNew();

        gcnew REPACK::RepackIdxBig(fileInfo);

        sw->Stop();
        Console::WriteLine("Taken time in Milliseconds: " + sw->ElapsedMilliseconds);
    }
    else
    {
        Console::WriteLine("The extension is not valid: " + Extension);
    }
}