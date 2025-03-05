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
#include <cstdarg>
#include <expected>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <vector>

namespace yeschief {
class CLI;
class Option;
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
    bool required                         = false;
    /**
     * Placeholder in help message for the option value
     */
    std::string value_help                = "VALUE";
    /**
     * Default value for this option. Will be set if option is not set by the user
     */
    std::optional<std::any> default_value = std::nullopt;
} OptionConfiguration;

/**
 * Different types of Fault, ease the treatment of Faults
 */
typedef enum {
    /**
     * Has received bad arguments. Mostly a dev error and not a user one
     */
    InvalidArgs,
    /**
     * User have given an unrecognized option
     */
    UnrecognizedOption,
    /**
     * User have not given a required option
     */
    MissingRequiredOption,
    /**
     * User have given a wrongly typed value for option
     */
    InvalidOptionType,
    /**
     * Command not found
     */
    UnknownCommand,
} FaultType;

/**
 * Wrapper around an exception message
 */
typedef struct {
    /**
     * The message of the exception. Can be useful for the user to understand what is wrong
     */
    std::string message;
    /**
     * Type of the fault
     */
    FaultType type;
} Fault;

/**
 * An option, nothing else
 */
class Option final {
  public:
    /**
     * @param name Name of the option (`--name`)
     * @param short_name Short name of the option (`-n`)
     * @param description What is does?
     * @param type Type of the option (bool,int,...)
     * @param configuration Advanced configuration of the option
     */
    Option(
        const std::string &name,
        const std::string &short_name,
        const std::string &description,
        const std::type_info &type,
        const OptionConfiguration &configuration
    );

    [[nodiscard]] auto getName() const -> std::string;

    [[nodiscard]] auto getShortName() const -> std::string;

    [[nodiscard]] auto getDescription() const -> std::string;

    [[nodiscard]] auto getType() const -> const std::type_info &;

    [[nodiscard]] auto getConfiguration() const -> OptionConfiguration;

  private:
    const std::string _name;
    const std::string _short_name;
    const std::string _description;
    const std::type_info &_type;
    const OptionConfiguration _configuration;
};

/**
 * Represents a group of options under the same namespace
 */
class OptionGroup final {
    friend class CLI;

  public:
    /**
     * @param parent Owner of this group
     * @param name Name of this group
     */
    explicit OptionGroup(CLI *parent, std::string name);

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
    template<typename T = bool>
    auto
    addOption(const std::string &name, const std::string &description, const OptionConfiguration &configuration = {})
        -> OptionGroup &;

  private:
    CLI *_parent;
    std::string _name;
    std::vector<std::shared_ptr<Option>> _options;

    auto addOption(const std::shared_ptr<Option> &option) -> void;
};

/**
 * Main class of the library. It represents the program itself and manage options and commands
 */
class CLI final {
    friend class OptionGroup;

    enum Mode { OPTIONS, COMMANDS };

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
    template<typename T = bool>
    auto
    addOption(const std::string &name, const std::string &description, const OptionConfiguration &configuration = {})
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
    auto addCommand(Command &command) -> CLI &;

    /**
     * Allow to set some options as positional arguments. This way the user will no longer need to use option name to
     * set its value
     *
     * Please note that if an option is repeatable it must be placed as the last one
     *
     * @param option_name
     * @param options List of options names to parse as positional arguments
     */
    template<typename... Tail> auto parsePositional(const std::string &option_name, Tail &&...options) -> void;

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
     *     These arguments come after options and in the order they are listed here.
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
     *
     * @param out Out stream on which help message is printed (default to console output STDOUT)
     */
    auto help(std::ostream &out = std::cout) const -> void;

  private:
    std::string _name;
    std::string _description;
    std::optional<Mode> _mode;
    std::map<std::string, OptionGroup> _groups;
    std::map<std::string, std::shared_ptr<Option>> _options;
    std::vector<std::string> _positional_options;
    std::map<std::string, Command *> _commands;
    std::map<std::string, CLI> _commands_cli;

    [[nodiscard]] auto buildUsageHelp() const -> std::string;

    [[nodiscard]] auto buildPositionalHelp() const -> std::string;

    [[nodiscard]] static auto buildOptionUsageHelp(const std::shared_ptr<Option> &option) -> std::string;

    static auto checkOptionType(const std::type_info &type) -> void;

    [[nodiscard]] static auto
    getValueForOption(const std::shared_ptr<Option> &option, const std::vector<std::string> &values)
        -> std::expected<std::any, Fault>;

    template<typename T = bool>
    auto addOption(
        const std::string &name,
        const std::string &description,
        const std::string &group_name,
        const OptionConfiguration &configuration
    ) -> CLI &;

    auto parsePositional() -> void {
        // Nothing to do here
    }
};

/**
 * A command that can be executed
 */
class Command {
  public:
    /**
     * @return Name of the command
     */
    [[nodiscard]] virtual auto getName() const -> std::string = 0;
    /**
     * @return Description of the command
     */
    [[nodiscard]] virtual auto getDescription() const -> std::string;
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

template<typename T>
auto yeschief::OptionGroup::addOption(
    const std::string &name, const std::string &description, const OptionConfiguration &configuration
) -> OptionGroup & {
    _parent->addOption<T>(name, description, _name, configuration);
    return *this;
}

template<typename T>
auto yeschief::CLI::addOption(
    const std::string &name, const std::string &description, const OptionConfiguration &configuration
) -> CLI & {
    return addOption<T>(name, description, "", configuration);
}

template<typename T>
auto yeschief::CLI::addOption(
    const std::string &name,
    const std::string &description,
    const std::string &group_name,
    const OptionConfiguration &configuration
) -> CLI & {
    if (_mode.has_value() && _mode.value() == COMMANDS) {
        throw std::logic_error("Cannot add an option group to a cli using commands");
    }
    _mode = OPTIONS;

    if (_options.contains(name)) {
        throw std::logic_error("CLI has already an option '" + name + "'");
    }
    if (! _groups.contains(group_name)) {
        throw std::logic_error("Option group '" + group_name + "' does not exist");
    }

    std::string long_name = name;
    std::string short_name;
    const std::regex name_regex("(.*),(.*)");
    if (std::smatch match; std::regex_match(name, match, name_regex)) {
        if (match.size() == 3) {
            long_name  = match[1].str();
            short_name = match[2].str();
            if (short_name.length() != 1 || ! isalpha(short_name[0])) {
                throw std::logic_error("Short name of an option can be only one letter, got '" + short_name + "'");
            }
        }
    }

    checkOptionType(typeid(T));
    auto option = std::make_shared<Option>(long_name, short_name, description, typeid(T), configuration);
    _options.insert(std::make_pair(long_name, option));
    _groups.at(group_name).addOption(option);

    return *this;
}

template<typename... Tail>
auto yeschief::CLI::parsePositional(const std::string &option_name, Tail &&...options) -> void {
    if (! _options.contains(option_name)) {
        throw std::logic_error("Option '" + option_name + "' doesn't exist");
    }
    const auto option = _options.at(option_name);
    if (! _positional_options.empty()) {
        const auto last_option_name  = _positional_options[_positional_options.size() - 1];
        const auto last_option       = _options.at(last_option_name);
        const auto &last_option_type = last_option->getType();
        if (last_option_type == typeid(std::vector<int>) || last_option_type == typeid(std::vector<float>)
            || last_option_type == typeid(std::vector<double>)) {
            throw std::logic_error("Cannot add a new positional argument after one with a list type");
        }
        if (option->getConfiguration().required && ! last_option->getConfiguration().required) {
            throw std::logic_error(
                "Option '" + option_name + "' is required but is placed after a non required one '" + last_option_name
                + "'"
            );
        }
    }

    _positional_options.push_back(option_name);
    parsePositional(std::forward<Tail>(options)...);
}

#endif // YESCHIEF_H
