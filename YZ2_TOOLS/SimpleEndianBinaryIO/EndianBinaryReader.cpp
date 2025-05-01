#pragma once
#ifndef __EndianBinaryReader
#define __EndianBinaryReader

#include "EndianBitConverter.cpp";

using namespace System;
using namespace System::IO;
using namespace System::Text;

namespace SimpleEndianBinaryIO {

	public ref class EndianBinaryReader : public BinaryReader {
	public:
		virtual property Endianness GetEndianness { Endianness get() { return endianness; } }

		virtual property bool IsNativeEndianness { bool get() { return EndianBitConverter::NativeEndianness == endianness; } }

		EndianBinaryReader(Stream^ stream, Endianness endianness) : BinaryReader(stream)
		{
			this->endianness = endianness;
		}

		EndianBinaryReader(Stream^ stream, Encoding^ encoding, Endianness endianness): BinaryReader(stream, encoding)
		{
			this->endianness = endianness;
		}

		EndianBinaryReader(Stream^ stream, Encoding^ encoding, bool leaveOpen, Endianness endianness) : BinaryReader(stream, encoding, leaveOpen)
		{
			this->endianness = endianness;
		}

		virtual property Int64 Position {
			Int64 get() { return BaseStream->Position; }
			void set(Int64 value) { BaseStream->Position = value; }
		}

		virtual property Int64 Length {
			Int64 get() { return BaseStream->Length; }
		}

		virtual Int64 Seek(Int64 offset, SeekOrigin origin) {
			return BaseStream->Seek(offset, origin);
		}

		virtual Int64 Seek(Int32 offset, SeekOrigin origin) {
			return BaseStream->Seek(offset, origin);
		}

		virtual void Close() override {
			BinaryReader::Close();
		}

		virtual bool Equals(Object^ obj) override {
			return BinaryReader::Equals(obj);
		}

		virtual int GetHashCode() override {
			return BinaryReader::GetHashCode();
		}

		virtual String^ ToString() override {
			return BinaryReader::ToString();
		}

		virtual String^ ReadString() override {
			return BinaryReader::ReadString();
		}

		virtual int PeekChar() override {
			return BinaryReader::PeekChar();
		}

		virtual int Read() override {
			return BinaryReader::Read();
		}

		virtual int Read(array<Char>^ buffer, int index, int count) override {
			return BinaryReader::Read(buffer, index, count);
		}

		virtual int Read(array<Byte>^ buffer, int index, int count) override {
			return BinaryReader::Read(buffer, index, count);
		}

		virtual bool ReadBoolean() override {
			return BinaryReader::ReadBoolean();
		}

		virtual Byte ReadByte() override {
			return BinaryReader::ReadByte();
		}

		virtual SByte ReadSByte() override {
			return BinaryReader::ReadSByte();
		}

		virtual array<Byte>^ ReadBytes(int count) override {
			return BinaryReader::ReadBytes(count);
		}

		virtual Char ReadChar() override {
			return BinaryReader::ReadChar();
		}

		virtual array<Char>^ ReadChars(int count) override {
			return BinaryReader::ReadChars(count);
		}

		virtual Decimal ReadDecimal() override {
			return BinaryReader::ReadDecimal();
		}

		virtual Single ReadSingle() override {
			return ReadSingle(endianness);
		}

		virtual Single ReadSingle(Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				return BinaryReader::ReadSingle();
			}
			else 
			{
				auto value = BinaryReader::ReadUInt32();
				auto reversed = EndianBitConverter::Reverse(value);
				return BitConverter::ToSingle(BitConverter::GetBytes(reversed), 0);
			}
		}

		virtual Double ReadDouble() override {
			return ReadDouble(endianness);
		}

		virtual Double ReadDouble(Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				return BinaryReader::ReadDouble();
			}
			else 
			{
				auto value = BinaryReader::ReadUInt64();
				auto reversed = EndianBitConverter::Reverse(value);
				return BitConverter::ToDouble(BitConverter::GetBytes(reversed), 0);
			}
		}

		virtual Int16 ReadInt16() override {
			return ReadInt16(endianness);
		}

		virtual Int16 ReadInt16(Endianness endianness) {
			auto val = BinaryReader::ReadInt16();
			return (endianness == EndianBitConverter::NativeEndianness) ? val : EndianBitConverter::Reverse(val);
		}

		virtual UInt16 ReadUInt16() override {
			return ReadUInt16(endianness);
		}

		virtual UInt16 ReadUInt16(Endianness endianness) {
			auto val = BinaryReader::ReadUInt16();
			return (endianness == EndianBitConverter::NativeEndianness) ? val : EndianBitConverter::Reverse(val);
		}

		virtual Int32 ReadInt32() override {
			return ReadInt32(endianness);
		}

		virtual Int32 ReadInt32(Endianness endianness) {
			auto val = BinaryReader::ReadInt32();
			return (endianness == EndianBitConverter::NativeEndianness) ? val : EndianBitConverter::Reverse(val);
		}

		virtual UInt32 ReadUInt32() override {
			return ReadUInt32(endianness);
		}

		virtual UInt32 ReadUInt32(Endianness endianness) {
			auto val = BinaryReader::ReadUInt32();
			return (endianness == EndianBitConverter::NativeEndianness) ? val : EndianBitConverter::Reverse(val);
		}

		virtual Int64 ReadInt64() override {
			return ReadInt64(endianness);
		}

		virtual Int64 ReadInt64(Endianness endianness) {
			auto val = BinaryReader::ReadInt64();
			return (endianness == EndianBitConverter::NativeEndianness) ? val : EndianBitConverter::Reverse(val);
		}

		virtual UInt64 ReadUInt64() override {
			return ReadUInt64(endianness);
		}

		virtual UInt64 ReadUInt64(Endianness endianness) {
			auto val = BinaryReader::ReadUInt64();
			return (endianness == EndianBitConverter::NativeEndianness) ? val : EndianBitConverter::Reverse(val);
		}

	protected:
		 Endianness endianness;
	};
}

#endif