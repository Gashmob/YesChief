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
#ifndef TEST_TOOLS_H
#define TEST_TOOLS_H

#include <string>
#include <utility>
#include <vector>
#include <yeschief.h>

inline auto toStringArray(const std::vector<std::string> &data) -> std::vector<char *> {
    std::vector<char *> strings;
    strings.reserve(data.size());
    for (auto &item : data) {
        strings.push_back(const_cast<char *>(item.c_str()));
    }

    return strings;
}

class CommandStub final : public yeschief::Command {
  public:
    explicit CommandStub(std::string name): _name(std::move(name)) {}

    [[nodiscard]] auto getName() const -> std::string override {
        return _name;
    }

    [[nodiscard]] auto getDescription() const -> std::string override {
        return "Stub class for Command.\nDescription on another line.";
    }

    auto setup(yeschief::CLI &cli) -> void override {
        cli.addOption<int>("exit", "Exit code of command");
    }

    auto run(const yeschief::CLIResults &results) -> int override {
        const auto exit_code = std::any_cast<int>(results.get("exit").value_or(0));

        return exit_code;
    }

  private:
    std::string _name;
};

#endif // TEST_TOOLS_H
