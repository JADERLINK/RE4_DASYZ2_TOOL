#pragma once
#ifndef __EndianBinaryWriter
#define __EndianBinaryWriter

#include "EndianBitConverter.cpp";

using namespace System;
using namespace System::IO;
using namespace System::Text;

namespace SimpleEndianBinaryIO {

	public ref class EndianBinaryWriter : public BinaryWriter {
	public:
		virtual property Endianness GetEndianness { Endianness get() { return endianness; } }

		virtual property bool IsNativeEndianness { bool get() { return EndianBitConverter::NativeEndianness == endianness; } }

		EndianBinaryWriter(Stream^ stream, Endianness endianness) : BinaryWriter(stream)
		{
			this->endianness = endianness;
		}

		EndianBinaryWriter(Stream^ stream, Encoding^ encoding, Endianness endianness) : BinaryWriter(stream, encoding)
		{
			this->endianness = endianness;
		}

		EndianBinaryWriter(Stream^ stream, Encoding^ encoding, bool leaveOpen, Endianness endianness) : BinaryWriter(stream, encoding, leaveOpen)
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

		virtual Int64 Seek(Int32 offset, SeekOrigin origin) override {
			return BaseStream->Seek(offset, origin);
		}

		virtual void Close() override {
			BinaryWriter::Close();
		}

		virtual bool Equals(Object^ obj) override {
			return BinaryWriter::Equals(obj);
		}

		virtual int GetHashCode() override {
			return BinaryWriter::GetHashCode();
		}

		virtual String^ ToString() override {
			return BinaryWriter::ToString();
		}

		virtual void Write(String^ value) override {
			BinaryWriter::Write(value);
		}

		virtual void Write(array<Byte>^ buffer) override {
			BinaryWriter::Write(buffer);
		}

		virtual void Write(array<Byte>^ buffer, int index, int count) override {
			BinaryWriter::Write(buffer, index, count);
		}

		virtual void Write(array<Char>^ chars) override {
			BinaryWriter::Write(chars);
		}

		virtual void Write(array<Char>^ chars, int index, int count) override {
			BinaryWriter::Write(chars, index, count);
		}

		virtual void Write(bool value) override {
			BinaryWriter::Write(value);
		}

		virtual void Write(Byte value) override {
			BinaryWriter::Write(value);
		}

		virtual void Write(SByte value) override {
			BinaryWriter::Write(value);
		}

		virtual void Write(Char ch) override {
			BinaryWriter::Write(ch);
		}

		virtual void Write(Decimal value) override {
			BinaryWriter::Write(value);
		}

		virtual void Write(Single value) override {
			Write(value, endianness);
		}

		virtual void Write(Single value, Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				BinaryWriter::Write(value);
			}
			else
			{
				BinaryWriter::Write(EndianBitConverter::Reverse(BitConverter::ToUInt32(BitConverter::GetBytes(value), 0)));
			}
		}

		virtual void Write(Double value) override {
			Write(value, endianness);
		}

		virtual void Write(Double value, Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				BinaryWriter::Write(value);
			}
			else
			{
				BinaryWriter::Write(EndianBitConverter::Reverse(BitConverter::ToUInt64(BitConverter::GetBytes(value), 0)));
			}
		}

		virtual void Write(Int16 value) override {
			Write(value, endianness);
		}

		virtual void Write(Int16 value, Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				BinaryWriter::Write(value);
			}
			else
			{
				BinaryWriter::Write(EndianBitConverter::Reverse(value));
			}
		}

		virtual void Write(UInt16 value) override {
			Write(value, endianness);
		}

		virtual void Write(UInt16 value, Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				BinaryWriter::Write(value);
			}
			else
			{
				BinaryWriter::Write(EndianBitConverter::Reverse(value));
			}
		}

		virtual void Write(Int32 value) override {
			Write(value, endianness);
		}

		virtual void Write(Int32 value, Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				BinaryWriter::Write(value);
			}
			else
			{
				BinaryWriter::Write(EndianBitConverter::Reverse(value));
			}
		}

		virtual void Write(UInt32 value) override {
			Write(value, endianness);
		}

		virtual void Write(UInt32 value, Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				BinaryWriter::Write(value);
			}
			else
			{
				BinaryWriter::Write(EndianBitConverter::Reverse(value));
			}
		}

		virtual void Write(Int64 value) override {
			Write(value, endianness);
		}

		virtual void Write(Int64 value, Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				BinaryWriter::Write(value);
			}
			else
			{
				BinaryWriter::Write(EndianBitConverter::Reverse(value));
			}
		}

		virtual void Write(UInt64 value) override {
			Write(value, endianness);
		}

		virtual void Write(UInt64 value, Endianness endianness) {
			if (endianness == EndianBitConverter::NativeEndianness)
			{
				BinaryWriter::Write(value);
			}
			else
			{
				BinaryWriter::Write(EndianBitConverter::Reverse(value));
			}
		}

	protected:
		Endianness endianness;
	};
}

#endif