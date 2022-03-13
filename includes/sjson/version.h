#pragma once

////////////////////////////////////////////////////////////////////////////////
// The MIT License (MIT)
//
// Copyright (c) 2022 Nicholas Frechette, Cody Jones, and sjson-cpp contributors
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

////////////////////////////////////////////////////////////////////////////////
// Macros to detect the sjson-cpp version
////////////////////////////////////////////////////////////////////////////////

#define SJSON_CPP_VERSION_MAJOR 0
#define SJSON_CPP_VERSION_MINOR 8
#define SJSON_CPP_VERSION_PATCH 1

////////////////////////////////////////////////////////////////////////////////
// In order to allow multiple versions of this library to coexist side by side
// within the same executable/library, the symbols have to be unique per version.
// We achieve this by using a versioned namespace that we optionally inline.
// To disable namespace inlining, define SJSON_CPP_NO_INLINE_NAMESPACE before including
// any sjson-cpp header.
////////////////////////////////////////////////////////////////////////////////

// Name of the namespace, e.g. v08
#define SJSON_CPP_IMPL_VERSION_NAMESPACE_NAME v ## SJSON_CPP_VERSION_MAJOR ## SJSON_CPP_VERSION_MINOR

#if defined(SJSON_CPP_NO_INLINE_NAMESPACE)
    // Namespace won't be inlined, its usage will have to be qualified with the
    // full version everywhere
    #define SJSON_CPP_IMPL_NAMESPACE sjson::SJSON_CPP_IMPL_VERSION_NAMESPACE_NAME

    #define SJSON_CPP_IMPL_VERSION_NAMESPACE_BEGIN \
        namespace SJSON_CPP_IMPL_VERSION_NAMESPACE_NAME \
        {
#else
    // Namespace is inlined, its usage does not need to be qualified with the
    // full version everywhere
    #define SJSON_CPP_IMPL_NAMESPACE sjson

    #define SJSON_CPP_IMPL_VERSION_NAMESPACE_BEGIN \
        inline namespace SJSON_CPP_IMPL_VERSION_NAMESPACE_NAME \
        {
#endif

#define SJSON_CPP_IMPL_VERSION_NAMESPACE_END \
    }
