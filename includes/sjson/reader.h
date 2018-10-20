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

// This define allows external libraries using sjson-cpp to detect if it has already be included as a dependency
#if !defined(SJSON_CPP_READER)
	#define SJSON_CPP_READER
#endif

#include "sjson/error.h"
#include "sjson/string_view.h"
#include "sjson/platform.h"

#include <cctype>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <algorithm>
#include <type_traits>

namespace sjson
{
	class ReaderError
	{
	public:
		ReaderError() : m_error(nullptr) {}
		explicit ReaderError(const char* error) : m_error(error) {}

		bool empty() const { return m_error == nullptr; }
		bool any() const { return m_error != nullptr; }
		const char* c_str() const { return m_error; }
		void reset() { m_error = nullptr; }

	private:
		const char*		m_error;
	};

	enum class ValueType
	{
		Unknown,
		Null,
		Bool,
		String,
		Number,
		Array,
		Object,
	};

	class ValueReader;

	namespace impl
	{
		class PairReaderIterator;

		struct ReaderContext
		{
			StringView		str;
			size_t			offset;

			ReaderContext*	parent;

			bool is_eof() const { return offset >= str.size(); }
			size_t remaining_size() const { return str.size() - offset; }
		};

		static constexpr ReaderContext k_invalid_context = { StringView(), size_t(~0ull), nullptr };

		inline bool is_eof(const ReaderContext& context, size_t offset) { return offset >= context.str.size(); }

		inline void skip_bom(ReaderContext& context)
		{
			if (context.remaining_size() < 3)
				return;

			size_t offset = context.offset;

			if (context.str[offset++] == char(uint8_t(0xEF)))
			{
				if (context.str[offset++] == char(uint8_t(0xBB)))
				{
					if (context.str[offset++] == char(uint8_t(0xBF)))
						context.offset = offset;
				}
			}
		}

		inline ReaderError skip_comment(ReaderContext& context, size_t& inout_offset)
		{
			SJSON_CPP_ASSERT(context.str[inout_offset] == '/', "Expected '/'");

			size_t offset = inout_offset + 1;
			if (is_eof(context, offset))
				return ReaderError("Input truncated");

			if (context.str[offset] == '/')
			{
				while (!is_eof(context, offset) && context.str[offset] != '\n')
					offset++;

				inout_offset = offset;
				return ReaderError();
			}
			else if (context.str[offset] == '*')
			{
				offset++;

				char prev_symbol = '\0';

				while (true)
				{
					if (is_eof(context, offset))
						return ReaderError("Input truncated");

					const char symbol = context.str[offset++];
					if (symbol == '/' && prev_symbol == '*')
					{
						inout_offset = offset;
						return ReaderError();
					}

					prev_symbol = symbol;
				}
			}
			else
				return ReaderError("Comment begins incorrectly");
		}

		inline ReaderError skip_comments_and_whitespace(ReaderContext& context)
		{
			size_t offset = context.offset;

			while (true)
			{
				if (is_eof(context, offset))
				{
					context.offset = offset;
					return ReaderError();
				}

				const char symbol = context.str[offset];
				if (std::isspace(symbol))
				{
					offset++;
					continue;
				}

				if (symbol == '/')
				{
					ReaderError err = skip_comment(context, offset);
					if (err.any())
						return err;

					continue;
				}

				context.offset = offset;
				return ReaderError();
			}
		}

		// The StringView value returned is a raw view of the SJSON buffer. Nothing is unescaped:
		// escaped quotation marks will remain, escaped unicode sequences will remain, etc.
		// It is the responsibility of the caller to handle this in a meaningful way.
		inline ReaderError read_string(ReaderContext& context, StringView& out_value)
		{
			if (context.is_eof())
				return ReaderError("Input truncated");

			if (context.str[context.offset] != '"')
				return ReaderError("Quotation mark expected");

			size_t offset = context.offset + 1;

			while (true)
			{
				if (is_eof(context, offset))
					return ReaderError("Input truncated");

				if (context.str[offset] == '"')
				{
					offset++;
					break;
				}

				if (context.str[offset++] == '\\')
				{
					// Strings are returned as slices of the input, so escape sequences cannot be un-escaped.
					// Assume the escape sequence is valid and skip over it.
					if (context.str[offset++] == 'u')
					{
						// This is an escaped unicode character, skip the 4 bytes that follow
						offset++;
						offset++;
						offset++;
						offset++;
					}
				}
			}

			out_value = StringView(context.str.c_str() + context.offset + 1, offset - context.offset - 2);
			context.offset = offset;
			return ReaderError();
		}

		// Unquoted strings do not support escaped unicode literals or any form of escaping
		// e.g. foo_\u0066_bar = "this is an invalid key"
		inline ReaderError read_unquoted_string(ReaderContext& context, StringView& out_value)
		{
			if (context.is_eof())
				return ReaderError("Input truncated");

			size_t offset = context.offset;

			while (true)
			{
				if (is_eof(context, offset))
					return ReaderError("Input truncated");

				const char symbol = context.str[offset];
				if (symbol == '"')
					return ReaderError("Cannot use quotation mark in unquoted string");

				if (symbol == '=' || std::isspace(symbol))
				{
					if (offset == context.offset)
						return ReaderError("Empty unquoted string found");

					break;
				}

				offset++;
			}

			out_value = StringView(context.str.c_str() + context.offset, offset - context.offset);
			context.offset = offset;
			return ReaderError();
		}

		inline ReaderError read_pair_name(ReaderContext& context, StringView& out_name)
		{
			if (context.is_eof())
				return ReaderError("Input truncated");

			if (context.str[context.offset] == '"')
				return read_string(context, out_name);
			else
				return read_unquoted_string(context, out_name);
		}

		inline ReaderError read_bool(ReaderContext& context, bool& out_value)
		{
			if (context.is_eof())
				return ReaderError("Input truncated");

			if (context.str[context.offset] == 't')
			{
				if (context.remaining_size() < 4)
					return ReaderError("Input truncated");

				if (context.str[context.offset + 1] == 'r' &&
					context.str[context.offset + 2] == 'u' &&
					context.str[context.offset + 3] == 'e')
				{
					out_value = true;
					context.offset += 4;
					return ReaderError();
				}
			}
			else if (context.str[context.offset] == 'f')
			{
				if (context.remaining_size() < 5)
					return ReaderError("Input truncated");

				if (context.str[context.offset + 1] == 'a' &&
					context.str[context.offset + 2] == 'l' &&
					context.str[context.offset + 3] == 's' &&
					context.str[context.offset + 4] == 'e')
				{
					out_value = false;
					context.offset += 5;
					return ReaderError();
				}
			}

			return ReaderError("'true' or 'false' expected");
		}

		inline ReaderError read_null(ReaderContext& context)
		{
			if (context.is_eof())
				return ReaderError("Input truncated");

			if (context.remaining_size() < 4)
				return ReaderError("Input truncated");

			if (context.str[context.offset + 0] == 'n' &&
				context.str[context.offset + 1] == 'u' &&
				context.str[context.offset + 2] == 'l' &&
				context.str[context.offset + 3] == 'l')
			{
				context.offset += 4;
				return ReaderError();
			}

			return ReaderError("'null' expected");
		}

		inline bool is_hex_digit(char value)
		{
			return std::isdigit(value)
				|| value == 'a' || value == 'A'
				|| value == 'b' || value == 'B'
				|| value == 'c' || value == 'C'
				|| value == 'd' || value == 'D'
				|| value == 'e' || value == 'E'
				|| value == 'f' || value == 'F';
		}

		inline ReaderError read_number(ReaderContext& context, StringView& out_number, int& out_base)
		{
			if (context.is_eof())
				return ReaderError("Input truncated");

			size_t offset = context.offset;
			int base;

			if (context.str[offset] == 'x' || context.str[offset] == 'X')
			{
				// Note: This is a divergence from the classic JSON/SJSON number format,
				// we support hexadecimal digits
				offset++;
				base = 16;

				while (is_hex_digit(context.str[offset]))
					offset++;
			}
			else
			{
				base = 10;
				if (context.str[offset] == '-')
					offset++;

				if (!std::isdigit(context.str[offset]))
					return ReaderError("Number expected");

				if (context.str[offset] == '0' && context.str[offset + 1] != '.')
				{
					// Just 0
					offset++;
				}
				else
				{
					if (context.str[offset] == '0')
					{
						if (context.str[offset + 1] != '.')
							return ReaderError("Decimal dot expected");

						offset += 2;

						while (!is_eof(context, offset) && std::isdigit(context.str[offset]))
							offset++;
					}
					else
					{
						while (!is_eof(context, offset) && std::isdigit(context.str[offset]))
							offset++;

						if (!is_eof(context, offset) && context.str[offset] == '.')
						{
							offset++;

							while (!is_eof(context, offset) && std::isdigit(context.str[offset]))
								offset++;
						}
					}

					if (!is_eof(context, offset) && (context.str[offset] == 'e' || context.str[offset] == 'E'))
					{
						offset++;

						if (!is_eof(context, offset) && (context.str[offset] == '+' || context.str[offset] == '-'))
						{
							offset++;

							if (!std::isdigit(context.str[offset]))
								return ReaderError("Invalid number");
						}
						else if (!std::isdigit(context.str[offset]))
							return ReaderError("Invalid number");

						while (!is_eof(context, offset) && std::isdigit(context.str[offset]))
							offset++;
					}
				}
			}

			out_number = StringView(context.str.c_str() + context.offset, offset - context.offset);
			out_base = base;
			context.offset = offset;
			return ReaderError();
		}

		template<typename IntegralType>
		inline ReaderError number_to_integer(const StringView& number, int base, IntegralType& out_value)
		{
			if (std::is_unsigned<IntegralType>::value)
			{
				char* last_used_symbol = nullptr;
				const uint64_t raw_value = impl::strtoull(number.c_str(), &last_used_symbol, base);
				if (size_t(last_used_symbol - number.c_str()) != number.size())
					return ReaderError("Number could not be converted");

				out_value = static_cast<IntegralType>(raw_value);
				return ReaderError();
			}
			else
			{
				char* last_used_symbol = nullptr;
				const int64_t raw_value = impl::strtoll(number.c_str(), &last_used_symbol, base);
				if (size_t(last_used_symbol - number.c_str()) != number.size())
					return ReaderError("Number could not be converted");

				out_value = static_cast<IntegralType>(raw_value);
				return ReaderError();
			}
		}

		inline ReaderError number_to_double(const StringView& number, int base, double& out_value)
		{
			if (base == 10)
			{
				char* last_used_symbol = nullptr;
				const double value = std::strtod(number.c_str(), &last_used_symbol);
				if (size_t(last_used_symbol - number.c_str()) != number.size())
					return ReaderError("Number could not be converted");

				out_value = value;
				return ReaderError();
			}
			else
			{
				int64_t value;
				ReaderError error = number_to_integer<int64_t>(number, base, value);
				if (error.any())
					return error;

				out_value = double(value);
				return ReaderError();
			}
		}

		inline ReaderError number_to_float(const StringView& number, int base, float& out_value)
		{
			if (base == 10)
			{
				char* last_used_symbol = nullptr;
				const float value = impl::strtof(number.c_str(), &last_used_symbol);
				if (size_t(last_used_symbol - number.c_str()) != number.size())
					return ReaderError("Number could not be converted");

				out_value = value;
				return ReaderError();
			}
			else
			{
				int64_t value;
				ReaderError error = number_to_integer<int64_t>(number, base, value);
				if (error.any())
					return error;

				out_value = float(value);
				return ReaderError();
			}
		}

		inline ReaderError skip_value(ReaderContext& context, const ReaderContext& value_context);

		struct PairReaderProxy
		{
			const StringView& name;
			const ReaderContext& context;

			PairReaderProxy(const StringView& name_, const ReaderContext& context_)
				: name(name_)
				, context(context_)
			{}
		};

		class PairReaderIterator
		{
		public:
			PairReaderIterator() : m_context(k_invalid_context), m_out_error(nullptr), m_is_root_object(false), m_value_offset(0), m_pair_name(), m_pair_context() {}
			PairReaderIterator(const ReaderContext& context, bool is_root_object, ReaderError* out_error)
				: m_context(context)
				, m_out_error(out_error)
				, m_is_root_object(is_root_object)
				, m_value_offset(context.offset - 1)
				, m_pair_name()
				, m_pair_context()
			{
				if (context.is_eof())
					return;

				if (is_root_object)
				{
					SJSON_CPP_ASSERT(!std::isspace(context.str[context.offset]), "Expected a value");
				}
				else
				{
					SJSON_CPP_ASSERT(context.str[context.offset] == '{', "Expected a '{'");

					m_context.offset++;		// Skip '{'
				}

				++(*this);
			}

			inline PairReaderIterator& operator++()
			{
				if (m_context.offset == m_value_offset)
				{
					ReaderError error = skip_value();
					if (error.any())
					{
						if (m_out_error != nullptr)
							*m_out_error = error;

						m_context = k_invalid_context;
						return *this;
					}
				}

				ReaderError error = skip_comments_and_whitespace(m_context);
				if (error.any())
				{
					if (m_out_error != nullptr)
						*m_out_error = error;

					m_context = k_invalid_context;
					return *this;
				}

				if (m_context.is_eof())
				{
					m_context = k_invalid_context;
					return *this;
				}

				if (m_context.str[m_context.offset] == ',')
				{
					// Skip and consume
					m_context.offset++;

					error = skip_comments_and_whitespace(m_context);
					if (error.any())
					{
						if (m_out_error != nullptr)
							*m_out_error = error;

						m_context = impl::k_invalid_context;
						return *this;
					}

					if (m_context.is_eof())
					{
						if (m_out_error != nullptr)
							*m_out_error = ReaderError("Input truncated");

						m_context = impl::k_invalid_context;
						return *this;
					}
				}

				if (!m_is_root_object && m_context.str[m_context.offset] == '}')
				{
					// We are done, skip, and consume
					m_context.offset++;	// TODO: Useless?

					// TODO: Just update the offset?
					m_pair_context = m_context;
					m_pair_context.parent = &m_context;
					m_context = k_invalid_context;
					return *this;
				}

				error = read_pair_name(m_context, m_pair_name);
				if (error.any())
				{
					if (m_out_error != nullptr)
						*m_out_error = error;

					m_context = k_invalid_context;
					return *this;
				}

				error = skip_comments_and_whitespace(m_context);
				if (error.any())
				{
					if (m_out_error != nullptr)
						*m_out_error = error;

					m_context = k_invalid_context;
					return *this;
				}

				if (m_context.is_eof())
				{
					if (m_out_error != nullptr)
						*m_out_error = ReaderError("Input truncated");

					m_context = k_invalid_context;
					return *this;
				}

				if (m_context.str[m_context.offset] != '=')
				{
					if (m_out_error != nullptr)
						*m_out_error = ReaderError("Equal sign expected");

					m_context = k_invalid_context;
					return *this;
				}

				m_context.offset++;

				error = skip_comments_and_whitespace(m_context);
				if (error.any())
				{
					if (m_out_error != nullptr)
						*m_out_error = error;

					m_context = k_invalid_context;
					return *this;
				}

				m_value_offset = m_context.offset;

				ReaderContext ctx = m_context;
				ctx.parent = &m_context;
				m_pair_context = ctx;

				return *this;
			}

			inline PairReaderProxy operator*() const { return PairReaderProxy(m_pair_name, m_pair_context); }

			inline bool operator==(const PairReaderIterator& other) const { return m_context.offset == other.m_context.offset; }
			inline bool operator!=(const PairReaderIterator& other) const { return m_context.offset != other.m_context.offset; }

		private:
			inline ReaderError skip_value() { return impl::skip_value(m_context, m_pair_context); }

			ReaderContext	m_context;
			ReaderError*	m_out_error;
			bool			m_is_root_object;

			size_t			m_value_offset;
			StringView		m_pair_name;
			ReaderContext	m_pair_context;

			friend ReaderError impl::skip_value(ReaderContext& context, const ReaderContext& value_context);
		};

		class ValueReaderIterator
		{
		public:
			ValueReaderIterator() : m_context(impl::k_invalid_context), m_value_context(impl::k_invalid_context), m_out_error(nullptr), m_value_offset(0) {}
			ValueReaderIterator(const impl::ReaderContext& context, ReaderError* out_error)
				: m_context(context)
				, m_value_context(impl::k_invalid_context)
				, m_out_error(out_error)
				, m_value_offset(context.offset - 1)
			{
				if (context.is_eof())
					return;

				SJSON_CPP_ASSERT(context.str[context.offset] == '[', "Expected a '['");

				m_context.offset++;		// Skip '['

				++(*this);
			}

			inline ValueReaderIterator& operator++()
			{
				if (m_context.offset == m_value_offset)
				{
					ReaderError error = skip_value();
					if (error.any())
					{
						if (m_out_error != nullptr)
							*m_out_error = error;

						m_context = impl::k_invalid_context;
						return *this;
					}
				}

				ReaderError error = skip_comments_and_whitespace(m_context);
				if (error.any())
				{
					if (m_out_error != nullptr)
						*m_out_error = error;

					m_context = impl::k_invalid_context;
					return *this;
				}

				if (m_context.is_eof())
				{
					m_context = impl::k_invalid_context;
					return *this;
				}

				if (m_context.str[m_context.offset] == ',')
				{
					// Skip and consume
					m_context.offset++;

					error = skip_comments_and_whitespace(m_context);
					if (error.any())
					{
						if (m_out_error != nullptr)
							*m_out_error = error;

						m_context = impl::k_invalid_context;
						return *this;
					}

					if (m_context.is_eof())
					{
						if (m_out_error != nullptr)
							*m_out_error = ReaderError("Input truncated");

						m_context = impl::k_invalid_context;
						return *this;
					}
				}

				if (m_context.str[m_context.offset] == ']')
				{
					// We are done, skip, and consume
					m_context.offset++;	// TODO: Useless?

					// TODO: Just update the offset?
					m_value_context = m_context;
					m_value_context.parent = &m_context;
					m_context = impl::k_invalid_context;
					return *this;
				}

				m_value_offset = m_context.offset;

				// TODO: Just update the offset?
				m_value_context = m_context;
				m_value_context.parent = &m_context;

				return *this;
			}

			inline const impl::ReaderContext& operator*() const { return m_value_context; }

			inline bool operator==(const ValueReaderIterator& other) const { return m_context.offset == other.m_context.offset; }
			inline bool operator!=(const ValueReaderIterator& other) const { return m_context.offset != other.m_context.offset; }

		private:
			inline ReaderError skip_value()
			{
				return impl::skip_value(m_context, m_value_context);
			}

			impl::ReaderContext	m_context;
			impl::ReaderContext	m_value_context;
			ReaderError*	m_out_error;

			size_t			m_value_offset;		// TODO: Can we remove this and use m_value_context.offset?

			friend ReaderError impl::skip_value(ReaderContext& context, const ReaderContext& value_context);
		};
	}

	class PairReaderList
	{
	public:
		inline PairReaderList(const impl::ReaderContext& context, bool is_root_object, ReaderError* out_error)
			: m_context(context)
			, m_out_error(out_error)
			, m_is_root_object(is_root_object)
		{}

		inline impl::PairReaderIterator begin() { return impl::PairReaderIterator(m_context, m_is_root_object, m_out_error); }
		inline impl::PairReaderIterator end() { return impl::PairReaderIterator(); }

	private:
		const impl::ReaderContext&	m_context;
		ReaderError*				m_out_error;
		bool						m_is_root_object;
	};

	class ValueReaderList
	{
	public:
		inline ValueReaderList(const impl::ReaderContext& context, ReaderError* out_error)
			: m_context(context)
			, m_out_error(out_error)
		{}

		inline impl::ValueReaderIterator begin() { return impl::ValueReaderIterator(m_context, m_out_error); }
		inline impl::ValueReaderIterator end() { return impl::ValueReaderIterator(); }

	private:
		const impl::ReaderContext&	m_context;
		ReaderError*				m_out_error;
	};

	// These convert SJSON text into built-in types
	inline bool from_sjson(ValueReader& sjson_value, bool default_value, ReaderError* out_error);
	inline StringView from_sjson(ValueReader& sjson_value, StringView default_value, ReaderError* out_error);
	inline double from_sjson(ValueReader& sjson_value, double default_value, ReaderError* out_error);
	inline float from_sjson(ValueReader& sjson_value, float default_value, ReaderError* out_error);
	inline int8_t from_sjson(ValueReader& sjson_value, int8_t default_value, ReaderError* out_error);
	inline uint8_t from_sjson(ValueReader& sjson_value, uint8_t default_value, ReaderError* out_error);
	inline int16_t from_sjson(ValueReader& sjson_value, int16_t default_value, ReaderError* out_error);
	inline uint16_t from_sjson(ValueReader& sjson_value, uint16_t default_value, ReaderError* out_error);
	inline int32_t from_sjson(ValueReader& sjson_value, int32_t default_value, ReaderError* out_error);
	inline uint32_t from_sjson(ValueReader& sjson_value, uint32_t default_value, ReaderError* out_error);
	inline int64_t from_sjson(ValueReader& sjson_value, int64_t default_value, ReaderError* out_error);
	inline uint64_t from_sjson(ValueReader& sjson_value, uint64_t default_value, ReaderError* out_error);

	class ValueReader
	{
	public:
		ValueReader() : m_context(impl::k_invalid_context) {}
		ValueReader(const impl::ReaderContext& context) : m_context(context) {}

		template<typename ElementType>
		ElementType read(ElementType default_value, ReaderError* out_error = nullptr)
		{
			return from_sjson(*this, default_value, out_error);
		}

		inline ValueReaderList get_values(ReaderError* out_error = nullptr)
		{
			if (m_context.is_eof())
			{
				if (out_error != nullptr)
					*out_error = ReaderError("Input truncated");

				return ValueReaderList(impl::k_invalid_context, out_error);
			}

			if (get_type() != ValueType::Array)
			{
				if (out_error != nullptr)
					*out_error = ReaderError("Expected an array");

				return ValueReaderList(impl::k_invalid_context, out_error);
			}

			return ValueReaderList(m_context, out_error);
		}

		inline size_t get_num_values(ReaderError* out_error = nullptr)
		{
			size_t count = 0;
			for (ValueReader value : get_values(out_error))
				count++;
			return count;
		}

		inline PairReaderList get_pairs(ReaderError* out_error = nullptr)
		{
			if (m_context.is_eof())
			{
				if (out_error != nullptr)
					*out_error = ReaderError("Input truncated");

				return PairReaderList(impl::k_invalid_context, false, out_error);
			}

			if (get_type() != ValueType::Object)
			{
				if (out_error != nullptr)
					*out_error = ReaderError("Expected an object");

				return PairReaderList(impl::k_invalid_context, false, out_error);
			}

			return PairReaderList(m_context, false, out_error);
		}

		inline size_t get_num_pairs(ReaderError* out_error = nullptr)
		{
			size_t count = 0;
			for (impl::PairReaderProxy pair : get_pairs(out_error))
				count++;
			return count;
		}

		ValueType get_type(ReaderError* out_error = nullptr) const
		{
			const char symbol = m_context.str[m_context.offset];

			if (symbol == 'n')
				return ValueType::Null;
			else if (symbol == 't')
				return ValueType::Bool;
			else if (symbol == 'f')
				return ValueType::Bool;
			else if (symbol == '\"')
				return ValueType::String;
			else if (symbol == '-' || std::isdigit(symbol))
				return ValueType::Number;
			else if (symbol == '[')
				return ValueType::Array;
			else if (symbol == '{')
				return ValueType::Object;

			if (out_error != nullptr)
				*out_error = ReaderError("Unknown value type");

			return ValueType::Unknown;
		}

	private:
		impl::ReaderContext m_context;

		friend impl::PairReaderIterator;

		// Built-in types are our friends
		friend bool from_sjson(ValueReader& sjson_value, bool default_value, ReaderError* out_error);
		friend StringView from_sjson(ValueReader& sjson_value, StringView default_value, ReaderError* out_error);
		friend double from_sjson(ValueReader& sjson_value, double default_value, ReaderError* out_error);
		friend float from_sjson(ValueReader& sjson_value, float default_value, ReaderError* out_error);
		friend int8_t from_sjson(ValueReader& sjson_value, int8_t default_value, ReaderError* out_error);
		friend uint8_t from_sjson(ValueReader& sjson_value, uint8_t default_value, ReaderError* out_error);
		friend int16_t from_sjson(ValueReader& sjson_value, int16_t default_value, ReaderError* out_error);
		friend uint16_t from_sjson(ValueReader& sjson_value, uint16_t default_value, ReaderError* out_error);
		friend int32_t from_sjson(ValueReader& sjson_value, int32_t default_value, ReaderError* out_error);
		friend uint32_t from_sjson(ValueReader& sjson_value, uint32_t default_value, ReaderError* out_error);
		friend int64_t from_sjson(ValueReader& sjson_value, int64_t default_value, ReaderError* out_error);
		friend uint64_t from_sjson(ValueReader& sjson_value, uint64_t default_value, ReaderError* out_error);
	};

	inline bool from_sjson(ValueReader& sjson_value, bool default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		bool value = default_value;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_bool(*value_context.parent, value);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_bool(context, value);
		}

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline StringView from_sjson(ValueReader& sjson_value, StringView default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView value = default_value;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_string(*value_context.parent, value);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_string(context, value);
		}

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline double from_sjson(ValueReader& sjson_value, double default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		double value = default_value;
		error = impl::number_to_double(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline float from_sjson(ValueReader& sjson_value, float default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		float value = default_value;
		error = impl::number_to_float(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline int8_t from_sjson(ValueReader& sjson_value, int8_t default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		int8_t value = default_value;
		error = impl::number_to_integer<int8_t>(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline uint8_t from_sjson(ValueReader& sjson_value, uint8_t default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		uint8_t value = default_value;
		error = impl::number_to_integer<uint8_t>(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline int16_t from_sjson(ValueReader& sjson_value, int16_t default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		int16_t value = default_value;
		error = impl::number_to_integer<int16_t>(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline uint16_t from_sjson(ValueReader& sjson_value, uint16_t default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		uint16_t value = default_value;
		error = impl::number_to_integer<uint16_t>(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline int32_t from_sjson(ValueReader& sjson_value, int32_t default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		int32_t value = default_value;
		error = impl::number_to_integer<int32_t>(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline uint32_t from_sjson(ValueReader& sjson_value, uint32_t default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		uint32_t value = default_value;
		error = impl::number_to_integer<uint32_t>(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline int64_t from_sjson(ValueReader& sjson_value, int64_t default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		int64_t value = default_value;
		error = impl::number_to_integer<int64_t>(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	inline uint64_t from_sjson(ValueReader& sjson_value, uint64_t default_value, ReaderError* out_error)
	{
		impl::ReaderContext& value_context = sjson_value.m_context;
		StringView number;
		int base;

		ReaderError error;
		if (value_context.parent != nullptr)
		{
			error = impl::read_number(*value_context.parent, number, base);
			value_context.parent = nullptr;
		}
		else
		{
			impl::ReaderContext context = value_context;
			error = impl::read_number(context, number, base);
		}

		if (error.any())
		{
			if (out_error != nullptr)
				*out_error = error;

			return default_value;
		}

		uint64_t value = default_value;
		error = impl::number_to_integer<uint64_t>(number, base, value);

		if (error.any() && out_error != nullptr)
			*out_error = error;

		return value;
	}

	struct PairReader
	{
		StringView		name;
		ValueReader		value;

		PairReader() {}
		PairReader(const impl::PairReaderProxy& proxy)
			: name(proxy.name)
			, value(proxy.context)
		{}
	};

	class Reader
	{
	public:
		inline Reader(const char* str, size_t str_length)
			: m_context{ StringView(str, str_length), 0, nullptr }
		{
			SJSON_CPP_ASSERT(str != nullptr, "Cannot read a null string");
			impl::skip_bom(m_context);
		}

		inline PairReaderList get_pairs(ReaderError* out_error = nullptr)
		{
			ReaderError error = impl::skip_comments_and_whitespace(m_context);
			if (error.any())
			{
				if (out_error != nullptr)
					*out_error = error;

				return PairReaderList(impl::k_invalid_context, true, out_error);
			}

			if (m_context.is_eof())
				return PairReaderList(impl::k_invalid_context, true, out_error);

			return PairReaderList(m_context, true, out_error);
		}

		inline size_t get_num_pairs(ReaderError* out_error = nullptr)
		{
			size_t count = 0;
			for (PairReader pair : get_pairs(out_error))
				count++;
			return count;
		}

	private:
		impl::ReaderContext		m_context;
	};

	//////////////////////////////////////////////////////////////////////////

	namespace impl
	{
		ReaderError skip_value(ReaderContext& context, const ReaderContext& value_context)
		{
			ValueReader value_to_skip(value_context);

			ReaderError error;
			switch (value_to_skip.get_type(&error))
			{
			case ValueType::Unknown:
			default:
				break;
			case ValueType::Null:
				error = impl::read_null(context);
				break;
			case ValueType::Bool:
			{
				bool value;
				error = impl::read_bool(context, value);
				break;
			}
			case ValueType::String:
			{
				StringView value;
				error = impl::read_string(context, value);
				break;
			}
			case ValueType::Number:
			{
				StringView value;
				int base;
				error = impl::read_number(context, value, base);
				break;
			}
			case ValueType::Array:
			{
				ValueReaderList value_list = value_to_skip.get_values(&error);
				if (error.any())
					return error;

				auto it = value_list.begin();
				if (error.any())
					return error;

				auto end_it = value_list.end();
				for (; it != end_it; ++it)
				{
					if (error.any())
						return error;
				}

				if (error.any())
					return error;

				context.offset = it.m_value_context.offset;
				break;
			}
			case ValueType::Object:
			{
				PairReaderList pair_list = value_to_skip.get_pairs(&error);
				if (error.any())
					return error;

				auto it = pair_list.begin();
				if (error.any())
					return error;

				auto end_it = pair_list.end();
				for (; it != end_it; ++it)
				{
					if (error.any())
						return error;
				}

				if (error.any())
					return error;

				context.offset = it.m_pair_context.offset;
				break;
			}
			}

			return error;
		}
	}
}
