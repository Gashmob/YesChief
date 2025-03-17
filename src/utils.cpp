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

#include <regex>

auto yeschief::join(const std::vector<std::string> &strings, const std::string &delimiter) -> std::string {
    if (strings.empty()) {
        return "";
    }

    std::string result = *strings.begin();
    for (auto it = std::next(strings.begin()); it != strings.end(); ++it) {
        result += delimiter + *it;
    }

    return result;
}

auto yeschief::split(const std::string &str, const std::string &delimiter) -> std::vector<std::string> {
    const std::regex split_regex(delimiter);
    std::sregex_token_iterator iter(str.begin(), str.end(), split_regex, -1);
    const std::sregex_token_iterator end;
    std::vector<std::string> result;
    while (iter != end) {
        std::string current = *iter++;
        if (current.empty()) {
            continue;
        }
        result.push_back(current);
    }

    return result;
}

auto yeschief::inArray(const std::vector<std::string> &array, const std::string &needle) -> bool {
    return std::ranges::find(array, needle) != array.end();
}

auto yeschief::parseArgv(const int argc, char **argv, const std::vector<std::string> &allowed_options)
    -> std::expected<ArgvParsingResult, Fault> {
    auto count     = argc;
    auto arguments = argv;
    std::map<std::string, std::vector<std::string>> raw_results;
    std::vector<std::string> option_order;
    std::vector<std::string> positional_arguments;

    std::optional<std::string> current_option = std::nullopt;
    bool in_positional                        = false;
    while (count > 0) {
        const auto argument = std::string(arguments[0]);
        if (in_positional) {
            positional_arguments.push_back(argument);
        } else {
            std::smatch match;
            if (std::regex_match(argument, match, std::regex("^--([^=]+)=([^=]+)$"))
                || std::regex_match(argument, match, std::regex("^-([^=-])=([^=]+)$"))) {
                if (! positional_arguments.empty()) {
                    return std::unexpected<Fault>({
                      .message = "Unrecognized option: " + positional_arguments[0],
                      .type    = UnrecognizedOption,
                    });
                }

                if (current_option.has_value()) {
                    raw_results.at(current_option.value()).emplace_back("true");
                }

                const std::string option = match[1];
                if (! inArray(allowed_options, option)) {
                    return std::unexpected<Fault>({
                      .message = "Unrecognized option: " + option,
                      .type    = UnrecognizedOption,
                    });
                }
                if (! raw_results.contains(option)) {
                    raw_results.insert(std::make_pair(option, std::vector<std::string>()));
                }

                std::string value = match[2];
                if (std::smatch value_match; std::regex_match(value, value_match, std::regex("^(['\"])(.*)\\1$"))) {
                    value = value_match[2];
                }

                raw_results.at(option).push_back(value);
                option_order.push_back(option);
                current_option = std::nullopt;
            }

            else if (std::regex_match(argument, match, std::regex("^--([^=]+)$"))
                     || std::regex_match(argument, match, std::regex("^-([^=-])$"))) {
                if (! positional_arguments.empty()) {
                    return std::unexpected<Fault>({
                      .message = "Unrecognized option: " + positional_arguments[0],
                      .type    = UnrecognizedOption,
                    });
                }

                if (current_option.has_value()) {
                    raw_results.at(current_option.value()).emplace_back("true");
                }

                const std::string option = match[1];
                if (! inArray(allowed_options, option)) {
                    return std::unexpected<Fault>({
                      .message = "Unrecognized option: " + option,
                      .type    = UnrecognizedOption,
                    });
                }
                current_option = option;
                if (! raw_results.contains(option)) {
                    raw_results.insert(std::make_pair(option, std::vector<std::string>()));
                }
                option_order.push_back(option);
            }

            else if (std::regex_match(argument, match, std::regex("^-([^=-]+)(=[^=]+)?$"))) {
                if (! positional_arguments.empty()) {
                    return std::unexpected<Fault>({
                      .message = "Unrecognized option: " + positional_arguments[0],
                      .type    = UnrecognizedOption,
                    });
                }

                const std::string options = match[1];
                for (const char option_char : options) {
                    if (current_option.has_value()) {
                        raw_results.at(current_option.value()).emplace_back("true");
                    }

                    const auto option = std::string(1, option_char);
                    if (! inArray(allowed_options, option)) {
                        return std::unexpected<Fault>({
                          .message = "Unrecognized option: " + option,
                          .type    = UnrecognizedOption,
                        });
                    }
                    current_option = option;
                    if (! raw_results.contains(option)) {
                        raw_results.insert(std::make_pair(option, std::vector<std::string>()));
                    }
                    option_order.push_back(option);
                }

                if (match[2].matched) {
                    std::string value = match[2].str().substr(1);
                    if (std::smatch value_match; std::regex_match(value, value_match, std::regex("^(['\"])(.*)\\1$"))) {
                        value = value_match[2];
                    }
                    raw_results.at(current_option.value()).push_back(value);
                    current_option = std::nullopt;
                }
            }

            else if (argument == "--") {
                if (current_option.has_value()) {
                    raw_results.at(current_option.value()).emplace_back("true");
                    current_option = std::nullopt;
                }
                in_positional = true;
            }

            else {
                if (current_option.has_value()) {
                    raw_results.at(current_option.value()).push_back(argument);
                    current_option = std::nullopt;
                } else {
                    positional_arguments.push_back(argument);
                }
            }
        }

        count--;
        arguments++;
    }

    if (current_option.has_value()) {
        raw_results.at(current_option.value()).emplace_back("true");
    }

    ArgvParsingResult parse_result = {
      .raw_results          = raw_results,
      .option_order         = option_order,
      .positional_arguments = positional_arguments,
    };
    return parse_result;
}

auto yeschief::toBoolean(const std::string &value) -> std::expected<bool, Fault> {
    if (value == "true" || value == "1") {
        return true;
    }
    if (value == "false" || value == "0") {
        return false;
    }
    return std::unexpected<Fault>({
      .message = "'" + value + "' cannot be parsed to a boolean value",
      .type    = InvalidOptionType,
    });
}

auto yeschief::toInt(const std::string &value) -> std::expected<int, Fault> {
    if (std::smatch match; std::regex_match(value, match, std::regex("^[+-]?\\d+$"))) {
        return std::stoi(value);
    }
    return std::unexpected<Fault>({
      .message = "'" + value + "' cannot be parsed to an int value",
      .type    = InvalidOptionType,
    });
}

auto yeschief::toFloat(const std::string &value) -> std::expected<float, Fault> {
    if (std::smatch match; std::regex_match(value, match, std::regex("^[+-]?([0-9]*[.])?[0-9]+$"))) {
        return std::stof(value);
    }
    return std::unexpected<Fault>({
      .message = "'" + value + "' cannot be parsed to an int value",
      .type    = InvalidOptionType,
    });
}

auto yeschief::toDouble(const std::string &value) -> std::expected<double, Fault> {
    if (std::smatch match; std::regex_match(value, match, std::regex("^[+-]?([0-9]*[.])?[0-9]+$"))) {
        return std::stod(value);
    }
    return std::unexpected<Fault>({
      .message = "'" + value + "' cannot be parsed to an int value",
      .type    = InvalidOptionType,
    });
}

auto yeschief::toUpper(const std::string &str) -> std::string {
    std::string result;
    for (const auto c : str) {
        result += toupper(c);
    }
    return result;
}
