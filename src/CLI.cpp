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
#include "utils.h"
#include "yeschief.h"

#include <ranges>
#include <regex>
#include <typeindex>
#include <utility>

using namespace yeschief;

CLI::CLI(std::string name, std::string description)
    : _name(std::move(name)), _description(std::move(description)), _mode(std::nullopt) {
    _groups.insert(std::make_pair("", OptionGroup(this, "")));
}

auto CLI::addGroup(const std::string &name) -> OptionGroup & {
    if (_mode.has_value() && _mode.value() == COMMANDS) {
        throw std::logic_error("Cannot add an option group to a cli using commands");
    }
    _mode = OPTIONS;

    if (_groups.contains(name)) {
        throw std::logic_error("Group '" + name + "' already exists");
    }

    _groups.insert(std::make_pair(name, OptionGroup(this, name)));
    return _groups.at(name);
}

auto CLI::addCommand(Command *command) -> CLI & {
    if (_mode.has_value() && _mode.value() == OPTIONS) {
        throw std::logic_error("Cannot add a command to a cli using options");
    }
    _mode = COMMANDS;

    const auto name = command->getName();
    if (_commands.contains(name)) {
        throw std::logic_error("Command '" + name + "' already exists");
    }

    CLI command_cli(name, command->getDescription());
    command->setup(command_cli);

    _commands.insert(std::make_pair(name, command));
    _commands_cli.insert(std::make_pair(name, command_cli));

    return *this;
}

auto CLI::run(const int argc, char **argv) const -> std::expected<CLIResults, Fault> {
    if (argc < 1) {
        return std::unexpected<Fault>({
          .message = "argc cannot be less than 1, argv should at least contains executable name",
          .type    = InvalidArgs,
        });
    }

    if (_mode.has_value() && _mode.value() == COMMANDS) {
        auto count     = argc - 1;
        auto arguments = argv + 1;
        if (count == 0) {
            auto results = CLIResults({});
            return results;
        }

        const std::string command_name(arguments[0]);
        if (! _commands.contains(command_name)) {
            return std::unexpected<Fault>({
              .message = "Command '" + command_name + "' not found",
              .type    = UnknownCommand,
            });
        }
        auto command       = _commands.at(command_name);
        auto cli           = _commands_cli.at(command_name);
        const auto results = cli.run(count, arguments);
        if (! results.has_value()) {
            return std::unexpected(results.error());
        }
        exit(command->run(results.value()));
    }

    std::vector<std::string> allowed_options;
    for (const auto &option : _options | std::ranges::views::values) {
        allowed_options.push_back(option->getName());
        allowed_options.push_back(option->getShortName());
    }
    const auto parse_results_expect = parseArgv(argc - 1, argv + 1, allowed_options);
    if (! parse_results_expect.has_value()) {
        return std::unexpected(parse_results_expect.error());
    }
    const auto &[raw_results, option_order, positional_arguments] = parse_results_expect.value();
    std::map<std::string, std::any> option_values;
    std::vector<std::string> missing_required;

    if (_positional_options.empty() && ! positional_arguments.empty()) {
        return std::unexpected<Fault>({
          .message = "Unrecognized option: " + positional_arguments[0],
          .type    = UnrecognizedOption,
        });
    }
    auto positional_index = 0;
    for (const auto &option_name : _positional_options) {
        if (positional_index == positional_arguments.size()) {
            break;
        }

        const auto &option      = _options.at(option_name);
        const auto &option_type = option->getType();

        if (option_type == typeid(std::vector<int>) || option_type == typeid(std::vector<float>)
            || option_type == typeid(std::vector<double>)) {
            auto values = std::vector(positional_arguments.begin() + positional_index, positional_arguments.end());
            auto value  = getValueForOption(option, values);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            option_values.insert(std::make_pair(option_name, value.value()));
            break;
        }

        auto value = getValueForOption(option, {positional_arguments[positional_index]});
        if (! value.has_value()) {
            return std::unexpected(value.error());
        }
        option_values.insert(std::make_pair(option_name, value.value()));

        positional_index++;
    }

    for (const auto &option : _options | std::ranges::views::values) {
        if (raw_results.contains(option->getName()) && raw_results.contains(option->getShortName())) {
            auto long_values_it  = raw_results.at(option->getName()).begin();
            auto short_values_it = raw_results.at(option->getShortName()).begin();
            std::vector<std::string> values;
            // Assert that long_values.length + short_values.length === option_orders of (-n, --name).
            // If it is not the case it means there is a bug in parseArgv and it should be fixed
            for (const auto &oo : option_order) {
                if (oo == option->getName()) {
                    values.push_back(*long_values_it++);
                } else if (oo == option->getShortName()) {
                    values.push_back(*short_values_it++);
                }
            }
            auto value = getValueForOption(option, values);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            option_values.insert(std::make_pair(option->getName(), value.value()));
        }

        else if (raw_results.contains(option->getName())) {
            auto values = raw_results.at(option->getName());
            auto value  = getValueForOption(option, values);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            option_values.insert(std::make_pair(option->getName(), value.value()));
        }

        else if (raw_results.contains(option->getShortName())) {
            auto values = raw_results.at(option->getShortName());
            auto value  = getValueForOption(option, values);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            option_values.insert(std::make_pair(option->getName(), value.value()));
        }

        else if (option->getConfiguration().default_value.has_value()) {
            option_values.insert(std::make_pair(option->getName(), option->getConfiguration().default_value.value()));
        }

        else if (option->getConfiguration().required && ! option_values.contains(option->getName())) {
            missing_required.push_back(option->getName());
        }
    }

    if (! missing_required.empty()) {
        return std::unexpected<Fault>({
          .message = "Some required options were not given: " + join(missing_required, ", "),
          .type    = MissingRequiredOption,
        });
    }

    CLIResults results(option_values);
    return results;
}

template<typename T> auto toAny(std::expected<T, Fault> exp) -> std::expected<std::any, Fault> {
    return exp.transform([](const auto &value) {
        return std::any(value);
    });
}

auto CLI::getValueForOption(const std::shared_ptr<Option> &option, const std::vector<std::string> &values)
    -> std::expected<std::any, Fault> {
    const auto last_index = values.size() - 1;
    if (option->getType() == typeid(bool)) {
        return toAny(toBoolean(values[last_index]));
    }

    if (values.size() == 1 && values[0] == "true") {
        if (option->getConfiguration().implicit_value.has_value()) {
            return option->getConfiguration().implicit_value.value();
        } else {
            return std::unexpected<Fault>({
              .message = "Option '" + option->getName() + "' needs a value",
              .type    = MissingOptionValue,
            });
        }
    }

    if (option->getType() == typeid(std::string)) {
        return values[last_index];
    }
    if (option->getType() == typeid(int)) {
        return toAny(toInt(values[last_index]));
    }
    if (option->getType() == typeid(float)) {
        return toAny(toFloat(values[last_index]));
    }
    if (option->getType() == typeid(double)) {
        return toAny(toDouble(values[last_index]));
    }
    if (option->getType() == typeid(std::vector<bool>)) {
        std::vector<bool> bool_results;
        bool_results.reserve(values.size());
        for (const auto &string_value : values) {
            const auto value = toBoolean(string_value);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            bool_results.push_back(value.value());
        }
        return bool_results;
    }
    if (option->getType() == typeid(std::vector<std::string>)) {
        return values;
    }
    if (option->getType() == typeid(std::vector<int>)) {
        std::vector<int> int_results;
        int_results.reserve(values.size());
        for (const auto &string_value : values) {
            const auto value = toInt(string_value);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            int_results.push_back(value.value());
        }
        return int_results;
    }
    if (option->getType() == typeid(std::vector<float>)) {
        std::vector<float> float_results;
        float_results.reserve(values.size());
        for (const auto &string_value : values) {
            const auto value = toFloat(string_value);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            float_results.push_back(value.value());
        }
        return float_results;
    }
    if (option->getType() == typeid(std::vector<double>)) {
        std::vector<double> double_results;
        double_results.reserve(values.size());
        for (const auto &string_value : values) {
            const auto value = toDouble(string_value);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            double_results.push_back(value.value());
        }
        return double_results;
    }

    throw std::runtime_error(
        "Type '" + std::string(option->getType().name()) + "' is not allowed. It should have been caught before"
    );
}

auto CLI::help(std::ostream &out) const -> void {
    out << "usage:\n"
        << "\t" << buildUsageHelp() << "\n"
        << "\n"
        << _description << "\n"
        << "\n";

    if (_mode.has_value() && _mode.value() == COMMANDS) {
        out << "Commands:\n"
            << "\n";

        for (const auto &command : _commands | std::ranges::views::values) {
            out << "\t" << _commands_cli.at(command->getName()).buildUsageHelp() << "\n";
            if (! command->getDescription().empty()) {
                out << "\t\t" << join(split(command->getDescription(), "\n"), "\n\t\t") << "\n";
            }
            out << "\n";
        }
    } else {
        out << buildPositionalHelp();

        for (const auto &[name, group] : _groups) {
            if (group._options.empty()) {
                continue;
            }

            out << (name.empty() ? "Options" : name) << ":\n"
                << "\n";

            for (const auto &option : group._options) {
                out << "\t" << buildOptionUsageHelp(option) << "\n"
                    << "\t\t" << join(split(option->getDescription(), "\n"), "\n\t\t") << "\n"
                    << "\n";
            }
        }
    }
}

auto CLI::buildUsageHelp() const -> std::string {
    auto usage = _name;

    if (_mode.has_value() && _mode.value() == COMMANDS) {
        usage += " [COMMAND] [OPTIONS]";
    } else {
        if (! _options.empty()) {
            usage += " [OPTIONS]";
        }
        for (const auto &option : _options | std::ranges::views::values) {
            if (option->getConfiguration().required && ! inArray(_positional_options, option->getName())) {
                usage += " --" + option->getName();
            }
        }
        for (const auto &option : _positional_options) {
            if (_options.at(option)->getConfiguration().required) {
                usage += " " + toUpper(option);
            } else {
                usage += " [" + toUpper(option) + "]";
            }
        }
    }

    return usage;
}

auto CLI::buildPositionalHelp() const -> std::string {
    if (_positional_options.empty()) {
        return "";
    }

    std::string help
        = "Positional arguments:\n"
          "\n"
          "\tThese arguments come after options and in the order they are listed here.\n";

    if (_options.at(_positional_options[0])->getConfiguration().required) {
        help += "\tOnly ";
        std::vector<std::string> required;
        for (const auto &option : _positional_options) {
            if (_options.at(option)->getConfiguration().required) {
                required.push_back(toUpper(option));
            } else {
                break;
            }
        }
        help += join(required, ", ");
        if (required.size() == 1) {
            help += " is required.\n\n";
        } else {
            help += " are required.\n\n";
        }
    }

    for (const auto &option_name : _positional_options) {
        help += "\t" + toUpper(option_name);
        const auto option = _options.at(option_name);
        if (option->getConfiguration().required) {
            help += " [REQUIRED]";
        }
        help += "\n\t\t";
        help += join(split(option->getDescription(), "\n"), "\n\t\t") + "\n\n";
    }

    return help;
}

auto CLI::buildOptionUsageHelp(const std::shared_ptr<Option> &option) -> std::string {
    auto usage = "--" + option->getName();
    if (option->getType() != typeid(bool)) {
        usage += " " + option->getConfiguration().value_help;
    }
    if (! option->getShortName().empty()) {
        usage += ", -" + option->getShortName();
        if (option->getType() != typeid(bool)) {
            usage += " " + option->getConfiguration().value_help;
        }
    }
    if (option->getConfiguration().required) {
        usage += " [REQUIRED]";
    }

    return usage;
}

auto CLI::checkOptionType(const std::type_info &type) -> void {
    if (type != typeid(bool) && type != typeid(std::string) && type != typeid(int) && type != typeid(float)
        && type != typeid(double) && type != typeid(std::vector<bool>) && type != typeid(std::vector<std::string>)
        && type != typeid(std::vector<int>) && type != typeid(std::vector<float>)
        && type != typeid(std::vector<double>)) {
        throw std::logic_error("Type '" + std::string(type.name()) + "' is not allowed for options");
    }
}
