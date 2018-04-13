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

#include <sjson/reader.h>

using namespace sjson;

static Reader reader_from_c_str(const char* c_str)
{
	return Reader(c_str, std::strlen(c_str));
}

TEST_CASE("Reader Misc", "[reader]")
{
	{
		Reader reader = reader_from_c_str("");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(num_pairs == 0);
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("     ");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(num_pairs == 0);
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("// lol \\n     ");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(num_pairs == 0);
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("\"key-one\" = true");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key-one");
			bool value = pair.value.read<bool>(false, &error);
			REQUIRE(error.empty());
			REQUIRE(value == true);
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = /* bar */ true");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			bool value = pair.value.read<bool>(false, &error);
			REQUIRE(error.empty());
			REQUIRE(value == true);
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = /* bar * true");
		ReaderError error;
		reader.get_num_pairs(&error);
		REQUIRE(error.any());
	}

	{
		Reader reader = reader_from_c_str("key = // bar \ntrue");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			bool value = pair.value.read<bool>(false, &error);
			REQUIRE(error.empty());
			REQUIRE(value == true);
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key /* bar */ = true");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			bool value = pair.value.read<bool>(false, &error);
			REQUIRE(error.empty());
			REQUIRE(value == true);
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("/* bar */ key = true");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			bool value = pair.value.read<bool>(false, &error);
			REQUIRE(error.empty());
			REQUIRE(value == true);
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = null");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Null);
			REQUIRE(error.empty());
			bool value = pair.value.read<bool>(false, &error);
			REQUIRE(error.any());
			REQUIRE(value == false);
			error.reset();
		}
		REQUIRE(error.empty());
	}
}

TEST_CASE("Reader Bool Reading", "[reader]")
{
	{
		Reader reader = reader_from_c_str("key = true");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Bool);
			REQUIRE(error.empty());
			bool value = pair.value.read<bool>(false, &error);
			REQUIRE(error.empty());
			REQUIRE(value == true);

			bool value1 = pair.value.read<bool>(false, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == true);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = false");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Bool);
			REQUIRE(error.empty());
			bool value = pair.value.read<bool>(true, &error);
			REQUIRE(error.empty());
			REQUIRE(value == false);
		}
		REQUIRE(error.empty());
	}
}

TEST_CASE("Reader String Reading", "[reader]")
{
	{
		Reader reader = reader_from_c_str("key = \"Quoted string\"");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			StringView value = pair.value.read<StringView>("", &error);
			REQUIRE(error.empty());
			REQUIRE(value == "Quoted string");

			StringView value1 = pair.value.read<StringView>("", &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == "Quoted string");

			bool value2 = pair.value.read<bool>(true, &error);
			REQUIRE(error.any());
			REQUIRE(value2 == true);
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		// Note: Escaped quotes \" are left escaped within the StringView because we do not allocate memory
		Reader reader = reader_from_c_str("key = \"Quoted \\\" string\"");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			StringView value = pair.value.read<StringView>("", &error);
			REQUIRE(error.empty());
			REQUIRE(value == "Quoted \\\" string");
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = \"New\\nline\"");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			StringView value = pair.value.read<StringView>("", &error);
			REQUIRE(error.empty());
			REQUIRE(value == "New\\nline");
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = \"Tab\\tulator\"");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			StringView value = pair.value.read<StringView>("", &error);
			REQUIRE(error.empty());
			REQUIRE(value == "Tab\\tulator");
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = \"Tab\\tulator\"");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			StringView value = pair.value.read<StringView>("", &error);
			REQUIRE(error.empty());
			REQUIRE(value == "Tab\\tulator");
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = \"bad");
		ReaderError error;
		reader.get_num_pairs(&error);
		REQUIRE(error.any());
	}

	{
		Reader reader = reader_from_c_str("key = bad");
		ReaderError error;
		reader.get_num_pairs(&error);
		REQUIRE(error.any());
	}
}

TEST_CASE("Reader Number Reading", "[reader]")
{
	{
		Reader reader = reader_from_c_str("key = 123.456789");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			double value = pair.value.read<double>(0.0, &error);
			REQUIRE(error.empty());
			REQUIRE(value == 123.456789);

			double value1 = pair.value.read<double>(0.0, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == 123.456789);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = 123.456789");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			float value = pair.value.read<float>(0.0f, &error);
			REQUIRE(error.empty());
			REQUIRE(value == 123.456789f);

			float value1 = pair.value.read<float>(0.0f, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == 123.456789f);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = -123");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			int8_t value = pair.value.read<int8_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value == -123);

			int8_t value1 = pair.value.read<int8_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == -123);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = 123");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			uint8_t value = pair.value.read<uint8_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value == 123);

			uint8_t value1 = pair.value.read<uint8_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == 123);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = -1234");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			int16_t value = pair.value.read<int16_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value == -1234);

			int16_t value1 = pair.value.read<int16_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == -1234);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = 1234");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			uint16_t value = pair.value.read<uint16_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value == 1234);

			uint16_t value1 = pair.value.read<uint16_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == 1234);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = -123456");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			int32_t value = pair.value.read<int32_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value == -123456);

			int32_t value1 = pair.value.read<int32_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == -123456);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = 123456");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			uint32_t value = pair.value.read<uint32_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value == 123456);

			uint32_t value1 = pair.value.read<uint32_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == 123456);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = -1234567890123456");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			int64_t value = pair.value.read<int64_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value == -1234567890123456);

			int64_t value1 = pair.value.read<int64_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == -1234567890123456);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	{
		Reader reader = reader_from_c_str("key = 1234567890123456");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Number);
			REQUIRE(error.empty());
			uint64_t value = pair.value.read<uint64_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value == 1234567890123456);

			uint64_t value1 = pair.value.read<uint64_t>(0, &error);
			REQUIRE(error.empty());
			REQUIRE(value1 == 1234567890123456);

			StringView value2 = pair.value.read<StringView>("bad", &error);
			REQUIRE(error.any());
			REQUIRE(value2 == "bad");
			error.reset();
		}
		REQUIRE(error.empty());
	}

	// TODO: Unit test: hexa, decimal to integer truncation, decimal hexa
}

TEST_CASE("Reader Array Reading", "[reader]")
{
	{
		Reader reader = reader_from_c_str("key = [ 123.456789, 456.789, 151.091 ]");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			//REQUIRE(pair.value.get_type(&error) == ValueType::Bool);
			REQUIRE(error.empty());
			//bool value = pair.value.read<bool>(false, &error);
			//REQUIRE(error.empty());
			//REQUIRE(value == true);
		}
		//double value[3] = { 0.0, 0.0, 0.0 };
		//REQUIRE(parser.read("key", value, 3));
		//REQUIRE(value[0] == 123.456789);
		//REQUIRE(value[1] == 456.789);
		//REQUIRE(value[2] == 151.091);
		//REQUIRE(parser.eof());
		//REQUIRE(parser.is_valid());
	}

#if 0
	{
		Parser parser = parser_from_c_str("key = [ 123.456789, 456.789, 151.091 ]");
		double value[3] = { 0.0, 0.0, 0.0 };
		REQUIRE(parser.read("key", value, 3));
		REQUIRE(value[0] == 123.456789);
		REQUIRE(value[1] == 456.789);
		REQUIRE(value[2] == 151.091);
		REQUIRE(parser.eof());
		REQUIRE(parser.is_valid());
	}

	{
		Parser parser = parser_from_c_str("key = [ \"123.456789\", \"456.789\", \"151.091\" ]");
		StringView value[3];
		REQUIRE(parser.read("key", value, 3));
		REQUIRE(value[0] == "123.456789");
		REQUIRE(value[1] == "456.789");
		REQUIRE(value[2] == "151.091");
		REQUIRE(parser.eof());
		REQUIRE(parser.is_valid());
	}

	{
		Parser parser = parser_from_c_str("bad_key = \"bad\"");
		double value[3] = { 0.0, 0.0, 0.0 };
		REQUIRE_FALSE(parser.try_read("key", value, 3, 1.0));
		REQUIRE(value[0] == 1.0);
		REQUIRE(value[1] == 1.0);
		REQUIRE(value[2] == 1.0);
		REQUIRE_FALSE(parser.eof());
		REQUIRE(parser.is_valid());
	}

	{
		Parser parser = parser_from_c_str("key = [ 123.456789, 456.789, 151.091 ]");
		double value[3] = { 0.0, 0.0, 0.0 };
		REQUIRE(parser.try_read("key", value, 3, 1.0));
		REQUIRE(value[0] == 123.456789);
		REQUIRE(value[1] == 456.789);
		REQUIRE(value[2] == 151.091);
		REQUIRE(parser.eof());
		REQUIRE(parser.is_valid());
	}

	{
		Parser parser = parser_from_c_str("bad_key = \"bad\"");
		StringView value[3];
		REQUIRE_FALSE(parser.try_read("key", value, 3, "default"));
		REQUIRE(value[0] == "default");
		REQUIRE(value[1] == "default");
		REQUIRE(value[2] == "default");
		REQUIRE_FALSE(parser.eof());
		REQUIRE(parser.is_valid());
	}

	{
		Parser parser = parser_from_c_str("key = [ \"123.456789\", \"456.789\", \"151.091\" ]");
		StringView value[3];
		REQUIRE(parser.try_read("key", value, 3, "default"));
		REQUIRE(value[0] == "123.456789");
		REQUIRE(value[1] == "456.789");
		REQUIRE(value[2] == "151.091");
		REQUIRE(parser.eof());
		REQUIRE(parser.is_valid());
	}
#endif

#if 0
	{
		Parser parser = parser_from_c_str("key = [ 123.456789, \"456.789\", false, [ 1.0, true ], { key0 = 1.0, key1 = false } ]");

		REQUIRE(parser.array_begins("key"));
		// TODO
		REQUIRE(parser.array_ends());
		REQUIRE(parser.eof());
		REQUIRE(parser.is_valid());
	}
#endif
}
