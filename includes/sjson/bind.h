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

#include "sjson/reader.h"

#include <type_traits>
#include <string.h>
#include <vector>

/*
	struct foo
	{
		StringView tmp0;
		int32 tmp1;
		bool tmp2;
	};

	// It would be nice to use something like this but we either have to pay a setup cost every time or we require dynamic allocation
	foo f;
	Binder binder(&error);
	binder.bind("tmp0", f.tmp0);
	binder.bind("tmp1", f.tmp1);
	binder.bind("tmp2", f.tmp2);
	binder.apply(reader);
	binder.apply(obj_value);

	// Macros work well and are clean
	SJSON_BIND_BEGIN(reader, &error);
		SJSON_BIND_VAR("tmp0", f.tmp0);
		SJSON_BIND_VAR("tmp1", f.tmp1);
		SJSON_BIND_VAR("tmp2", f.tmp2);
	SJSON_BIND_END();
*/

#define SJSON_BIND_BEGIN(source_, error_) \
	for (sjson::PairReader _pair : source_.get_pairs(error_)) \
	{ \
		sjson::ReaderError _tmp_error; \
		sjson::ReaderError* _bind_error = error_ != nullptr ? error_ : &_tmp_error; \
		if (_bind_error->any()) \
			break

#define SJSON_BIND_VAR(key_name_, variable_) \
		else if (_pair.name == key_name_) \
			variable_ = _pair.value.read(variable_, _bind_error)

#define SJSON_BIND_STR(key_name_, variable_) \
		else if (_pair.name == key_name_) \
			sjson::impl::bound_string_read(_pair.value, variable_, *_bind_error)

#define SJSON_BIND_ARR(key_name_, array_, num_elements_) \
		else if (_pair.name == key_name_) \
			sjson::impl::bound_array_read<std::remove_all_extents<decltype(array_)>::type>(_pair.value, array_, num_elements_, *_bind_error)

#define SJSON_BIND_VEC(key_name_, vector_) \
		else if (_pair.name == key_name_) \
			sjson::impl::bound_vector_read<sjson::vector_element_type<decltype(vector_)>::type>(_pair.value, vector_, *_bind_error)

#define SJSON_BIND_END() \
	} \
	do {} while (false)

namespace sjson
{
	template<typename VectorType>
	struct vector_element_type
	{};

	template<typename ElementType>
	struct vector_element_type<std::vector<ElementType>>
	{
		using type = ElementType;
	};

	template<typename ElementType>
	inline void vector_push(std::vector<ElementType>& vec, ElementType value)
	{
		vec.push_back(value);
	}

	inline void string_copy(std::string& str, const StringView& value)
	{
		str = std::string(value.c_str(), value.size());
	}

	namespace impl
	{
		template<typename StringType>
		inline void bound_string_read(ValueReader& sjson_value, StringType& out_data, ReaderError& out_error)
		{
			StringView value = sjson_value.read<StringView>("", &out_error);
			sjson::string_copy(out_data, value);
		}

		template<>
		inline void bound_string_read<StringView>(ValueReader& sjson_value, StringView& out_data, ReaderError& out_error)
		{
			out_data = sjson_value.read(out_data, &out_error);
		}

		template<typename ElementType>
		inline void bound_array_read(ValueReader& sjson_value, ElementType* out_data, size_t num_elements, ReaderError& out_error)
		{
			size_t num_read_elements = 0;
			for (ValueReader element_value : sjson_value.get_values(&out_error))
			{
				if (num_read_elements >= num_elements)
				{
					out_error = ReaderError("Expected fewer elements when reading array");
					break;
				}

				out_data[num_read_elements] = element_value.read(out_data[num_read_elements], &out_error);
				num_read_elements++;

				if (out_error.any())
					break;
			}
		}

		template<typename ElementType, typename VectorType>
		inline void bound_vector_read(ValueReader& sjson_value, VectorType& out_data, ReaderError& out_error)
		{
			ElementType default_value = ElementType();
			for (ValueReader element_value : sjson_value.get_values(&out_error))
			{
				sjson::vector_push<ElementType>(out_data, element_value.read(default_value, &out_error));

				if (out_error.any())
					break;
			}
		}
	}
}
