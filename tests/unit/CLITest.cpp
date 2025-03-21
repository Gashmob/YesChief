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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <yeschief.h>

using namespace ::testing;

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
    ASSERT_EXIT(
        cli.addOption("name,foo", "My option"),
        KilledBySignal(SIGABRT),
        HasSubstr("Short name of an option can be only one letter")
    );
}

TEST(CLI, addOptionMultiple) {
    yeschief::CLI cli("name", "description");
    cli.addOption("name", "My option").addOption("help", "Help message");
}

TEST(CLI, addOptionThrowIfAddExisting) {
    yeschief::CLI cli("name", "description");
    cli.addOption("name", "My option");
    ASSERT_EXIT(cli.addOption("name", "My option"), KilledBySignal(SIGABRT), HasSubstr("CLI has already this option"));
}

TEST(CLI, addOptionThowIfInvalidType) {
    yeschief::CLI cli("name", "description");
    ASSERT_EXIT(
        cli.addOption<yeschief::Fault>("name", "My option"),
        KilledBySignal(SIGABRT),
        HasSubstr("Type is not allowed for options")
    );
}

TEST(CLI, addOptionWithType) {
    yeschief::CLI cli("name", "description");
    cli.addOption<std::string>("name", "My option");
}

TEST(CLI, addOptionThrowIfCommandAddedBefore) {
    yeschief::CLI cli("name", "description");
    CommandStub command("");
    cli.addCommand(&command);
    ASSERT_EXIT(
        cli.addOption("name", "My option"),
        KilledBySignal(SIGABRT),
        HasSubstr("Cannot add an option group to a cli using commands")
    );
}

TEST(CLI, addGroupThenAddOption) {
    yeschief::CLI cli("name", "description");
    cli.addGroup("My group").addOption("foo", "bar");
}

TEST(CLI, addGroupThrowIfAddExisting) {
    yeschief::CLI cli("name", "description");
    cli.addGroup("My group").addOption("foo", "bar");
    ASSERT_EXIT(cli.addGroup("My group"), KilledBySignal(SIGABRT), HasSubstr("Group already exists"));
}

TEST(CLI, addGroupThrowIfCommandAddedBefore) {
    yeschief::CLI cli("name", "description");
    CommandStub command("");
    cli.addCommand(&command);
    ASSERT_EXIT(
        cli.addGroup("My group"),
        KilledBySignal(SIGABRT),
        HasSubstr("Cannot add an option group to a cli using commands")
    );
}

TEST(CLI, addCommand) {
    yeschief::CLI cli("name", "description");
    CommandStub command("");
    cli.addCommand(&command);
}

TEST(CLI, addCommandThrowIfExisting) {
    yeschief::CLI cli("name", "description");
    CommandStub command("my-command");
    cli.addCommand(&command);
    ASSERT_EXIT(cli.addCommand(&command), KilledBySignal(SIGABRT), HasSubstr("Command already exists"));
}

TEST(CLI, addCommandMultiple) {
    yeschief::CLI cli("name", "description");
    CommandStub command1("command_1");
    CommandStub command2("command_2");
    cli.addCommand(&command1).addCommand(&command2);
}

TEST(CLI, addCommandThrowIfOptionsAddedBefore) {
    yeschief::CLI cli("name", "description");
    cli.addOption("name", "My option");
    CommandStub command("my-command");
    ASSERT_EXIT(
        cli.addCommand(&command), KilledBySignal(SIGABRT), HasSubstr("Cannot add a command to a cli using options")
    );
}

TEST(CLI, parsePositionalThrowIfNonExisting) {
    yeschief::CLI cli("name", "description");
    ASSERT_EXIT(cli.parsePositional("hello"), KilledBySignal(SIGABRT), HasSubstr("Option doesn't exists"));
}

TEST(CLI, parsePositionalThrowIfRequiredAfterNonRequired) {
    yeschief::CLI cli("name", "description");
    cli.addOption("required", "This is required", {.required = true}).addOption("non-required", "This is not required");
    ASSERT_EXIT(
        cli.parsePositional("non-required", "required"),
        KilledBySignal(SIGABRT),
        HasSubstr("Option is required but is placed after a non required one")
    );
}

TEST(CLI, parsePositionalThrowIfOptionAfterListOne) {
    yeschief::CLI cli("name", "description");
    cli.addOption<std::vector<int>>("list", "This is a list").addOption("value", "This is not a list");
    ASSERT_EXIT(
        cli.parsePositional("list", "value"),
        KilledBySignal(SIGABRT),
        HasSubstr("Cannot add a new positional argument after one with a list type")
    );
}

TEST(CLI, parsePositional) {
    yeschief::CLI cli("name", "description");
    cli.addOption("foo", "Foo!", {.required = true}).addOption("bar", "Bar!");
    cli.parsePositional("foo", "bar");
}

TEST(CLI, helpWithoutOptions) {
    const auto cli = yeschief::CLI(
        "my-program",
        R"(Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam volutpat vitae felis id ornare. Etiam ac
sollicitudin arcu. Morbi aliquet mauris varius vestibulum gravida. Mauris quis laoreet lectus. Sed sit amet
pharetra tellus. Duis sed egestas dolor. Suspendisse potenti. Proin maximus efficitur tincidunt. Pellentesque
eu sodales dui. Vestibulum hendrerit finibus tortor, accumsan tincidunt urna maximus feugiat. Vivamus rhoncus
felis lacus, at ultricies ante consequat vitae. Mauris eu dignissim ex, at malesuada dui. Mauris sagittis mattis accumsan.)"
    );
    std::stringstream ss;
    cli.help(ss);
    const std::string result(std::istreambuf_iterator<char>(ss), {});

    ASSERT_STREQ(
        R"(usage:
  my-program

Lorem ipsum dolor sit amet, consectetur adipiscing elit. Etiam volutpat vitae felis id ornare. Etiam ac
sollicitudin arcu. Morbi aliquet mauris varius vestibulum gravida. Mauris quis laoreet lectus. Sed sit amet
pharetra tellus. Duis sed egestas dolor. Suspendisse potenti. Proin maximus efficitur tincidunt. Pellentesque
eu sodales dui. Vestibulum hendrerit finibus tortor, accumsan tincidunt urna maximus feugiat. Vivamus rhoncus
felis lacus, at ultricies ante consequat vitae. Mauris eu dignissim ex, at malesuada dui. Mauris sagittis mattis accumsan.

)",
        result.c_str()
    );
}

TEST(CLI, helpWithOptions) {
    yeschief::CLI cli("cli", "description");
    cli.addOption<std::string>("name,n", "My option", {.required = true}).addOption("help", "Multiline\nhelp message");
    cli.addOption<int>("number", "A number", {.value_help = "<n>"})
        .addOption<std::string>("character", "A character", {.required = true});
    cli.parsePositional("character", "number");
    cli.addGroup("Special").addOption("rand", "Display a random number");
    std::stringstream ss;
    cli.help(ss);
    const std::string result(std::istreambuf_iterator<char>(ss), {});

    ASSERT_STREQ(
        R"(usage:
  cli [OPTIONS] --name CHARACTER [NUMBER]

description

Positional arguments:

  These arguments come after options and in the order they are listed here.
  Only CHARACTER is required.

  CHARACTER [REQUIRED]
    A character

  NUMBER
    A number

Options:

  --name VALUE, -n VALUE [REQUIRED]
    My option

  --help
    Multiline
    help message

  --number <n>
    A number

  --character VALUE [REQUIRED]
    A character

Special:

  --rand
    Display a random number

)",
        result.c_str()
    );
}

TEST(CLI, helpWithCommands) {
    yeschief::CLI cli("cli", "description");
    CommandStub command("my-command");
    cli.addCommand(&command);
    std::stringstream ss;
    cli.help(ss);
    const std::string result(std::istreambuf_iterator<char>(ss), {});

    ASSERT_STREQ(
        R"(usage:
  cli [COMMAND] [OPTIONS]

description

Commands:

  my-command [OPTIONS]
    Stub class for Command.
    Description on another line.

)",
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

TEST(CLI, runReturnsFaultWhenUnrecognizedArgument) {
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

TEST(CLI, runReturnsResultWhenOptionGivenMultipleTimes) {
    yeschief::CLI cli("name", "description");
    cli.addOption<int>("foo,f", "Bar?");
    const auto result = cli.run(5, toStringArray({"name", "--foo=2", "-f", "45", "--foo=12"}).data());
    ASSERT_TRUE(result);
    ASSERT_EQ(12, std::any_cast<int>(result.value().get("foo").value()));
}

TEST(CLI, runReturnsResultVectorWhenVectorOption) {
    yeschief::CLI cli("name", "description");
    cli.addOption<std::vector<std::string>>("foo,f", "Bar?");
    const auto result = cli.run(3, toStringArray({"name", "-f=hello", "-f='world!'"}).data());
    ASSERT_TRUE(result);
    ASSERT_THAT(
        std::any_cast<std::vector<std::string>>(result.value().get("foo").value()), ElementsAre("hello", "world!")
    );
}

TEST(CLI, runReturnsFaultWhenOptionGivenWithBadType) {
    yeschief::CLI cli("name", "description");
    cli.addOption<int>("foo,f", "Bar?");
    const auto result = cli.run(2, toStringArray({"name", "--foo=false"}).data());
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::InvalidOptionType, result.error().type);
}

TEST(CLI, runReturnsResultWhenOptionIsInAnotherGroup) {
    yeschief::CLI cli("name", "description");
    cli.addGroup("My group").addOption("foo,f", "Bar?");
    const auto result = cli.run(2, toStringArray({"name", "--foo"}).data());
    ASSERT_TRUE(result);
    ASSERT_TRUE(result.value().get("foo"));
}

TEST(CLI, runReturnsFaultWhenRequiredPositionalNotGiven) {
    yeschief::CLI cli("name", "description");
    cli.addOption<int>("foo", "bar", {.required = true});
    cli.parsePositional("foo");
    const auto result = cli.run(1, toStringArray({"name"}).data());
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::MissingRequiredOption, result.error().type);
}

TEST(CLI, runReturnsResultWhenPositionalGiven) {
    yeschief::CLI cli("name", "description");
    cli.addOption<int>("foo", "bar", {.required = true});
    cli.parsePositional("foo");
    const auto result = cli.run(2, toStringArray({"name", "2"}).data());
    ASSERT_TRUE(result);
    ASSERT_EQ(2, std::any_cast<int>(result->get("foo").value()));
}

TEST(CLI, runReturnsResultForOptionWithDefaultValue) {
    yeschief::CLI cli("name", "description");
    cli.addOption<double>("foo", "bar", {.default_value = 3.14});
    const auto result = cli.run(1, toStringArray({"name"}).data());
    ASSERT_TRUE(result);
    ASSERT_EQ(3.14, std::any_cast<double>(result.value().get("foo").value()));
}

TEST(CLI, runReturnsFaultIfOptionGivenWithoutValue) {
    yeschief::CLI cli("name", "description");
    cli.addOption<int>("foo", "bar");
    const auto result = cli.run(2, toStringArray({"name", "--foo"}).data());
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::MissingOptionValue, result.error().type);
}

TEST(CLI, runReturnsResultIfImplicitOptionWithoutValue) {
    yeschief::CLI cli("name", "description");
    cli.addOption<int>("foo", "bar", {.implicit_value = 42});
    const auto result = cli.run(2, toStringArray({"name", "--foo"}).data());
    ASSERT_TRUE(result);
    ASSERT_EQ(42, std::any_cast<int>(result.value().get("foo").value()));
}

TEST(CLI, runReturnsEmptyWhenNoCommands) {
    yeschief::CLI cli("name", "description");
    CommandStub command("my-command");
    cli.addCommand(&command);
    const auto result = cli.run(1, toStringArray({"name"}).data());
    ASSERT_TRUE(result);
}

TEST(CLI, runReturnsFaultWhenCommandDoNotExists) {
    yeschief::CLI cli("name", "description");
    CommandStub command("my-command");
    cli.addCommand(&command);
    const auto result = cli.run(2, toStringArray({"name", "help"}).data());
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::UnknownCommand, result.error().type);
}

TEST(CLI, runReturnsResultWhenCommand) {
    yeschief::CLI cli("name", "description");
    CommandStub command("my-command");
    cli.addCommand(&command);
    ASSERT_EXIT(cli.run(2, toStringArray({"name", "my-command"}).data()), ExitedWithCode(0), ".*");
}

TEST(CLI, runReturnsFaultWhenCommandWithBadArgs) {
    yeschief::CLI cli("name", "description");
    CommandStub command("my-command");
    cli.addCommand(&command);
    const auto result = cli.run(3, toStringArray({"name", "my-command", "--foo"}).data());
    ASSERT_FALSE(result);
    ASSERT_EQ(yeschief::FaultType::UnrecognizedOption, result.error().type);
}

TEST(CLI, runReturnsResultWhenCommandWithArgs) {
    yeschief::CLI cli("name", "description");
    CommandStub command("my-command");
    cli.addCommand(&command);
    ASSERT_EXIT(cli.run(3, toStringArray({"name", "my-command", "--exit=12"}).data()), ExitedWithCode(12), ".*");
}
