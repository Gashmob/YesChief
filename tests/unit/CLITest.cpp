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
#include "test_tools.hpp"

#include <gtest/gtest.h>
#include <yeschief.h>

TEST(CLI, itReturnsResults) {
    const auto cli = yeschief::CLI("name", "description");
    ASSERT_TRUE(cli.run(0, {}));
}

TEST(CLI, itReturnsFault) {
    const auto cli    = yeschief::CLI("name", "description");
    const auto result = cli.run(1, toStringArray({"name"}).data());
    ASSERT_FALSE(result);
    ASSERT_STREQ("argc is greater than 0", result.error().message.c_str());
}

TEST(CLI, helpMessage) {
    const auto cli = yeschief::CLI(
        "my-program",
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam volutpat vitae felis id ornare. Etiam ac\n"
        "sollicitudin arcu. Morbi aliquet mauris varius vestibulum gravida. Mauris quis laoreet lectus. Sed sit amet\n"
        "pharetra tellus. Duis sed egestas dolor. Suspendisse potenti. Proin maximus efficitur tincidunt. "
        "Pellentesque\n"
        "eu sodales dui. Vestibulum hendrerit finibus tortor, accumsan tincidunt urna maximus feugiat. Vivamus "
        "rhoncus\n"
        "felis lacus, at ultricies ante consequat vitae. Mauris eu dignissim ex, at malesuada dui. Mauris sagittis\n"
        "mattis accumsan."
    );
    std::stringstream ss;
    cli.help(ss);
    const std::string result(std::istreambuf_iterator<char>(ss), {});

    ASSERT_STREQ(
        "usage:\n"
        "\tmy-program\n"
        "\n"
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam volutpat vitae felis id ornare. Etiam ac\n"
        "sollicitudin arcu. Morbi aliquet mauris varius vestibulum gravida. Mauris quis laoreet lectus. Sed sit amet\n"
        "pharetra tellus. Duis sed egestas dolor. Suspendisse potenti. Proin maximus efficitur tincidunt. "
        "Pellentesque\n"
        "eu sodales dui. Vestibulum hendrerit finibus tortor, accumsan tincidunt urna maximus feugiat. Vivamus "
        "rhoncus\n"
        "felis lacus, at ultricies ante consequat vitae. Mauris eu dignissim ex, at malesuada dui. Mauris sagittis\n"
        "mattis accumsan.\n"
        "\n",
        result.c_str()
    );
}
