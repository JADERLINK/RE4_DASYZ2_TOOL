#pragma once
#ifndef __Endianness
#define __Endianness

using namespace System;

namespace SimpleEndianBinaryIO {
    public enum class Endianness : Byte
    {
        LittleEndian = 0,
        BigEndian = 1
    };
}

#endif