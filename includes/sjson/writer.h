#pragma once

////////////////////////////////////////////////////////////////////////////////
// The MIT License (MIT)
//
// Copyright (c) 2018 Nicholas Frechette, Cody Jones, and sjson-cpp contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////////////

#include "sjson/error.h"

#include <functional>
#include <cstdio>
#include <cstdint>
#include <cinttypes>

namespace acl
{
	// TODO: Cleanup the locking stuff, wrap it in #ifdef to strip when asserts are disabled

	class Writer;
	class ArrayWriter;
	class ObjectWriter;

	// TODO: Make this an argument to the writer. For now we assume that SJSON generated files
	// can be shared between various OS and having the most conservative line ending is safer.
	constexpr const char* k_line_terminator = "\r\n";

	class StreamWriter
	{
	public:
		virtual ~StreamWriter() {}

		virtual void write(const void* buffer, size_t buffer_size) = 0;

		void write(const char* str) { write(str, std::strlen(str)); }
	};

	class FileStreamWriter final : public StreamWriter
	{
	public:
		FileStreamWriter(std::FILE* file)
			: m_file(file)
		{}

		virtual void write(const void* buffer, size_t buffer_size) override
		{
			fprintf(m_file, "%s", reinterpret_cast<const char*>(buffer));
		}

	private:
		std::FILE* m_file;
	};

	class ArrayWriter
	{
	public:
		void push_value(const char* value);
		void push_value(bool value);
		void push_value(double value);
		void push_value(float value) { push_value(double(value)); }
		void push_value(int8_t value) { push_signed_integer(value); }
		void push_value(uint8_t value) { push_unsigned_integer(value); }
		void push_value(int16_t value) { push_signed_integer(value); }
		void push_value(uint16_t value) { push_unsigned_integer(value); }
		void push_value(int32_t value) { push_signed_integer(value); }
		void push_value(uint32_t value) { push_unsigned_integer(value); }
		void push_value(int64_t value) { push_signed_integer(value); }
		void push_value(uint64_t value) { push_unsigned_integer(value); }

		void push_object(std::function<void(ObjectWriter& object)> writer_fun);
		void push_array(std::function<void(ArrayWriter& array_writer)> writer_fun);

		void push_newline();

	private:
		ArrayWriter(StreamWriter& stream_writer, uint32_t indent_level);

		ArrayWriter(const ArrayWriter&) = delete;
		ArrayWriter& operator=(const ArrayWriter&) = delete;

		void push_signed_integer(int64_t value);
		void push_unsigned_integer(uint64_t value);
		void write_indentation();

		StreamWriter& m_stream_writer;
		uint32_t m_indent_level;
		bool m_is_empty;
		bool m_is_locked;
		bool m_is_newline;

		friend ObjectWriter;
	};

	class ObjectWriter
	{
	public:
		void insert_value(const char* key, const char* value);
		void insert_value(const char* key, bool value);
		void insert_value(const char* key, double value);
		void insert_value(const char* key, float value) { insert_value(key, double(value)); }
		void insert_value(const char* key, int8_t value) { insert_signed_integer(key, value); }
		void insert_value(const char* key, uint8_t value) { insert_unsigned_integer(key, value); }
		void insert_value(const char* key, int16_t value) { insert_signed_integer(key, value); }
		void insert_value(const char* key, uint16_t value) { insert_unsigned_integer(key, value); }
		void insert_value(const char* key, int32_t value) { insert_signed_integer(key, value); }
		void insert_value(const char* key, uint32_t value) { insert_unsigned_integer(key, value); }
		void insert_value(const char* key, int64_t value) { insert_signed_integer(key, value); }
		void insert_value(const char* key, uint64_t value) { insert_unsigned_integer(key, value); }

		void insert_object(const char* key, std::function<void(ObjectWriter& object_writer)> writer_fun);
		void insert_array(const char* key, std::function<void(ArrayWriter& array_writer)> writer_fun);

		void insert_newline();

		// Implement operator[] for convenience
		class ValueRef
		{
		public:
			ValueRef(ValueRef&& other);
			~ValueRef();

			void operator=(const char* value);
			void operator=(bool value);
			void operator=(double value);
			void operator=(float value) { *this = double(value); }
			void operator=(int8_t value) { assign_signed_integer(value); }
			void operator=(uint8_t value) { assign_unsigned_integer(value); }
			void operator=(int16_t value) { assign_signed_integer(value); }
			void operator=(uint16_t value) { assign_unsigned_integer(value); }
			void operator=(int32_t value) { assign_signed_integer(value); }
			void operator=(uint32_t value) { assign_unsigned_integer(value); }
			void operator=(int64_t value) { assign_signed_integer(value); }
			void operator=(uint64_t value) { assign_unsigned_integer(value); }

			void operator=(std::function<void(ObjectWriter& object_writer)> writer_fun);
			void operator=(std::function<void(ArrayWriter& array_writer)> writer_fun);

		private:
			ValueRef(ObjectWriter& object_writer, const char* key);

			ValueRef(const ValueRef&) = delete;
			ValueRef& operator=(const ValueRef&) = delete;

			void assign_signed_integer(int64_t value);
			void assign_unsigned_integer(uint64_t value);

			ObjectWriter* m_object_writer;
			bool m_is_empty;
			bool m_is_locked;

			friend ObjectWriter;
		};

		ValueRef operator[](const char* key) { return ValueRef(*this, key); }

	protected:
		ObjectWriter(StreamWriter& stream_writer, uint32_t indent_level);

		ObjectWriter(const ObjectWriter&) = delete;
		ObjectWriter& operator=(const ObjectWriter&) = delete;

		void insert_signed_integer(const char* key, int64_t value);
		void insert_unsigned_integer(const char* key, uint64_t value);
		void write_indentation();

		StreamWriter& m_stream_writer;
		uint32_t m_indent_level;
		bool m_is_locked;
		bool m_has_live_value_ref;

		friend ArrayWriter;
	};

	class Writer : public ObjectWriter
	{
	public:
		Writer(StreamWriter& stream_writer);

	private:
		Writer(const Writer&) = delete;
		Writer& operator=(const Writer&) = delete;
	};

	//////////////////////////////////////////////////////////////////////////

	inline ObjectWriter::ObjectWriter(StreamWriter& stream_writer, uint32_t indent_level)
		: m_stream_writer(stream_writer)
		, m_indent_level(indent_level)
		, m_is_locked(false)
		, m_has_live_value_ref(false)
	{}

	inline void ObjectWriter::insert_value(const char* key, const char* value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot insert SJSON value in locked object");
		SJSON_CPP_ENSURE(!m_has_live_value_ref, "Cannot insert SJSON value in object when it has a live ValueRef");

		write_indentation();

		m_stream_writer.write(key);
		m_stream_writer.write(" = \"");
		m_stream_writer.write(value);
		m_stream_writer.write("\"");
		m_stream_writer.write(k_line_terminator);
	}

	inline void ObjectWriter::insert_value(const char* key, bool value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot insert SJSON value in locked object");
		SJSON_CPP_ENSURE(!m_has_live_value_ref, "Cannot insert SJSON value in object when it has a live ValueRef");

		write_indentation();

		m_stream_writer.write(key);
		m_stream_writer.write(" = ");

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%s%s", value ? "true" : "false", k_line_terminator);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to insert SJSON value: [%s = %s]", key, value);
		m_stream_writer.write(buffer, length);
	}

	inline void ObjectWriter::insert_value(const char* key, double value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot insert SJSON value in locked object");
		SJSON_CPP_ENSURE(!m_has_live_value_ref, "Cannot insert SJSON value in object when it has a live ValueRef");

		write_indentation();

		m_stream_writer.write(key);
		m_stream_writer.write(" = ");

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%.10f%s", value, k_line_terminator);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to insert SJSON value: [%s = %.10f]", key, value);
		m_stream_writer.write(buffer, length);
	}

	inline void ObjectWriter::insert_signed_integer(const char* key, int64_t value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot insert SJSON value in locked object");
		SJSON_CPP_ENSURE(!m_has_live_value_ref, "Cannot insert SJSON value in object when it has a live ValueRef");

		write_indentation();

		m_stream_writer.write(key);
		m_stream_writer.write(" = ");

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%" PRId64 "%s", value, k_line_terminator);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to insert SJSON value: [%s = %lld]", key, value);
		m_stream_writer.write(buffer, length);
	}

	inline void ObjectWriter::insert_unsigned_integer(const char* key, uint64_t value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot insert SJSON value in locked object");
		SJSON_CPP_ENSURE(!m_has_live_value_ref, "Cannot insert SJSON value in object when it has a live ValueRef");

		write_indentation();

		m_stream_writer.write(key);
		m_stream_writer.write(" = ");

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%" PRIu64 "%s", value, k_line_terminator);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to insert SJSON value: [%s = %llu]", key, value);
		m_stream_writer.write(buffer, length);
	}

	inline void ObjectWriter::insert_object(const char* key, std::function<void(ObjectWriter& object_writer)> writer_fun)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot insert SJSON object in locked object");
		SJSON_CPP_ENSURE(!m_has_live_value_ref, "Cannot insert SJSON object in object when it has a live ValueRef");

		write_indentation();

		m_stream_writer.write(key);
		m_stream_writer.write(" = {");
		m_stream_writer.write(k_line_terminator);
		m_is_locked = true;

		ObjectWriter object_writer(m_stream_writer, m_indent_level + 1);
		writer_fun(object_writer);

		m_is_locked = false;
		write_indentation();

		m_stream_writer.write("}");
		m_stream_writer.write(k_line_terminator);
	}

	inline void ObjectWriter::insert_array(const char* key, std::function<void(ArrayWriter& array_writer)> writer_fun)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot insert SJSON array in locked object");
		SJSON_CPP_ENSURE(!m_has_live_value_ref, "Cannot insert SJSON array in object when it has a live ValueRef");

		write_indentation();

		m_stream_writer.write(key);
		m_stream_writer.write(" = [ ");
		m_is_locked = true;

		ArrayWriter array_writer(m_stream_writer, m_indent_level + 1);
		writer_fun(array_writer);

		if (array_writer.m_is_newline)
		{
			write_indentation();
			m_stream_writer.write("]");
			m_stream_writer.write(k_line_terminator);
		}
		else
		{
			m_stream_writer.write(" ]");
			m_stream_writer.write(k_line_terminator);
		}

		m_is_locked = false;
	}

	inline void ObjectWriter::write_indentation()
	{
		for (uint32_t level = 0; level < m_indent_level; ++level)
			m_stream_writer.write("\t");
	}

	inline void ObjectWriter::insert_newline()
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot insert newline in locked object");
		SJSON_CPP_ENSURE(!m_has_live_value_ref, "Cannot insert newline in object when it has a live ValueRef");

		m_stream_writer.write(k_line_terminator);
	}

	inline ObjectWriter::ValueRef::ValueRef(ObjectWriter& object_writer, const char* key)
		: m_object_writer(&object_writer)
		, m_is_empty(true)
		, m_is_locked(false)
	{
		SJSON_CPP_ENSURE(!object_writer.m_is_locked, "Cannot insert SJSON value in locked object");
		SJSON_CPP_ENSURE(!object_writer.m_has_live_value_ref, "Cannot insert SJSON value in object when it has a live ValueRef");

		object_writer.write_indentation();
		object_writer.m_stream_writer.write(key);
		object_writer.m_stream_writer.write(" = ");
		object_writer.m_has_live_value_ref = true;
		object_writer.m_is_locked = true;
	}

	inline ObjectWriter::ValueRef::ValueRef(ValueRef&& other)
		: m_object_writer(other.m_object_writer)
		, m_is_empty(other.m_is_empty)
		, m_is_locked(other.m_is_locked)
	{
		other.m_object_writer = nullptr;
	}

	inline ObjectWriter::ValueRef::~ValueRef()
	{
		if (m_object_writer != nullptr)
		{
			SJSON_CPP_ENSURE(!m_is_empty, "ValueRef has no associated value");
			SJSON_CPP_ENSURE(!m_is_locked, "ValueRef is locked");
			SJSON_CPP_ENSURE(m_object_writer->m_has_live_value_ref, "Expected a live ValueRef to be present");
			SJSON_CPP_ENSURE(m_object_writer->m_is_locked, "Expected object writer to be locked");

			m_object_writer->m_has_live_value_ref = false;
			m_object_writer->m_is_locked = false;
		}
	}

	inline void ObjectWriter::ValueRef::operator=(const char* value)
	{
		SJSON_CPP_ENSURE(m_is_empty, "Cannot write multiple values within a ValueRef");
		SJSON_CPP_ENSURE(m_object_writer != nullptr, "ValueRef not initialized");
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot assign a value when locked");

		m_object_writer->m_stream_writer.write("\"");
		m_object_writer->m_stream_writer.write(value);
		m_object_writer->m_stream_writer.write("\"");
		m_object_writer->m_stream_writer.write(k_line_terminator);
		m_is_empty = false;
	}

	inline void ObjectWriter::ValueRef::operator=(bool value)
	{
		SJSON_CPP_ENSURE(m_is_empty, "Cannot write multiple values within a ValueRef");
		SJSON_CPP_ENSURE(m_object_writer != nullptr, "ValueRef not initialized");
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot assign a value when locked");

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%s%s", value ? "true" : "false", k_line_terminator);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to assign SJSON value: %s", value);
		m_object_writer->m_stream_writer.write(buffer, length);
		m_is_empty = false;
	}

	inline void ObjectWriter::ValueRef::operator=(double value)
	{
		SJSON_CPP_ENSURE(m_is_empty, "Cannot write multiple values within a ValueRef");
		SJSON_CPP_ENSURE(m_object_writer != nullptr, "ValueRef not initialized");
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot assign a value when locked");

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%.10f%s", value, k_line_terminator);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to assign SJSON value: %.10f", value);
		m_object_writer->m_stream_writer.write(buffer, length);
		m_is_empty = false;
	}

	inline void ObjectWriter::ValueRef::operator=(std::function<void(ObjectWriter& object_writer)> writer_fun)
	{
		SJSON_CPP_ENSURE(m_is_empty, "Cannot write multiple values within a ValueRef");
		SJSON_CPP_ENSURE(m_object_writer != nullptr, "ValueRef not initialized");
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot assign a value when locked");

		m_object_writer->m_stream_writer.write("{");
		m_object_writer->m_stream_writer.write(k_line_terminator);
		m_is_locked = true;

		ObjectWriter object_writer(m_object_writer->m_stream_writer, m_object_writer->m_indent_level + 1);
		writer_fun(object_writer);

		m_is_locked = false;
		m_object_writer->write_indentation();
		m_object_writer->m_stream_writer.write("}");
		m_object_writer->m_stream_writer.write(k_line_terminator);
		m_is_empty = false;
	}

	inline void ObjectWriter::ValueRef::operator=(std::function<void(ArrayWriter& array_writer)> writer_fun)
	{
		SJSON_CPP_ENSURE(m_is_empty, "Cannot write multiple values within a ValueRef");
		SJSON_CPP_ENSURE(m_object_writer != nullptr, "ValueRef not initialized");
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot assign a value when locked");

		m_object_writer->m_stream_writer.write("[ ");
		m_is_locked = true;

		ArrayWriter array_writer(m_object_writer->m_stream_writer, m_object_writer->m_indent_level + 1);
		writer_fun(array_writer);

		if (array_writer.m_is_newline)
		{
			m_object_writer->write_indentation();
			m_object_writer->m_stream_writer.write("]");
			m_object_writer->m_stream_writer.write(k_line_terminator);
		}
		else
		{
			m_object_writer->m_stream_writer.write(" ]");
			m_object_writer->m_stream_writer.write(k_line_terminator);
		}

		m_is_locked = false;
		m_is_empty = false;
	}

	inline void ObjectWriter::ValueRef::assign_signed_integer(int64_t value)
	{
		SJSON_CPP_ENSURE(m_is_empty, "Cannot write multiple values within a ValueRef");
		SJSON_CPP_ENSURE(m_object_writer != nullptr, "ValueRef not initialized");
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot assign a value when locked");

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%" PRId64 "%s", value, k_line_terminator);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to assign SJSON value: %lld", value);
		m_object_writer->m_stream_writer.write(buffer, length);
		m_is_empty = false;
	}

	inline void ObjectWriter::ValueRef::assign_unsigned_integer(uint64_t value)
	{
		SJSON_CPP_ENSURE(m_is_empty, "Cannot write multiple values within a ValueRef");
		SJSON_CPP_ENSURE(m_object_writer != nullptr, "ValueRef not initialized");
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot assign a value when locked");

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%" PRIu64 "%s", value, k_line_terminator);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to assign SJSON value: %llu", value);
		m_object_writer->m_stream_writer.write(buffer, length);
		m_is_empty = false;
	}

	//////////////////////////////////////////////////////////////////////////

	inline ArrayWriter::ArrayWriter(StreamWriter& stream_writer, uint32_t indent_level)
		: m_stream_writer(stream_writer)
		, m_indent_level(indent_level)
		, m_is_empty(true)
		, m_is_locked(false)
		, m_is_newline(false)
	{}

	inline void ArrayWriter::push_value(const char* value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot push SJSON value in locked array");

		if (!m_is_empty && !m_is_newline)
			m_stream_writer.write(", ");

		if (m_is_newline)
			write_indentation();

		m_stream_writer.write("\"");
		m_stream_writer.write(value);
		m_stream_writer.write("\"");
		m_is_empty = false;
		m_is_newline = false;
	}

	inline void ArrayWriter::push_value(bool value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot push SJSON value in locked array");

		if (!m_is_empty && !m_is_newline)
			m_stream_writer.write(", ");

		if (m_is_newline)
			write_indentation();

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%s", value ? "true" : "false");
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to push SJSON value: %s", value);
		m_stream_writer.write(buffer, length);
		m_is_empty = false;
		m_is_newline = false;
	}

	inline void ArrayWriter::push_value(double value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot push SJSON value in locked array");

		if (!m_is_empty && !m_is_newline)
			m_stream_writer.write(", ");

		if (m_is_newline)
			write_indentation();

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%.10f", value);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to push SJSON value: %.10f", value);
		m_stream_writer.write(buffer, length);
		m_is_empty = false;
		m_is_newline = false;
	}

	inline void ArrayWriter::push_signed_integer(int64_t value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot push SJSON value in locked array");

		if (!m_is_empty && !m_is_newline)
			m_stream_writer.write(", ");

		if (m_is_newline)
			write_indentation();

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%" PRId64, value);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to push SJSON value: %lld", value);
		m_stream_writer.write(buffer, length);
		m_is_empty = false;
		m_is_newline = false;
	}

	inline void ArrayWriter::push_unsigned_integer(uint64_t value)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot push SJSON value in locked array");

		if (!m_is_empty && !m_is_newline)
			m_stream_writer.write(", ");

		if (m_is_newline)
			write_indentation();

		char buffer[256];
		size_t length = snprintf(buffer, sizeof(buffer), "%" PRIu64, value);
		SJSON_CPP_ENSURE(length > 0 && length < sizeof(buffer), "Failed to push SJSON value: %llu", value);
		m_stream_writer.write(buffer, length);
		m_is_empty = false;
		m_is_newline = false;
	}

	inline void ArrayWriter::push_object(std::function<void(ObjectWriter& object_writer)> writer_fun)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot push SJSON object in locked array");

		if (!m_is_empty && !m_is_newline)
		{
			m_stream_writer.write(",");
			m_stream_writer.write(k_line_terminator);
		}
		else if (m_is_empty)
			m_stream_writer.write(k_line_terminator);

		write_indentation();
		m_stream_writer.write("{");
		m_stream_writer.write(k_line_terminator);
		m_is_locked = true;

		ObjectWriter object_writer(m_stream_writer, m_indent_level + 1);
		writer_fun(object_writer);

		write_indentation();
		m_stream_writer.write("}");
		m_stream_writer.write(k_line_terminator);

		m_is_locked = false;
		m_is_empty = false;
		m_is_newline = true;
	}

	inline void ArrayWriter::push_array(std::function<void(ArrayWriter& array_writer)> writer_fun)
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot push SJSON array in locked array");

		if (!m_is_empty && !m_is_newline)
			m_stream_writer.write(", ");

		if (m_is_newline)
			write_indentation();

		m_stream_writer.write("[ ");
		m_is_locked = true;

		ArrayWriter array_writer(m_stream_writer, m_indent_level);
		writer_fun(array_writer);

		m_is_locked = false;
		m_stream_writer.write(" ]");
		m_is_empty = false;
		m_is_newline = false;
	}

	inline void ArrayWriter::push_newline()
	{
		SJSON_CPP_ENSURE(!m_is_locked, "Cannot insert newline in locked array");

		m_stream_writer.write(k_line_terminator);
		m_is_newline = true;
	}

	inline void ArrayWriter::write_indentation()
	{
		for (uint32_t level = 0; level < m_indent_level; ++level)
			m_stream_writer.write("\t");
	}

	//////////////////////////////////////////////////////////////////////////

	inline Writer::Writer(StreamWriter& stream_writer)
		: ObjectWriter(stream_writer, 0)
	{}
}
