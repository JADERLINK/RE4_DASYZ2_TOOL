#pragma once
#ifndef __EndianBitConverter
#define __EndianBitConverter

#include "Endianness.h";

using namespace System;

namespace SimpleEndianBinaryIO {

    public ref class EndianBitConverter abstract sealed {
    public:

        static property Endianness NativeEndianness
        {
            Endianness get()
            {
                return BitConverter::IsLittleEndian ? Endianness::LittleEndian : Endianness::BigEndian;
            }
        }

        static array<Byte>^ GetBytes(Int16 value, Endianness endianness)
        {
            return endianness == NativeEndianness ? BitConverter::GetBytes(value) : BitConverter::GetBytes(Reverse(value));
        }

        static array<Byte>^ GetBytes(UInt16 value, Endianness endianness)
        {
            return endianness == NativeEndianness ? BitConverter::GetBytes(value) : BitConverter::GetBytes(Reverse(value));
        }

        static array<Byte>^ GetBytes(Int32 value, Endianness endianness)
        {
            return endianness == NativeEndianness ? BitConverter::GetBytes(value) : BitConverter::GetBytes(Reverse(value));
        }

        static array<Byte>^ GetBytes(UInt32 value, Endianness endianness)
        {
            return endianness == NativeEndianness ? BitConverter::GetBytes(value) : BitConverter::GetBytes(Reverse(value));
        }

        static array<Byte>^ GetBytes(Int64 value, Endianness endianness)
        {
            return endianness == NativeEndianness ? BitConverter::GetBytes(value) : BitConverter::GetBytes(Reverse(value));
        }

        static array<Byte>^ GetBytes(UInt64 value, Endianness endianness)
        {
            return endianness == NativeEndianness ? BitConverter::GetBytes(value) : BitConverter::GetBytes(Reverse(value));
        }

        static array<Byte>^ GetBytes(Single value, Endianness endianness)
        {
            if (endianness == NativeEndianness) 
            {
                return BitConverter::GetBytes(value);
            }
            else
            {
                UInt32 raw = BitConverter::ToUInt32(BitConverter::GetBytes(value), 0);
                return BitConverter::GetBytes(Reverse(raw));
            }
        }

        static array<Byte>^ GetBytes(Double value, Endianness endianness)
        {
            if (endianness == NativeEndianness)
            {
                return BitConverter::GetBytes(value);
            }
            else
            {
                UInt64 raw = BitConverter::ToUInt64(BitConverter::GetBytes(value), 0);
                return BitConverter::GetBytes(Reverse(raw));
            }
        }

        static Double ToDouble(array<Byte>^ value, int startIndex, Endianness endianness)
        {
            return endianness == NativeEndianness
                ? BitConverter::ToDouble(value, startIndex)
                : BitConverter::ToDouble(BitConverter::GetBytes(Reverse(BitConverter::ToUInt64(value, startIndex))), 0);
        }

        static Single ToSingle(array<Byte>^ value, int startIndex, Endianness endianness)
        {
            return endianness == NativeEndianness
                ? BitConverter::ToSingle(value, startIndex)
                : BitConverter::ToSingle(BitConverter::GetBytes(Reverse(BitConverter::ToUInt32(value, startIndex))), 0);
        }

        static Int16 ToInt16(array<Byte>^ value, int startIndex, Endianness endianness)
        {
            Int16 v = BitConverter::ToInt16(value, startIndex);
            return endianness == NativeEndianness ? v : Reverse(v);
        }

        static UInt16 ToUInt16(array<Byte>^ value, int startIndex, Endianness endianness)
        {
            UInt16 v = BitConverter::ToUInt16(value, startIndex);
            return endianness == NativeEndianness ? v : Reverse(v);
        }

        static Int32 ToInt32(array<Byte>^ value, int startIndex, Endianness endianness)
        {
            Int32 v = BitConverter::ToInt32(value, startIndex);
            return endianness == NativeEndianness ? v : Reverse(v);
        }

        static UInt32 ToUInt32(array<Byte>^ value, int startIndex, Endianness endianness)
        {
            UInt32 v = BitConverter::ToUInt32(value, startIndex);
            return endianness == NativeEndianness ? v : Reverse(v);
        }

        static Int64 ToInt64(array<Byte>^ value, int startIndex, Endianness endianness)
        {
            Int64 v = BitConverter::ToInt64(value, startIndex);
            return endianness == NativeEndianness ? v : Reverse(v);
        }

        static UInt64 ToUInt64(array<Byte>^ value, int startIndex, Endianness endianness)
        {
            UInt64 v = BitConverter::ToUInt64(value, startIndex);
            return endianness == NativeEndianness ? v : Reverse(v);
        }

        static Int16 Reverse(Int16 value)
        {
            return (Int16)((((UInt16)value & 0xFF00) >> 8) | (((UInt16)value & 0x00FF) << 8));
        }

        static UInt16 Reverse(UInt16 value)
        {
            return (UInt16)(((value & 0xFF00) >> 8) | ((value & 0x00FF) << 8));
        }

        static Int32 Reverse(Int32 value)
        {
            return (Int32)(
                (((UInt32)value & 0xFF000000) >> 24) |
                (((UInt32)value & 0x00FF0000) >> 8) |
                (((UInt32)value & 0x0000FF00) << 8) |
                (((UInt32)value & 0x000000FF) << 24));
        }

        static UInt32 Reverse(UInt32 value)
        {
            return (UInt32)(
                ((value & 0xFF000000) >> 24) |
                ((value & 0x00FF0000) >> 8) |
                ((value & 0x0000FF00) << 8) |
                ((value & 0x000000FF) << 24));
        }

        static Int64 Reverse(Int64 value)
        {
            return (Int64)(
                (((UInt64)value & 0xFF00000000000000ULL) >> 56) |
                (((UInt64)value & 0x00FF000000000000ULL) >> 40) |
                (((UInt64)value & 0x0000FF0000000000ULL) >> 24) |
                (((UInt64)value & 0x000000FF00000000ULL) >> 8) |
                (((UInt64)value & 0x00000000FF000000ULL) << 8) |
                (((UInt64)value & 0x0000000000FF0000ULL) << 24) |
                (((UInt64)value & 0x000000000000FF00ULL) << 40) |
                (((UInt64)value & 0x00000000000000FFULL) << 56));
        }

        static UInt64 Reverse(UInt64 value)
        {
            return
                ((value & 0xFF00000000000000ULL) >> 56) |
                ((value & 0x00FF000000000000ULL) >> 40) |
                ((value & 0x0000FF0000000000ULL) >> 24) |
                ((value & 0x000000FF00000000ULL) >> 8) |
                ((value & 0x00000000FF000000ULL) << 8) |
                ((value & 0x0000000000FF0000ULL) << 24) |
                ((value & 0x000000000000FF00ULL) << 40) |
                ((value & 0x00000000000000FFULL) << 56);
        }
    };
}

#endif