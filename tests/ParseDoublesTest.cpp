#include "gtest/gtest.h"
#include "utils.h"
#include <vector>
#include <string>

TEST(ParseDoublesTest, HandlesValidInput)
{
    std::string line = "1.0 2.5 -3.0 0.0";
    std::vector<double> result;
    ASSERT_TRUE(parseDoubles(line, result));
    std::vector<double> expected = {1.0, 2.5, -3.0, 0.0};
    ASSERT_EQ(expected.size(), result.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_DOUBLE_EQ(expected[i], result[i]);
    }
}

TEST(ParseDoublesTest, HandlesEmptyInput)
{
    std::string line = "";
    std::vector<double> result;
    ASSERT_TRUE(parseDoubles(line, result));
    ASSERT_TRUE(result.empty());
}

TEST(ParseDoublesTest, HandlesWhitespaceOnlyInput)
{
    std::string line = "   \t  ";
    std::vector<double> result;
    ASSERT_TRUE(parseDoubles(line, result));
    ASSERT_TRUE(result.empty());
}

TEST(ParseDoublesTest, HandlesInvalidCharacters)
{
    std::string line = "1.0 2.a 3.0";
    std::vector<double> result;
    ASSERT_FALSE(parseDoubles(line, result));
}

TEST(ParseDoublesTest, HandlesMixedValidAndInvalid)
{
    std::string line = "1.0 2.0abc 3.0";
    std::vector<double> result;
    ASSERT_FALSE(parseDoubles(line, result));
}

TEST(ParseDoublesTest, HandlesLeadingAndTrailingWhitespace)
{
    std::string line = "  1.0 2.0   ";
    std::vector<double> result;
    ASSERT_TRUE(parseDoubles(line, result));
    std::vector<double> expected = {1.0, 2.0};
    ASSERT_EQ(expected.size(), result.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_DOUBLE_EQ(expected[i], result[i]);
    }
}
