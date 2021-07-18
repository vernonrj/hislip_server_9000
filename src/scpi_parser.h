#pragma once
#include <iostream>
#include <functional>
#include <locale>
#include <optional>
#include <regex>
#include <string_view>

// already tired of writing split functions
void splitterator(std::string_view s, std::string_view delimiter, std::function<void(std::string_view)> on_elem_fn) {
    auto pos = s.begin();
    auto next = s.find(delimiter);
    for (; next != std::string::npos; next = s.find(delimiter, next+1)) {
        std::string_view elem = std::string_view(pos, s.begin() + next);
        on_elem_fn(elem);
        pos += next+1;
    }
    std::string_view last(pos, s.end());
    if (!last.empty()) {
        on_elem_fn(last);
    }
}

namespace scpi {
    struct Command {
        std::string header;
        std::vector<std::string> args;

        std::string norm_header() const {
            std::string normalized;
            splitterator(header, ":", [&normalized](std::string_view head) {
                std::string sh = std::string(head.substr(0, 4));
                for (auto& c: sh) {
                    c = std::toupper(c);
                }
                if (sh.size() == 4 && sh[3] == 'A' || sh[3] == 'E' || sh[3] == 'I' || sh[3] == 'O' || sh[3] == 'U') {
                    sh.pop_back();
                }
                if (sh.empty() || sh[0] != '*') {
                    normalized += ":";
                }
                normalized += sh;
                if (head.find('?') != std::string::npos) {
                    normalized.push_back('?');
                }
            });
            return normalized;
        }

        bool operator==(const Command&) const = default;
    };

    std::ostream& operator<<(std::ostream& os, const Command& cmd) {
        os << "Command(header=\"" << cmd.header << "\", args = [";
        bool first = true;
        for (const auto& a: cmd.args) {
            if (!first) {
                os << ", ";
            } else {
                first = false;
            }
            os << "\"" << a << "\"";
        }
        os << "])";
        return os;
    }

    std::optional<Command> parse(std::string data) {
        if (data.empty()) {
            return {};
        }

        std::regex cmd_re = std::regex((data[0] == '*')
            ? "(\\*\\w+\\\?\?)(?:\\s+(.*))\?"
            : "(:?(?:\\w+)(?::\\w+)*\\\?\?)(?:\\s+(.*))\?");
        std::smatch base_match;

        Command cmd;

        if (std::regex_match(data, base_match, cmd_re)) {
            const auto num_matches = base_match.size();
            cmd.header = base_match.str(1);
            if (num_matches > 1) {
                std::string args = base_match.str(2);
                auto pos = args.begin();
                auto next = args.find(" ");
                for (; next != std::string::npos; next = args.find(" ", next)) {
                    cmd.args.emplace_back(pos, pos + next);
                    pos += next;
                }
                std::string last_arg(pos, args.end());
                if (!last_arg.empty()) {
                    cmd.args.push_back(last_arg);
                }
            }
            return cmd;
        }
        return {};
    }
};