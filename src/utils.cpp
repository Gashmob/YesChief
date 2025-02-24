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
