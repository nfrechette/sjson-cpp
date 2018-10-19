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
		SJSON_BIND_VAR("tmp0", f.tmp0, StringView, "");
		SJSON_BIND_VAR("tmp1", f.tmp1, int32, 0);
		SJSON_BIND_VAR("tmp2", f.tmp2, bool, false);
	SJSON_BIND_END();
*/

#define SJSON_BIND_BEGIN(source_, error_) \
	for (sjson::PairReader _pair : source_.get_pairs(error_)) \
	{ \
		sjson::ReaderError _tmp_error; \
		sjson::ReaderError* _bind_error = error_ != nullptr ? error_ : &_tmp_error; \
		if (_bind_error->any()) \
			break

#define SJSON_BIND_VAR(key_name_, variable_, type_, default_value_) \
		else if (_pair.name == key_name_) \
			variable_ = _pair.value.read<type_>(default_value_, _bind_error)

#define SJSON_BIND_END() \
	} \
	do {} while (false)
