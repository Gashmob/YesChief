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
#include <utility>

using namespace yeschief;

CLI::CLI(std::string name, std::string description): _name(std::move(name)), _description(std::move(description)) {
    _groups.insert(std::make_pair("", OptionGroup(this)));
}

auto CLI::addOption(const std::string &name, const std::string &description, const OptionConfiguration &configuration)
    -> CLI & {
    if (_options.contains(name)) {
        throw std::logic_error("CLI has already an option '" + name + "'");
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

    auto option = std::make_shared<Option>(long_name, short_name, description, configuration.required);
    _options.insert(std::make_pair(long_name, option));
    _groups.at("").addOption(option);

    return *this;
}

auto CLI::run(int argc, char **argv) const -> std::expected<CLIResults, Fault> {
    if (argc < 1) {
        return std::unexpected<Fault>({
          .message = "argc cannot be less than 1, argv should at least contains executable name",
          .type    = InvalidArgs,
        });
    }

    // Skip program name
    argc--;
    argv++;

    std::map<std::string, std::any> option_values;
    while (argc > 0) {
        const std::string argument(argv[0]);
        if (argument.starts_with("--")) {
            const auto long_name = argument.substr(2);
            if (! _options.contains(long_name)) {
                return std::unexpected<Fault>({
                  .message = "Unrecognized option: " + argument,
                  .type    = UnrecognizedOption,
                });
            }
            option_values[long_name] = true;
        } else if (argument.starts_with("-")) {
            const auto short_name = argument.substr(1);
            const auto find       = std::ranges::find_if(_options, [&short_name](const auto &option) {
                return ! option.second->getShortName().empty() && option.second->getShortName() == short_name;
            });
            if (find == _options.end()) {
                return std::unexpected<Fault>({
                  .message = "Unrecognized option: " + argument,
                  .type    = UnrecognizedOption,
                });
            }
            option_values[find->second->getName()] = true;
        } else {
            return std::unexpected<Fault>({
              .message = "Unrecognized option: " + argument,
              .type    = UnrecognizedOption,
            });
        }

        argc--;
        argv++;
    }

    std::vector<std::string> missing_required;
    for (const auto &option : _options | std::ranges::views::values) {
        if (option->isRequired() && ! option_values.contains(option->getName())) {
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

auto CLI::help(std::ostream &out) const -> void {
    out << "usage:\n"
        << "\t" << buildUsageHelp() << "\n"
        << "\n"
        << _description << "\n"
        << "\n";

    for (const auto &[name, group] : _groups) {
        if (group._options.empty()) {
            continue;
        }

        out << (name.empty() ? "Options" : name) << ":\n"
            << "\n";

        for (const auto &option : group._options) {
            out << "\t--" << option->getName();
            if (! option->getShortName().empty()) {
                out << ", -" << option->getShortName();
            }
            if (option->isRequired()) {
                out << " REQUIRED";
            }
            out << "\n"
                << "\t\t" << join(split(option->getDescription(), "\n"), "\n\t\t") << "\n";
        }
    }
}

auto CLI::buildUsageHelp() const -> std::string {
    auto usage = _name;

    if (! _options.empty()) {
        usage += " [OPTIONS]";
    }
    for (const auto &option : _options | std::ranges::views::values) {
        if (option->isRequired()) {
            usage += " --" + option->getName();
        }
    }

    return usage;
}
