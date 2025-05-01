#pragma once

using namespace System;

namespace YZ2 {

	public ref class YZ2Actions abstract sealed {
	public:

		static void YZ2Encode(array<unsigned char>^ in_content, array<unsigned char>^% final_out_content);

		static void YZ2Decode(array<unsigned char>^ in_content, array<unsigned char>^% final_out_content);

	};

}