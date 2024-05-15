#include <gtest/gtest.h>

#include "../../src/utils/helpers.h"

#include <sstream>

TEST(SplitTests, EmptyDelimiter)
{
    std::string delimiter = "";
    std::vector<std::string_view> splits{};
    std::string s_no_delimiter{ "teststringforsplittingwithadelimiter" };

    split(s_no_delimiter, delimiter, splits);

    EXPECT_EQ(splits.size(), 0);
}

TEST(SplitTests, SingleCharDelimiter)
{
    std::string delimiter = ",";

    std::string s_no_delimiter{ "teststringforsplittingwithadelimiter" };
    std::string s_single_delimiter{ "teststringforsplitting,withadelimiter" };
    std::string s_with_delimiter{ "test,string,for,splitting,with,a,delimiter" };
    std::string s_delimiter_in_begin{ ",teststringforsplittingwithadelimiter" };
    std::string s_delimiter_in_end{ "teststringforsplittingwithadelimiter," };
    std::string s_only_delimiter{ ",,," };
    std::string s_empty{ "" };
    std::string s_sparse{ "sparse,,,,text," };

    std::vector<std::string_view> splits_1{};
    split(s_no_delimiter, delimiter, splits_1);
    EXPECT_EQ(splits_1.size(), 1);
    EXPECT_EQ(splits_1[0], s_no_delimiter);

    std::vector<std::string_view> splits_2{};
    split(s_single_delimiter, delimiter, splits_2);
    EXPECT_EQ(splits_2.size(), 2);
    EXPECT_EQ(splits_2[0], "teststringforsplitting");
    EXPECT_EQ(splits_2[1], "withadelimiter");

    std::vector<std::string_view> splits_3{};
    split(s_with_delimiter, delimiter, splits_3);
    EXPECT_EQ(splits_3.size(), 7);
    EXPECT_EQ(splits_3[0], "test");
    EXPECT_EQ(splits_3[1], "string");
    EXPECT_EQ(splits_3[2], "for");
    EXPECT_EQ(splits_3[3], "splitting");
    EXPECT_EQ(splits_3[4], "with");
    EXPECT_EQ(splits_3[5], "a");
    EXPECT_EQ(splits_3[6], "delimiter");

    std::vector<std::string_view> splits_4{};
    split(s_delimiter_in_begin, delimiter, splits_4);
    EXPECT_EQ(splits_4.size(), 2);
    EXPECT_EQ(splits_4[0], "");
    EXPECT_EQ(splits_4[1], "teststringforsplittingwithadelimiter");

    std::vector<std::string_view> splits_5{};
    split(s_delimiter_in_end, delimiter, splits_5);
    EXPECT_EQ(splits_5.size(), 2);
    EXPECT_EQ(splits_5[0], "teststringforsplittingwithadelimiter");
    EXPECT_EQ(splits_5[1], "");

    std::vector<std::string_view> splits_6{};
    split(s_only_delimiter, delimiter, splits_6);
    EXPECT_EQ(splits_6.size(), 4);
    for (int i = 0; i < splits_6.size(); i++) EXPECT_EQ(splits_6[i], "");

    std::vector<std::string_view> splits_7{};
    split(s_empty, delimiter, splits_7);
    EXPECT_EQ(splits_7.size(), 0);

    std::vector<std::string_view> splits_8{};
    split(s_sparse, delimiter, splits_8);
    EXPECT_EQ(splits_8.size(), 6);
    for (int i = 0; i < splits_8.size(); i++)
    {
        if (i == 0) EXPECT_EQ(splits_8[i], "sparse");
        else if (i == 4) EXPECT_EQ(splits_8[i], "text");
        else EXPECT_EQ(splits_8[i], "");
    }
}

TEST(SplitTests, MultiCharDelimiter)
{
    std::string delimiter = "!?!";

    std::string only_delimiter = "!?!";
    std::string no_delimiter = "some text";
    std::string delimiter_in_end = "some text!?!";
    std::string delimiter_in_begin = "!?!some text";
    std::string delimiters_in_middle = "some text!?!again some text!?!something_useless";
    std::string s_only_delimiter{ "!?!!?!!?!" };
    std::string s_empty{ "" };
    std::string s_sparse{ "sparse!?!!?!!?!!?!text!?!" };
    std::string s_with_delimiter_char{ "text!?!including!?!delimiter!char!?!in!?it" };

    std::vector<std::string_view> splits_1{};
    split(only_delimiter, delimiter, splits_1);
    EXPECT_EQ(splits_1.size(), 2);
    EXPECT_EQ(splits_1[0], "");
    EXPECT_EQ(splits_1[1], "");

    std::vector<std::string_view> splits_2{};
    split(no_delimiter, delimiter, splits_2);
    EXPECT_EQ(splits_2.size(), 1);
    EXPECT_EQ(splits_2[0], "some text");

    std::vector<std::string_view> splits_3{};
    split(delimiter_in_end, delimiter, splits_3);
    EXPECT_EQ(splits_3.size(), 2);
    EXPECT_EQ(splits_3[0], "some text");
    EXPECT_EQ(splits_3[1], "");

    std::vector<std::string_view> splits_4{};
    split(delimiter_in_begin, delimiter, splits_4);
    EXPECT_EQ(splits_4.size(), 2);
    EXPECT_EQ(splits_4[0], "");
    EXPECT_EQ(splits_4[1], "some text");

    std::vector<std::string_view> splits_5{};
    split(delimiters_in_middle, delimiter, splits_5);
    EXPECT_EQ(splits_5.size(), 3);
    EXPECT_EQ(splits_5[0], "some text");
    EXPECT_EQ(splits_5[1], "again some text");
    EXPECT_EQ(splits_5[2], "something_useless");

    std::vector<std::string_view> splits_6{};
    split(s_only_delimiter, delimiter, splits_6);
    EXPECT_EQ(splits_6.size(), 4);
    for (int i = 0; i < splits_6.size(); i++)
    {
        EXPECT_EQ(splits_6[i], "");
    }

    std::vector<std::string_view> splits_7{};
    split(s_empty, delimiter, splits_7);
    EXPECT_EQ(splits_7.size(), 0);

    std::vector<std::string_view> splits_8{};
    split(s_sparse, delimiter, splits_8);
    EXPECT_EQ(splits_8.size(), 6);
    for (int i = 0; i < splits_8.size(); i++)
    {
        if (i == 0) EXPECT_EQ(splits_8[i], "sparse");
        else if (i == 4) EXPECT_EQ(splits_8[i], "text");
        else EXPECT_EQ(splits_8[i], "");
    }

    std::vector<std::string_view> splits_9{};
    split(s_with_delimiter_char, delimiter, splits_9);
    EXPECT_EQ(splits_9.size(), 4);
    EXPECT_EQ(splits_9[0], "text");
    EXPECT_EQ(splits_9[1], "including");
    EXPECT_EQ(splits_9[2], "delimiter!char");
    EXPECT_EQ(splits_9[3], "in!?it");
}

TEST(SplitTests, WhiteSpaceRemoval)
{
    std::string delimiter = "::";

    std::string s_1{ "text :: to be\n::\tsplitted:: :: \twith white space\r\n" };
    std::string s_2{ " :: " };
    std::string s_3{ " ::text" };

    std::vector<std::string_view> splits_1;
    split(s_1, delimiter, splits_1);
    EXPECT_EQ(splits_1.size(), 5);
    EXPECT_EQ(splits_1[0], "text");
    EXPECT_EQ(splits_1[1], "to be");
    EXPECT_EQ(splits_1[2], "splitted");
    EXPECT_EQ(splits_1[3], "");
    EXPECT_EQ(splits_1[4], "with white space");

    std::vector<std::string_view> splits_2;
    split(s_2, delimiter, splits_2);
    EXPECT_EQ(splits_2.size(), 2);
    EXPECT_EQ(splits_2[0], "");
    EXPECT_EQ(splits_2[1], "");

    std::vector<std::string_view> splits_3;
    split(s_3, delimiter, splits_3);
    EXPECT_EQ(splits_3.size(), 2);
    EXPECT_EQ(splits_3[0], "");
    EXPECT_EQ(splits_3[1], "text");
}

TEST(ReadChunk, ReadIndividualChunks)
{
    std::stringstream source{ "_some_string_stream_some_string_stream_" };
    std::string buffer_0{};
    std::string buffer_3{};
    std::string buffer_7{};
    std::string buffer_9{};
    auto source_size = source.str().size();
    long chunk_size = 5;

    // Chunk number 0.
    bool result_0 = read_chunk(source, buffer_0, source_size, chunk_size, 0);
    // Chunk number 3.
    bool result_3 = read_chunk(source, buffer_3, source_size, chunk_size, 3);
    // Chunk number 7.
    bool result_7 = read_chunk(source, buffer_7, source_size, chunk_size, 7);
    // Chunk number 9 (must be empty & read_chunk() return false).
    bool result_9 = read_chunk(source, buffer_9, source_size, chunk_size, 9);

    EXPECT_TRUE(result_0);
    EXPECT_TRUE(result_3);
    EXPECT_TRUE(result_7);
    EXPECT_FALSE(result_9);
    EXPECT_EQ(buffer_0, "_some");
    EXPECT_EQ(buffer_3, "ream_");
    EXPECT_EQ(buffer_7, "eam_");
    EXPECT_TRUE(buffer_9.empty());
}

TEST(ReadChunk, ReadAllChunks)
{
    long size = 1'070'503;
    std::string rand_str;
    rand_str.resize(size);
    std::srand(std::time(nullptr));
    for (int i = 0; i < size; i++) rand_str += 'a' + rand() % 26;

    std::stringstream source{ rand_str };
    std::string buffer{};
    size_t source_size = rand_str.size();
    size_t chunk_size = 67;
    size_t expected_chunks_count = (rand_str.size() / chunk_size) + (rand_str.size() % chunk_size == 0 ? 0 : 1);

    std::vector<std::string> chunks{};
    int chunk_count = 0;
    while (read_chunk(source, buffer, source_size, chunk_size, chunk_count))
    {
        chunks.push_back(buffer);
        chunk_count++;
    }

    std::string chunks_put_together{};
    long chunks_total_size = 0;
    for (auto& c : chunks)
    {
        chunks_put_together += c;
        chunks_total_size += c.size();
    }

    EXPECT_EQ(chunk_count, expected_chunks_count);
    EXPECT_EQ(chunks_put_together, rand_str);
    EXPECT_EQ(chunks_total_size, rand_str.size());
}
