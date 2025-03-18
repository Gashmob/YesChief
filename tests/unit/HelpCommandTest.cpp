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
#include <gtest/gtest.h>
#include <test_tools.hpp>
#include <yeschief.h>

using namespace ::testing;

TEST(HelpCommand, runWithEmptyResultsDisplayFullHelp) {
    yeschief::CLI cli("name", "description");
    yeschief::HelpCommand help(&cli);
    cli.addCommand(&help);

    internal::CaptureStdout();
    ASSERT_EQ(0, help.run(yeschief::CLIResults({})));
    const auto output = internal::GetCapturedStdout();
    ASSERT_STREQ(
        R"(usage:
  name [COMMAND] [OPTIONS]

description

Commands:

  help [OPTIONS] [COMMAND]
    Display this help message
    When COMMAND is given, display help for this command

)",
        output.c_str()
    );
}

TEST(HelpCommand, runWithUnknownCommandDisplayFullHelp) {
    yeschief::CLI cli("name", "description");
    yeschief::HelpCommand help(&cli);
    cli.addCommand(&help);

    std::map<std::string, std::any> option_values;
    option_values.insert(std::make_pair("command", std::string("something")));
    internal::CaptureStdout();
    ASSERT_EQ(1, help.run(yeschief::CLIResults(option_values)));
    const auto output = internal::GetCapturedStdout();
    ASSERT_STREQ(
        R"(usage:
  name [COMMAND] [OPTIONS]

description

Commands:

  help [OPTIONS] [COMMAND]
    Display this help message
    When COMMAND is given, display help for this command

)",
        output.c_str()
    );
}

TEST(HelpCommand, runWithCommandDisplayCommandHelp) {
    yeschief::CLI cli("name", "description");
    yeschief::HelpCommand help(&cli);
    CommandStub command("my-command");
    cli.addCommand(&command);
    cli.addCommand(&help);

    std::map<std::string, std::any> option_values;
    option_values.insert(std::make_pair("command", std::string("my-command")));
    internal::CaptureStdout();
    ASSERT_EQ(0, help.run(yeschief::CLIResults(option_values)));
    const auto output = internal::GetCapturedStdout();
    ASSERT_STREQ(
        R"(usage:
  my-command [OPTIONS]

Stub class for Command.
Description on another line.

Options:

  --exit VALUE
    Exit code of command

)",
        output.c_str()
    );
}
