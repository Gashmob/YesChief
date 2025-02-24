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
