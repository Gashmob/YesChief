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
#ifndef UTILS_H
#define UTILS_H

#include "yeschief.h"

#include <expected>
#include <map>
#include <string>
#include <vector>

namespace yeschief {
auto join(const std::vector<std::string> &strings, const std::string &delimiter = "") -> std::string;

auto split(const std::string &str, const std::string &delimiter) -> std::vector<std::string>;

typedef struct {
    std::map<std::string, std::vector<std::string>> raw_results;
    std::vector<std::string> option_order;
} ArgvParsingResult;

auto parseArgv(int argc, char **argv, const std::vector<std::string> &allowed_options)
    -> std::expected<ArgvParsingResult, Fault>;

auto toBoolean(const std::string &value) -> std::expected<bool, Fault>;

auto toInt(const std::string &value) -> std::expected<int, Fault>;

auto toFloat(const std::string &value) -> std::expected<float, Fault>;

auto toDouble(const std::string &value) -> std::expected<double, Fault>;
} // namespace yeschief

#endif // UTILS_H
