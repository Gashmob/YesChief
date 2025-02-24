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

TEST(CLI, addOption) {
    yeschief::CLI cli("name", "description");
    cli.addOption("name", "My option");
}

TEST(CLI, addOptionWithShortName) {
    yeschief::CLI cli("name", "description");
    cli.addOption("name,n", "My option");
}

TEST(CLI, addOptionWithInvalidShortName) {
    yeschief::CLI cli("name", "description");
    ASSERT_THROW(cli.addOption("name,foo", "My option"), std::logic_error);
}

TEST(CLI, addOptionMultiple) {
    yeschief::CLI cli("name", "description");
    cli.addOption("name", "My option").addOption("help", "Help message");
}

TEST(CLI, addOptionThrowIfAddExisting) {
    yeschief::CLI cli("name", "description");
    cli.addOption("name", "My option");
    ASSERT_THROW(cli.addOption("name", "My option"), std::logic_error);
}

TEST(CLI, helpWithoutOptions) {
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

TEST(CLI, helpWithOptions) {
    yeschief::CLI cli("cli", "description");
    cli.addOption("name,n", "My option", {.required = true}).addOption("help", "Multiline\nhelp message");
    std::stringstream ss;
    cli.help(ss);
    const std::string result(std::istreambuf_iterator<char>(ss), {});

    ASSERT_STREQ(
        "usage:\n"
        "\tcli [OPTIONS] --name\n"
        "\n"
        "description\n"
        "\n"
        "Options:\n"
        "\n"
        "\t--name, -n REQUIRED\n"
        "\t\tMy option\n"
        "\t--help\n"
        "\t\tMultiline\n"
        "\t\thelp message\n",
        result.c_str()
    );
}

TEST(CLI, runReturnsFaultWhenArgc0) {
    const yeschief::CLI cli("name", "description");
    const auto result = cli.run(0, {});
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::InvalidArgs, result.error().type);
}

TEST(CLI, runReturnsEmptyWhenArgc1) {
    const yeschief::CLI cli("name", "description");
    const auto result = cli.run(1, toStringArray({"name"}).data());
    ASSERT_TRUE(result);
}

TEST(CLI, runReturnsFaultWhenUnrecognizedLongOption) {
    const yeschief::CLI cli("name", "description");
    const auto result = cli.run(2, toStringArray({"name", "--foo"}).data());
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::UnrecognizedOption, result.error().type);
}

TEST(CLI, runReturnsFaultWhenUnrecognizedShortOption) {
    const yeschief::CLI cli("name", "description");
    const auto result = cli.run(2, toStringArray({"name", "-f"}).data());
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::UnrecognizedOption, result.error().type);
}

TEST(CLI, runReturnsFaultWhenUnrecognizedArguement) {
    const yeschief::CLI cli("name", "description");
    const auto result = cli.run(2, toStringArray({"name", "bar"}).data());
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::UnrecognizedOption, result.error().type);
}

TEST(CLI, runReturnsResultsWhenLongOptionGiven) {
    yeschief::CLI cli("name", "description");
    cli.addOption("foo,f", "Bar?");
    const auto result = cli.run(2, toStringArray({"name", "--foo"}).data());
    ASSERT_TRUE(result);
    ASSERT_TRUE(result.value().get("foo"));
}

TEST(CLI, runReturnsResultWhenShortOptionGiven) {
    yeschief::CLI cli("name", "description");
    cli.addOption("foo,f", "Bar?");
    const auto result = cli.run(2, toStringArray({"name", "-f"}).data());
    ASSERT_TRUE(result);
    ASSERT_TRUE(result.value().get("foo"));
}

TEST(CLI, runReturnsFaultWhenMissingRequiredOption) {
    yeschief::CLI cli("name", "description");
    cli.addOption("foo,f", "Bar?", {.required = true});
    const auto result = cli.run(1, toStringArray({"name"}).data());
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::MissingRequiredOption, result.error().type);
}
