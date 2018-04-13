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

// NVIDIA CodeWorks used for Android has an old version of Clang that does not fully support C++11
#if defined(__clang__) && __clang_major__ <= 3 && __clang_minor__ <= 8
	#include <stdlib.h>
#else
	#include <cstdlib>
#endif

namespace sjson
{
	namespace impl
	{
		inline unsigned long long int strtoull(const char* str, char** endptr, int base)
		{
#if defined(__clang__) && __clang_major__ <= 3 && __clang_minor__ <= 8
			return ::strtoull(str, endptr, base);
#else
			return std::strtoull(str, endptr, base);
#endif
		}

		inline long long int strtoll(const char* str, char** endptr, int base)
		{
#if defined(__clang__) && __clang_major__ <= 3 && __clang_minor__ <= 8
			return ::strtoll(str, endptr, base);
#else
			return std::strtoll(str, endptr, base);
#endif
		}

		inline float strtof(const char* str, char** endptr)
		{
#if defined(__clang__) && __clang_major__ <= 3 && __clang_minor__ <= 8
			return ::strtof(str, endptr);
#else
			return std::strtof(str, endptr);
#endif
		}
	}
}
