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
    _groups.emplace("", OptionGroup(this, ""));
}

auto CLI::addGroup(const std::string &name) -> OptionGroup & {
    assert_message(
        ! _mode.has_value() || _mode.value() != Mode::COMMANDS, "Cannot add an option group to a cli using commands"
    );
    _mode = Mode::OPTIONS;

    assert_message(! _groups.contains(name), "Group already exists");

    return _groups.emplace(name, OptionGroup(this, name)).first->second;
}

auto CLI::addCommand(Command *command) -> CLI & {
    assert_message(
        ! _mode.has_value() || _mode.value() != Mode::OPTIONS, "Cannot add a command to a cli using options"
    );
    _mode = Mode::COMMANDS;

    const auto name = command->getName();
    assert_message(! _commands.contains(name), "Command already exists");

    CLI command_cli(name, command->getDescription());
    command->setup(command_cli);

    _commands.emplace(name, command);
    _commands_cli.emplace(name, command_cli);

    return *this;
}

auto CLI::run(const int argc, char **argv) const -> std::expected<CLIResults, Fault> {
    if (argc < 1) {
        return std::unexpected<Fault>({
          .message = "argc cannot be less than 1, argv should at least contains executable name",
          .type    = FaultType::InvalidArgs,
        });
    }

    if (_mode.has_value() && _mode.value() == Mode::COMMANDS) {
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
              .type    = FaultType::UnknownCommand,
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
        allowed_options.push_back(option->name);
        allowed_options.push_back(option->short_name);
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
          .type    = FaultType::UnrecognizedOption,
        });
    }
    size_t positional_index = 0;
    for (const auto &option_name : _positional_options) {
        if (positional_index == positional_arguments.size()) {
            break;
        }

        const auto option       = _options.at(option_name);
        const auto &option_type = option->type;

        if (option_type == typeid(std::vector<int>) || option_type == typeid(std::vector<float>)
            || option_type == typeid(std::vector<double>)) {
            auto values = std::vector(positional_arguments.begin() + positional_index, positional_arguments.end());
            auto value  = getValueForOption(option, values);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            option_values.emplace(option_name, value.value());
            break;
        }

        auto value = getValueForOption(option, {positional_arguments[positional_index]});
        if (! value.has_value()) {
            return std::unexpected(value.error());
        }
        option_values.emplace(option_name, value.value());

        positional_index++;
    }

    for (const auto &option : _options | std::ranges::views::values) {
        const auto long_raw_values  = raw_results.find(option->name);
        const auto short_raw_values = raw_results.find(option->short_name);
        if (long_raw_values != raw_results.end() && short_raw_values != raw_results.end()) {
            auto long_values_it  = long_raw_values->second.begin();
            auto short_values_it = short_raw_values->second.begin();
            std::vector<std::string> values;
            // Assert that long_values.length + short_values.length === option_orders of (-n, --name).
            // If it is not the case it means there is a bug in parseArgv and it should be fixed
            for (const auto &oo : option_order) {
                if (oo == option->name) {
                    values.push_back(*long_values_it++);
                } else if (oo == option->short_name) {
                    values.push_back(*short_values_it++);
                }
            }
            auto value = getValueForOption(option, values);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            option_values.emplace(option->name, value.value());
        }

        else if (long_raw_values != raw_results.end()) {
            auto values = raw_results.at(option->name);
            auto value  = getValueForOption(option, values);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            option_values.emplace(option->name, value.value());
        }

        else if (short_raw_values != raw_results.end()) {
            auto values = raw_results.at(option->short_name);
            auto value  = getValueForOption(option, values);
            if (! value.has_value()) {
                return std::unexpected(value.error());
            }
            option_values.emplace(option->name, value.value());
        }

        else if (option->configuration.default_value.has_value()) {
            option_values.emplace(option->name, option->configuration.default_value.value());
        }

        else if (option->configuration.required && ! option_values.contains(option->name)) {
            missing_required.push_back(option->name);
        }
    }

    if (! missing_required.empty()) {
        return std::unexpected<Fault>({
          .message = "Some required options were not given: " + join(missing_required, ", "),
          .type    = FaultType::MissingRequiredOption,
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

auto CLI::getValueForOption(const std::shared_ptr<const Option> &option, const std::vector<std::string> &values)
    -> std::expected<std::any, Fault> {
    const auto last_index = values.size() - 1;
    if (option->type == typeid(bool)) {
        return toAny(toBoolean(values[last_index]));
    }

    if (values.size() == 1 && values[0] == "true") {
        if (option->configuration.implicit_value.has_value()) {
            return option->configuration.implicit_value.value();
        } else {
            return std::unexpected<Fault>({
              .message = "Option '" + option->name + "' needs a value",
              .type    = FaultType::MissingOptionValue,
            });
        }
    }

    if (option->type == typeid(std::string)) {
        return values[last_index];
    }
    if (option->type == typeid(int)) {
        return toAny(toInt(values[last_index]));
    }
    if (option->type == typeid(float)) {
        return toAny(toFloat(values[last_index]));
    }
    if (option->type == typeid(double)) {
        return toAny(toDouble(values[last_index]));
    }
    if (option->type == typeid(std::vector<bool>)) {
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
    if (option->type == typeid(std::vector<std::string>)) {
        return values;
    }
    if (option->type == typeid(std::vector<int>)) {
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
    if (option->type == typeid(std::vector<float>)) {
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
    if (option->type == typeid(std::vector<double>)) {
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
        "Type '" + std::string(option->type.name()) + "' is not allowed. It should have been caught before"
    );
}

auto CLI::help(std::ostream &out) const -> void {
    out << std::format(
        R"(usage:
  {}

{}

)",
        buildUsageHelp(),
        _description
    );

    if (_mode.has_value() && _mode.value() == Mode::COMMANDS) {
        out << "Commands:\n"
            << "\n";

        for (const auto &command : _commands | std::ranges::views::values) {
            out << "  " << _commands_cli.at(command->getName()).buildUsageHelp() << "\n";
            if (! command->getDescription().empty()) {
                out << "    " << join(split(command->getDescription(), "\n"), "\n    ") << "\n";
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
                out << "  " << buildOptionUsageHelp(option) << "\n"
                    << "    " << join(split(option->description, "\n"), "\n    ") << "\n"
                    << "\n";
            }
        }
    }
}

auto CLI::buildUsageHelp() const -> std::string {
    auto usage = _name;

    if (_mode.has_value() && _mode.value() == Mode::COMMANDS) {
        usage += " [COMMAND] [OPTIONS]";
    } else {
        if (! _options.empty()) {
            usage += " [OPTIONS]";
        }
        for (const auto &option : _options | std::ranges::views::values) {
            if (option->configuration.required && ! inArray(_positional_options, option->name)) {
                usage += " --" + option->name;
            }
        }
        for (const auto &option : _positional_options) {
            if (_options.at(option)->configuration.required) {
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
          "  These arguments come after options and in the order they are listed here.\n";

    if (_options.at(_positional_options[0])->configuration.required) {
        help += "  Only ";
        std::vector<std::string> required;
        for (const auto &option : _positional_options) {
            if (_options.at(option)->configuration.required) {
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
        help += "  " + toUpper(option_name);
        const auto option = _options.at(option_name);
        if (option->configuration.required) {
            help += " [REQUIRED]";
        }
        help += "\n    ";
        help += join(split(option->description, "\n"), "\n    ") + "\n\n";
    }

    return help;
}

auto CLI::buildOptionUsageHelp(const std::shared_ptr<const Option> &option) -> std::string {
    auto usage = "--" + option->name;
    if (option->type != typeid(bool)) {
        usage += " " + option->configuration.value_help;
    }
    if (! option->short_name.empty()) {
        usage += ", -" + option->short_name;
        if (option->type != typeid(bool)) {
            usage += " " + option->configuration.value_help;
        }
    }
    if (option->configuration.required) {
        usage += " [REQUIRED]";
    }

    return usage;
}

auto CLI::checkOptionType(const std::type_info &type) -> void {
    assert_message(
        type == typeid(bool) || type == typeid(std::string) || type == typeid(int) || type == typeid(float)
            || type == typeid(double) || type == typeid(std::vector<bool>) || type == typeid(std::vector<std::string>)
            || type == typeid(std::vector<int>) || type == typeid(std::vector<float>)
            || type == typeid(std::vector<double>),
        "Type is not allowed for options"
    );
}
