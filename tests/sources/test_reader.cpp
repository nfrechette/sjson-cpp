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

		int pair_count = 0;
		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Array);
			REQUIRE(error.empty());

			int value_count = 0;
			for (ValueReader value : pair.value.get_values(&error))
			{
				double value1 = value.read<double>(0.0, &error);
				switch (value_count)
				{
				case 0:
					REQUIRE(value1 == 123.456789);
					break;
				case 1:
					REQUIRE(value1 == 456.789);
					break;
				case 2:
					REQUIRE(value1 == 151.091);
					break;
				}
				value_count++;
			}

			REQUIRE(error.empty());
			REQUIRE(value_count == 3);
			pair_count++;
		}
		REQUIRE(error.empty());
		REQUIRE(pair_count == 1);
	}

	{
		Reader reader = reader_from_c_str("key = [ 123.456789, 456.789, 151.091 ]");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		int pair_count = 0;
		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Array);
			REQUIRE(error.empty());

			int value_count = 0;
			for (ValueReader value : pair.value.get_values(&error))
			{
				double value1 = value.read<double>(0.0, &error);
				REQUIRE(value1 == 123.456789);
				value_count++;

				// Break in the middle of the iteration
				if (value_count == 1)
					break;
			}

			REQUIRE(error.empty());
			REQUIRE(value_count == 1);
			pair_count++;
		}
		REQUIRE(error.empty());
		REQUIRE(pair_count == 1);
	}
}

TEST_CASE("Reader Object Reading", "[reader]")
{
	{
		Reader reader = reader_from_c_str("root_key = { key0 = false, key1 = 123.0, key2 = \"ok\" }, tmp = true");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 2);

		int pair_count = 0;
		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			if (pair_count == 0)
			{
				REQUIRE(pair.name == "root_key");
				REQUIRE(pair.value.get_type(&error) == ValueType::Object);
				REQUIRE(error.empty());

				int pair1_count = 0;
				for (PairReader pair1 : pair.value.get_pairs(&error))
				{
					switch (pair1_count)
					{
					case 0:
						REQUIRE(pair1.name == "key0");
						REQUIRE(pair1.value.read<bool>(true, &error) == false);
						break;
					case 1:
						REQUIRE(pair1.name == "key1");
						REQUIRE(pair1.value.read<float>(0.0f, &error) == 123.0);
						break;
					case 2:
						REQUIRE(pair1.name == "key2");
						REQUIRE(pair1.value.read<StringView>("", &error) == "ok");
						break;
					}

					REQUIRE(error.empty());
					pair1_count++;
				}

				REQUIRE(error.empty());
				REQUIRE(pair1_count == 3);
			}
			else if (pair_count == 1)
			{
				REQUIRE(pair.name == "tmp");
				REQUIRE(pair.value.read<bool>(false, &error));
			}

			pair_count++;
		}
		REQUIRE(error.empty());
		REQUIRE(pair_count == 2);
	}

	{
		Reader reader = reader_from_c_str("root_key = { key0 = false, key1 = 123.0, key2 = \"ok\" }, tmp = true");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 2);

		int pair_count = 0;
		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			if (pair_count == 0)
			{
				REQUIRE(pair.name == "root_key");
				REQUIRE(pair.value.get_type(&error) == ValueType::Object);
				REQUIRE(error.empty());

				int pair1_count = 0;
				for (PairReader pair1 : pair.value.get_pairs(&error))
				{
					REQUIRE(pair1.name == "key0");
					REQUIRE(pair1.value.read<bool>(true, &error) == false);
					pair1_count++;

					// Break in the middle of the iteration
					if (pair1_count == 1)
						break;
				}

				REQUIRE(error.empty());
				REQUIRE(pair1_count == 1);
			}
			else if (pair_count == 1)
			{
				REQUIRE(pair.name == "tmp");
				REQUIRE(pair.value.read<bool>(false, &error));
			}

			pair_count++;
		}
		REQUIRE(error.empty());
		REQUIRE(pair_count == 2);
	}
}

TEST_CASE("Reader Mixed Reading", "[reader]")
{
	{
		Reader reader = reader_from_c_str("key = [ 123.456789, false, [ 1.0, true, { tmp0 = false, tmp1 = 2.0 } ], \"456.789\", [ 1.0, false, [], {} ] ]");
		ReaderError error;
		size_t num_pairs = reader.get_num_pairs(&error);
		REQUIRE(error.empty());
		REQUIRE(num_pairs == 1);

		int pair_count = 0;
		for (PairReader pair : reader.get_pairs(&error))
		{
			REQUIRE(error.empty());
			REQUIRE(pair.name == "key");
			REQUIRE(pair.value.get_type(&error) == ValueType::Array);
			REQUIRE(error.empty());

			int value_count0 = 0;
			for (ValueReader value : pair.value.get_values(&error))
			{
				switch (value_count0)
				{
				case 0:
				{
					REQUIRE(value.read<double>(0.0, &error) == 123.456789);
					REQUIRE(error.empty());
					break;
				}
				case 1:
				{
					REQUIRE(value.read<bool>(true, &error) == false);
					REQUIRE(error.empty());
					break;
				}
				case 2:
				{
					REQUIRE(value.get_num_values(&error) == 3);
					REQUIRE(error.empty());

					int value_count1 = 0;
					for (ValueReader value3 : value.get_values(&error))
					{
						switch (value_count1)
						{
						case 0:
						{
							REQUIRE(value3.read<double>(0.0, &error) == 1.0);
							REQUIRE(error.empty());
							break;
						}
						case 1:
						{
							REQUIRE(value3.read<bool>(false, &error) == true);
							REQUIRE(error.empty());
							break;
						}
						case 2:
						{
							REQUIRE(value3.get_type(&error) == ValueType::Object);
							REQUIRE(error.empty());
							REQUIRE(value3.get_num_pairs(&error) == 2);
							REQUIRE(error.empty());

							int pair1_count = 0;
							for (PairReader pair1 : value3.get_pairs(&error))
							{
								REQUIRE(error.empty());
								switch (pair1_count)
								{
								case 0:
									REQUIRE(pair1.name == "tmp0");
									REQUIRE(pair1.value.read<bool>(true, &error) == false);
									REQUIRE(error.empty());
									break;
								case 1:
									REQUIRE(pair1.name == "tmp1");
									REQUIRE(pair1.value.read<double>(0.0, &error) == 2.0);
									REQUIRE(error.empty());
									break;
								}
								pair1_count++;
							}

							REQUIRE(error.empty());
							REQUIRE(pair1_count == 2);
							break;
						}
						}

						value_count1++;
					}

					REQUIRE(error.empty());
					REQUIRE(value_count1 == 3);
					break;
				}
				case 3:
				{
					StringView value6 = value.read<StringView>("", &error);
					REQUIRE(error.empty());
					REQUIRE(value6 == "456.789");
					break;
				}
				case 4:
				{
					REQUIRE(value.get_num_values(&error) == 4);
					REQUIRE(error.empty());

					int value_count2 = 0;
					for (ValueReader value7 : value.get_values(&error))
					{
						switch (value_count2)
						{
						case 0:
						{
							double value8 = value7.read<double>(0.0, &error);
							REQUIRE(error.empty());
							REQUIRE(value8 == 1.0);
							break;
						}
						case 1:
						{
							bool value9 = value7.read<bool>(true, &error);
							REQUIRE(error.empty());
							REQUIRE(value9 == false);
							break;
						}
						case 2:
						{
							REQUIRE(value7.get_type(&error) == ValueType::Array);
							REQUIRE(error.empty());
							REQUIRE(value7.get_num_values(&error) == 0);
							REQUIRE(error.empty());
							int value_count3 = 0;
							for (ValueReader value10 : value7.get_values(&error))
							{
								(void)value10;
								value_count3++;
							}
							REQUIRE(error.empty());
							REQUIRE(value_count3 == 0);
							break;
						}
						case 3:
						{
							REQUIRE(value7.get_type(&error) == ValueType::Object);
							REQUIRE(error.empty());
							REQUIRE(value7.get_num_pairs(&error) == 0);
							REQUIRE(error.empty());
							int pair_count2 = 0;
							for (PairReader pair2 : value7.get_pairs(&error))
							{
								(void)pair2;
								pair_count2++;
							}
							REQUIRE(error.empty());
							REQUIRE(pair_count2 == 0);
							break;
						}
						}

						value_count2++;
					}

					REQUIRE(error.empty());
					REQUIRE(value_count2 == 4);
					break;
				}
				}

				value_count0++;
			}

			REQUIRE(error.empty());
			REQUIRE(value_count0 == 5);
			pair_count++;
		}
		REQUIRE(error.empty());
		REQUIRE(pair_count == 1);
	}
}
