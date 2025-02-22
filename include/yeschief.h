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
#ifndef YESCHIEF_H
#define YESCHIEF_H
/**
 * **Unique header file of the library, it contains all the tools needed for the usage of it.**
 *
 * Usage documentation is available here: [Usage documentation](USAGE.md)
 */

#include <any>
#include <expected>
#include <map>
#include <optional>
#include <string>

namespace yeschief {
class CLI;
class OptionGroup;
class Command;
class CLIResults;

/**
 * Advanced configuration of an option
 */
typedef struct {
    /**
     * Set to `true` if the option is mandatory. Default to `false`
     *
     * `CLI::run` will return a `Fault` if a required option is not set by the user
     */
    bool required;
} OptionConfiguration;

/**
 * Wrapper around an exception message
 */
typedef struct {
    /**
     * The message of the exception. Can be useful for the user to understand what is wrong
     */
    std::string message;
} Fault;

/**
 * Main class of the library. It represents the program itself and manage options and commands
 */
class CLI final {
  public:
    /**
     * @param name Name of your program
     * @param description Description of your program
     */
    CLI(std::string name, std::string description);

    /**
     * Add an option to your program.
     *
     * The name of the option can be written in 2 ways:
     * - "name" -> for `--name`
     * - "name,n" -> for `--name` and `-n`
     *
     * For the second way, the format should always be `<long>,<short>`. For the long name you can have any length but
     * the short name must be only 1 letter
     *
     * @param name Name of the option
     * @param description Description of the option
     * @param configuration Advanced configuration
     * @return The CLI object itself to chain calls
     */
    auto addOption(const std::string &name, const std::string &description, OptionConfiguration &configuration)
        -> CLI &;

    /**
     * Add a group of options to your program. You can then add options to your group the exact same way you add them to
     * the current class
     *
     * @param name Name of the group
     * @return The created OptionGroup, you can then customize it
     */
    [[nodiscard]] auto addGroup(const std::string &name) -> OptionGroup &;

    /**
     * Add a command to your program
     *
     * The added command must be a child class of Command. The object passed in parameters will be the one used when
     * launching CLI::run
     *
     * @param command The Command instance
     * @return The CLI object itself to chain calls
     */
    auto addCommand(const Command &command) -> CLI &;

    /**
     * Allow to set some options as positional arguments. This way the user will no longer need to use option name to
     * set its value
     *
     * Please note that if an option is repeatable it must be placed as the last one
     *
     * @param option List of options names to parse as positional arguments
     * @param ...
     */
    auto parsePositional(const std::string &option, ...) -> void;

    /**
     * Parse argv against defined options and commands.
     *
     * - In case you specified options, it will return a CLIResult instance with values associated to each option
     * - In case you specified commands, it will launch run method of corresponding command with a CLIResult instance
     * with values associated to each options of the command. If the command have sub-commands, it will chain the calls
     * recursively
     *
     * In both case if something went wrong during argv parsing, the method will return a Fault with the corresponding
     * message
     *
     * @param argc Count of argument
     * @param argv Array of argument
     * @return The result of the parsing or a Fault if something went wrong
     */
    auto run(int argc, char **argv) const -> std::expected<CLIResults, Fault>;

    /**
     * Display a help message built from defined options or commands with this template:
     *
     * ```text
     * usage:
     *     <program name> [OPTIONS] <REQUIRED OPTIONS> <POSITIONAL ARGUMENTS...> <- if options
     *     <program name> [COMMAND] [OPTIONS]                                    <- if commands
     *
     * <program description>
     *
     * Positional arguments: <- if options and positional arguments not empty
     *
     *     These arguments come after any options and in the order they are listed here.
     *     Only <REQUIRED POSITIONAL ARGUMENTS> is required.
     *
     *     <LIST OF POSITIONAL ARGUMENT>
     *         <DESCRIPTION OF EACH ARGUMENT>
     *
     * <Option group name>: <- if options (if there is no option group, the group will be named "Options")
     *
     *     <List of options> [REQUIRED]
     *         <Description of each option>
     *
     *     --help
     *         Display this help message
     *
     * Commands: <- if commands
     *
     *     <List of commands usage>
     *         <Description of each command>
     *
     *     help [COMMAND]
     *         Display this help message. If command is specified, display help for this command
     * ```
     *
     * To see example of outputs, please refer to the usage documentation: [Usage documentation](USAGE.md)
     */
    auto help() const -> void;

  private:
    std::string _name;
    std::string _description;
};

/**
 * Represents a group of options under the same namespace
 */
class OptionGroup final {
  public:
    /**
     * Add an option to your group.
     *
     * The name of the option can be written in 2 ways:
     * - "name" -> for `--name`
     * - "name,n" -> for `--name` and `-n`
     *
     * For the second way, the format should always be `<long>,<short>`. For the long name you can have any length but
     * the short name must be only 1 letter
     *
     * @param name Name of the option
     * @param description Description of the option
     * @param configuration Advanced configuration
     * @return The OptionGroup object itself to chain calls
     */
    auto addOption(const std::string &name, const std::string &description, OptionConfiguration &configuration)
        -> OptionGroup &;
};

/**
 * A command that can be executed
 */
class Command {
  public:
    /**
     * Configure options or sub-commands of the current command.
     *
     * The configuration is done with a CLI object the same way you do it with your own one
     *
     * @param cli The sub CLI of the command
     */
    virtual auto setup(CLI &cli) -> void;

    /**
     * Parse argv against configuration of the command and pass the result to this method.
     *
     * This method is launched when parent CLI is run with an argv corresponding to the current command
     *
     * @param results Results of the argv parsing
     * @return Exit code of command
     */
    virtual auto run(const CLIResults &results) -> int = 0;

    virtual ~Command() = default;
};

/**
 * Contains options values collected by parsing argv
 */
class CLIResults final {
  public:
    /**
     * @param values Values for each given option
     */
    explicit CLIResults(const std::map<std::string, std::any> &values);

    /**
     * Returns value set by the user for a specific option. If the user doesn't have set the option, then it returns
     * std::nullopt
     *
     * @param option Name of the option
     * @return Value of the option or std::nullopt if option has no value
     */
    [[nodiscard]] auto get(const std::string &option) const -> std::optional<std::any>;

  private:
    std::map<std::string, std::any> _values;
};
} // namespace yeschief

#endif // YESCHIEF_H
