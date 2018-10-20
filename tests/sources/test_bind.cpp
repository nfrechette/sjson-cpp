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

#include <catch.hpp>

#include <sjson/bind.h>
#include <sjson/reader.h>

static sjson::Reader reader_from_c_str(const char* c_str)
{
	return sjson::Reader(c_str, std::strlen(c_str));
}

TEST_CASE("Bind Macros", "[bind]")
{
	{
		sjson::StringView var0 = "???";
		bool var1 = false;
		int32_t var2 = 123;
		float var3 = 456.0f;
		int8_t var4[4] = { 1, 1, 1, 1 };
		std::vector<int16_t> var5;

		sjson::Reader reader = reader_from_c_str("var0 = \"ok\", var1 = true, var2 = 31, var3 = 42.0, var4 = [ 0, 2, 3, 4 ], var5 = [ 0, 1, 2, 3, 4 ]");
		sjson::ReaderError error;

		SJSON_BIND_BEGIN(reader, &error);
			SJSON_BIND_STR("var0", var0, "???");
			SJSON_BIND_VAR("var1", var1, false);
			SJSON_BIND_VAR("var2", var2, 123);
			SJSON_BIND_VAR("var3", var3, 456.0f);
			SJSON_BIND_ARR("var4", var4, 4, 1);
			SJSON_BIND_VEC("var5", var5, 9);
		SJSON_BIND_END();

		REQUIRE(error.empty());
		REQUIRE(var0 == "ok");
		REQUIRE(var1 == true);
		REQUIRE(var2 == 31);
		REQUIRE(var3 == 42.0f);
		REQUIRE(var4[0] == 0);
		REQUIRE(var4[1] == 2);
		REQUIRE(var4[2] == 3);
		REQUIRE(var4[3] == 4);
		REQUIRE(var5[0] == 0);
		REQUIRE(var5[1] == 1);
		REQUIRE(var5[2] == 2);
		REQUIRE(var5[3] == 3);
		REQUIRE(var5[4] == 4);
	}

	{
		sjson::StringView var0 = "???";
		bool var1 = false;
		int32_t var2 = 123;
		float var3 = 456.0f;
		int8_t var4[4] = { 1, 1, 1, 1 };
		std::vector<int16_t> var5;

		sjson::Reader reader = reader_from_c_str("root = { var0 = \"ok\", var1 = true, var2 = 31, var3 = 42.0, var4 = [ 0, 2, 3, 4 ], var5 = [ 0, 1, 2, 3, 4 ] }");
		sjson::ReaderError error;

		REQUIRE(reader.get_num_pairs(&error) == 1);
		REQUIRE(error.empty());

		for (sjson::PairReader pair : reader.get_pairs())
		{
			REQUIRE(pair.name == "root");

			SJSON_BIND_BEGIN(pair.value, &error);
				SJSON_BIND_STR("var0", var0, "???");
				SJSON_BIND_VAR("var1", var1, false);
				SJSON_BIND_VAR("var2", var2, 123);
				SJSON_BIND_VAR("var3", var3, 456.0f);
				SJSON_BIND_ARR("var4", var4, 4, 1);
				SJSON_BIND_VEC("var5", var5, 9);
			SJSON_BIND_END();

			REQUIRE(error.empty());
			REQUIRE(var0 == "ok");
			REQUIRE(var1 == true);
			REQUIRE(var2 == 31);
			REQUIRE(var3 == 42.0f);
			REQUIRE(var4[0] == 0);
			REQUIRE(var4[1] == 2);
			REQUIRE(var4[2] == 3);
			REQUIRE(var4[3] == 4);
			REQUIRE(var5[0] == 0);
			REQUIRE(var5[1] == 1);
			REQUIRE(var5[2] == 2);
			REQUIRE(var5[3] == 3);
			REQUIRE(var5[4] == 4);
		}

		REQUIRE(error.empty());
	}
}
