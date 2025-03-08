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
#include "yeschief.h"

using namespace yeschief;

HelpCommand::HelpCommand(CLI *cli): _cli(cli) {}

auto HelpCommand::getName() const -> std::string {
    return "help";
}

auto HelpCommand::getDescription() const -> std::string {
    return "Display this help message\n"
           "When COMMAND is given, display help for this command";
}

auto HelpCommand::setup(CLI &cli) -> void {
    cli.addOption<std::string>("command", "Display help of this command");
    cli.parsePositional("command");
}

auto HelpCommand::run(const CLIResults &results) -> int {
    const auto command_option = results.get("command");
    if (command_option.has_value()) {
        const auto command_name = std::any_cast<std::string>(command_option.value());
        if (_cli->_commands_cli.contains(command_name)) {
            _cli->_commands_cli.at(command_name).help(std::cout);
            return 0;
        }

        _cli->help(std::cout);
        return 1;
    }

    _cli->help(std::cout);

    return 0;
}
