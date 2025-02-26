/*
 * MIT License
 *
 * Copyright (c) 2025-Present Kevin Traini
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "../../src/utils.h"

#include "test_tools.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace ::testing;

TEST(utils, joinReturnsEmptyWhenEmpty) {
    ASSERT_STREQ("", yeschief::join({}).c_str());
}

TEST(utils, joinReturnsFirstWhenOnlyOneElement) {
    ASSERT_STREQ("a", yeschief::join({"a"}).c_str());
}

TEST(utils, joinWithoutDelemiter) {
    ASSERT_STREQ("abc", yeschief::join({"a", "b", "c"}).c_str());
}

TEST(utils, joinWithDelimiter) {
    ASSERT_STREQ("a, b, c", yeschief::join({"a", "b", "c"}, ", ").c_str());
}

TEST(utils, splitEmpty) {
    ASSERT_THAT(yeschief::split("", " "), IsEmpty());
}

TEST(utils, splitOnSpace) {
    ASSERT_THAT(yeschief::split("a b c", " "), ElementsAre("a", "b", "c"));
}

TEST(utils, splitOnSpaceNoEmpty) {
    ASSERT_THAT(yeschief::split("a b    c", " "), ElementsAre("a", "b", "c"));
}

TEST(utils, splitOnLongString) {
    ASSERT_THAT(yeschief::split("ahellobhelloc", "hello"), ElementsAre("a", "b", "c"));
}

TEST(utils, parseArgvEmptyReturnsEmpty) {
    const auto &[raw_results, option_order] = yeschief::parseArgv(0, {}, {}).value();
    ASSERT_THAT(raw_results, IsEmpty());
    ASSERT_THAT(option_order, IsEmpty());
}

TEST(utils, parseArgvSimpleLongOption) {
    ASSERT_THAT(
        yeschief::parseArgv(1, toStringArray({"--name"}).data(), {"name"}).value().raw_results,
        ElementsAre(Pair("name", ElementsAre("true")))
    );
}

TEST(utils, parseArgvLongOptionEqualValue) {
    ASSERT_THAT(
        yeschief::parseArgv(1, toStringArray({"--name=value"}).data(), {"name"}).value().raw_results,
        ElementsAre(Pair("name", ElementsAre("value")))
    );
}

TEST(utils, parseArgvLongOptionEqualStringValue) {
    ASSERT_THAT(
        yeschief::parseArgv(1, toStringArray({"--name='value'"}).data(), {"name"}).value().raw_results,
        ElementsAre(Pair("name", ElementsAre("value")))
    );
    ASSERT_THAT(
        yeschief::parseArgv(1, toStringArray({"--name=\"value\""}).data(), {"name"}).value().raw_results,
        ElementsAre(Pair("name", ElementsAre("value")))
    );
}

TEST(utils, parseArgvLongOptionWithValue) {
    ASSERT_THAT(
        yeschief::parseArgv(2, toStringArray({"--name", "value"}).data(), {"name"}).value().raw_results,
        ElementsAre(Pair("name", ElementsAre("value")))
    );
}

TEST(utils, parseArgvLongOptionWithValues) {
    ASSERT_THAT(
        yeschief::parseArgv(4, toStringArray({"--name", "value1", "--name", "value2"}).data(), {"name"})
            .value()
            .raw_results,
        ElementsAre(Pair("name", ElementsAre("value1", "value2")))
    );
}

TEST(utils, parseArgvSimpleShortOption) {
    ASSERT_THAT(
        yeschief::parseArgv(1, toStringArray({"-n"}).data(), {"n"}).value().raw_results,
        ElementsAre(Pair("n", ElementsAre("true")))
    );
}

TEST(utils, parseArgvShortOptionEqualValue) {
    ASSERT_THAT(
        yeschief::parseArgv(1, toStringArray({"-n=value"}).data(), {"n"}).value().raw_results,
        ElementsAre(Pair("n", ElementsAre("value")))
    );
}

TEST(utils, parseArgvShortOptionEqualStringValue) {
    ASSERT_THAT(
        yeschief::parseArgv(1, toStringArray({"-n='value'"}).data(), {"n"}).value().raw_results,
        ElementsAre(Pair("n", ElementsAre("value")))
    );
    ASSERT_THAT(
        yeschief::parseArgv(1, toStringArray({"-n=\"value\""}).data(), {"n"}).value().raw_results,
        ElementsAre(Pair("n", ElementsAre("value")))
    );
}

TEST(utils, parseArgvShortOptionWithValue) {
    ASSERT_THAT(
        yeschief::parseArgv(2, toStringArray({"-n", "value"}).data(), {"n"}).value().raw_results,
        ElementsAre(Pair("n", ElementsAre("value")))
    );
}

TEST(utils, parseArgvShortOptionWithValues) {
    ASSERT_THAT(
        yeschief::parseArgv(4, toStringArray({"-n", "value1", "-n", "value2"}).data(), {"n"}).value().raw_results,
        ElementsAre(Pair("n", ElementsAre("value1", "value2")))
    );
}

TEST(utils, parseArgvMultipleOptions) {
    const auto [raw_results, option_order]
        = yeschief::parseArgv(3, toStringArray({"-n", "value", "--number=3"}).data(), {"n", "number"}).value();
    ASSERT_THAT(raw_results, ElementsAre(Pair("n", ElementsAre("value")), Pair("number", ElementsAre("3"))));
    ASSERT_THAT(option_order, ElementsAre("n", "number"));
}

TEST(utils, parseArgvSetTrueWhenNoValue) {
    ASSERT_THAT(
        yeschief::parseArgv(1, toStringArray({"-n"}).data(), {"n"}).value().raw_results,
        ElementsAre(Pair("n", ElementsAre("true")))
    );
    ASSERT_THAT(
        yeschief::parseArgv(2, toStringArray({"-n", "--version"}).data(), {"n", "version"}).value().raw_results,
        ElementsAre(Pair("n", ElementsAre("true")), Pair("version", ElementsAre("true")))
    );
}

TEST(utils, parseArgvFaultWhenValueGivenWithoutOption) {
    const auto result = yeschief::parseArgv(1, toStringArray({"value"}).data(), {});
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(yeschief::FaultType::UnrecognizedOption, result.error().type);
}

TEST(utils, parseArgvFaultWhenOptionIsNotAllowed) {
    const auto result = yeschief::parseArgv(1, toStringArray({"--option"}).data(), {});
    ASSERT_FALSE(result.has_value());
    ASSERT_EQ(yeschief::FaultType::UnrecognizedOption, result.error().type);
}

TEST(utils, toBoolean) {
    ASSERT_TRUE(yeschief::toBoolean("true").value());
    ASSERT_TRUE(yeschief::toBoolean("1").value());
    ASSERT_FALSE(yeschief::toBoolean("false").value());
    ASSERT_FALSE(yeschief::toBoolean("0").value());
    ASSERT_EQ(yeschief::FaultType::InvalidOptionType, yeschief::toBoolean("foobar").error().type);
}

TEST(utils, toInt) {
    ASSERT_EQ(1, yeschief::toInt("1").value());
    ASSERT_EQ(1, yeschief::toInt("+1").value());
    ASSERT_EQ(123456, yeschief::toInt("123456").value());
    ASSERT_EQ(-2, yeschief::toInt("-2").value());
    ASSERT_EQ(yeschief::FaultType::InvalidOptionType, yeschief::toInt("blabla").error().type);
}

TEST(utils, toFloat) {
    ASSERT_EQ(1.0f, yeschief::toFloat("1").value());
    ASSERT_EQ(0.1f, yeschief::toFloat(".1").value());
    ASSERT_EQ(0.1f, yeschief::toFloat("0.1").value());
    ASSERT_EQ(-3.4f, yeschief::toFloat("-3.4").value());
    ASSERT_EQ(.81f, yeschief::toFloat("+.81").value());
    ASSERT_EQ(yeschief::FaultType::InvalidOptionType, yeschief::toFloat("hello").error().type);
}

TEST(utils, toDouble) {
    ASSERT_EQ(1.0, yeschief::toDouble("1").value());
    ASSERT_EQ(0.1, yeschief::toDouble(".1").value());
    ASSERT_EQ(0.1, yeschief::toDouble("0.1").value());
    ASSERT_EQ(-3.4, yeschief::toDouble("-3.4").value());
    ASSERT_EQ(.81, yeschief::toDouble("+.81").value());
    ASSERT_EQ(yeschief::FaultType::InvalidOptionType, yeschief::toDouble("world").error().type);
}
