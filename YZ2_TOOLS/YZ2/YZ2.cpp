#pragma once

#include "OutEvM.h"
#include "InEvM.h"

#include "yz2Code/yz2Decode.cxx"
#include "yz2Code/yz2Encode.cxx"

using namespace System;
using namespace System::IO;
using namespace System::Globalization;
using namespace System::Text;
using namespace System::Collections::Generic;

#include <iostream>
#include <vcclr.h>  // Required for pin_ptr

namespace YZ2 {

	public ref class YZ2Actions abstract sealed {
	public:

		static void YZ2Encode(array<unsigned char>^ in_content, array<unsigned char>^% final_out_content)
		{
            pin_ptr<unsigned char> in_ptr = &in_content[0];

            array<unsigned char>^ out_content = gcnew array<unsigned char>(in_content->Length + 2048);
            pin_ptr<unsigned char> out_ptr = &out_content[0];

            OutEvM out_ev(out_ptr, out_content->Length);

            // has to have a separate scope
            {
                yz2Encode< OutEvM > yz2_enc(out_ev, false);

                yz2_enc.EncodeArea(
                    (unsigned char*)in_ptr  // in : The start position of the data to be processed (search buffer START)
                );

                //  YZ encoding process
                yz2_enc.Encode(             // ---: -----------------
                    in_content->Length,     // in : Length to compress
                    in_content->Length      // in : End position of the data to be processed (reference buffer END)
                );
            }

            MemoryStream^ ms = gcnew MemoryStream();

            String^ headerText = out_ev.GetPosition().ToString("x", CultureInfo::InvariantCulture) + "\t" + in_content->Length.ToString("x", CultureInfo::InvariantCulture) + "\n";
            array<unsigned char>^ header = gcnew array<unsigned char>(32);

            ASCIIEncoding^ encoding = gcnew ASCIIEncoding();
            encoding->GetBytes(headerText, 0, headerText->Length, header, 0);

            ms->Write(header, 0, 32);
            ms->Write(out_content, 0, out_ev.GetPosition());
            ms->Close();
            final_out_content = ms->ToArray();
		}

        static void YZ2Decode(array<unsigned char>^ compressed_content, array<unsigned char>^% final_out_content)
        {
            MemoryStream^ stream = gcnew MemoryStream(compressed_content);
            
            int compressedSize = compressed_content->Length - 32;

            array<unsigned char>^ in_header = gcnew array<unsigned char>(32);
            array<unsigned char>^ in_content = gcnew array<unsigned char>(compressedSize);

            stream->Read(in_header, 0, 32);
            stream->Read(in_content, 0, compressedSize);
            stream->Close();

            pin_ptr<unsigned char> in_ptr = &in_content[0];

            ASCIIEncoding^ encoding = gcnew ASCIIEncoding();
            String^ textHeader = encoding->GetString(in_header, 0, 32);
            array<String^>^ split = textHeader->Split(gcnew array<wchar_t>{'\t', '\n'});
            int outLength = Int32::Parse(split[1], System::Globalization::NumberStyles::HexNumber, System::Globalization::CultureInfo::InvariantCulture);

            array<unsigned char>^ out_content = gcnew array<unsigned char>(outLength);
            pin_ptr<unsigned char> out_ptr = &out_content[0];

            // has to have a separate scope
            {
                InEvM in_ev(in_ptr, compressedSize);
                yz2Decode< InEvM > yz2_dec(in_ev, false);

                yz2_dec.DecodeArea(            // ---: --------------------------------------------
                    (unsigned char*)out_ptr,   // in : Starting position of decompression target data (START search buffer)
                    outLength                  // in : End position of decompression target data (END reference buffer)
                );

                //  YZ Decoding Process
                int rtn_code = yz2_dec.Decode(  // ---: ----------------
                    outLength                   // in : Length to be stretched
                );

            }

            final_out_content = out_content;
        }
	};

}