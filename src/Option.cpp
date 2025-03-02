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

Option::Option(
    const std::string &name,
    const std::string &short_name,
    const std::string &description,
    const std::type_info &type,
    const OptionConfiguration &configuration
)
    : _name(name), _short_name(short_name), _description(description), _type(type), _configuration(configuration) {}

auto Option::getName() const -> std::string {
    return _name;
}

auto Option::getShortName() const -> std::string {
    return _short_name;
}

auto Option::getDescription() const -> std::string {
    return _description;
}

auto Option::getType() const -> const std::type_info & {
    return _type;
}

auto Option::getConfiguration() const -> OptionConfiguration {
    return _configuration;
}
