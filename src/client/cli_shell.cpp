#include "cli_shell.hpp"
#include "proto.hpp"
#include "util.hpp"
#include "net.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <cstdio>
#include <cstring>

using namespace std;

bool build_request_line(const std::string& user_input,
                        std::string& line, std::string& err_msg) {
    auto toks = util::tokenize(user_input);
    if (toks.empty()) { err_msg = ""; return false; }

    const string& cmd = toks[0];

    if (cmd == "register") {
        if (toks.size() != 2) { err_msg = "usage: register <username>"; return false; }
        line = string(proto::CMD_REGISTER) + " " + toks[1];
        return true;

    } else if (cmd == "login") {
        if (toks.size() != 3) { err_msg = "usage: login <username> <listen_port>"; return false; }
        uint16_t p;
        if (!util::parse_port(toks[2], p, proto::kMinPort, proto::kMaxPort)) {
            err_msg = "invalid port (range " + to_string(proto::kMinPort) + "-" + to_string(proto::kMaxPort) + ")";
            return false;
        }
        line = string(proto::CMD_LOGIN) + " " + toks[1] + " " + to_string(p);
        return true;

    } else if (cmd == "logout") {
        if (toks.size() != 1) { err_msg = "usage: logout"; return false; }
        line = proto::CMD_LOGOUT;
        return true;

    } else if (cmd == "list") {
        if (toks.size() != 1) { err_msg = "usage: list"; return false; }
        line = proto::CMD_LIST;
        return true;

    } else if (cmd == "help") {
        std::cout
          << "commands:\n"
          << "  register <username>\n"
          << "  login <username> <listen_port>\n"
          << "  logout\n"
          << "  list\n"
          << "  help\n"
          << "  quit\n";
        err_msg.clear();
        return false;

    } else if (cmd == "quit" || cmd == "exit") {
        // 由 run_cli_shell 處理，這裡回 false 讓外層結束
        err_msg = "__QUIT__";
        return false;

    } else {
        err_msg = "unknown command. try: help";
        return false;
    }
}

void pretty_print_server_reply(const std::string& line) {
    if (line.rfind("OK", 0) == 0) {
        std::cout << "[OK]\n";
        return;
    }
    if (line.rfind("ERR ", 0) == 0) {
        std::cout << "[ERROR] " << line.substr(4) << "\n";
        return;
    }
    if (line.rfind("LIST-OK ", 0) == 0) {
        // 第一行：LIST-OK n
        std::cout << "[ONLINE]\n";
        // 其餘行在 run_cli_shell 會逐行收取並轉印
        return;
    }
    // 其他（未知類型）就原樣輸出
    std::cout << line << "\n";
}

void run_cli_shell(int server_fd) {
    std::string rbuf; // for readline_into()

    std::cout << "Type 'help' for commands. 'quit' to exit.\n";
    while (true) {
        std::cout << "> " << std::flush;
        std::string input;
        if (!std::getline(std::cin, input)) break;

        std::string req, err;
        if (!build_request_line(input, req, err)) {
            if (err == "__QUIT__") break;
            if (!err.empty()) std::cout << err << "\n";
            continue;
        }

        if (!net::send_line(server_fd, req)) {
            std::cout << "send failed: " << std::strerror(errno) << "\n";
            break;
        }

        // 同步等待一行回覆（LIST 會有多行；第一行先拿出來判斷數量）
        std::string line;
        while (true) {
            bool got = net::readline_into(rbuf, server_fd, line);
            if (!got) {
                if (errno == 0 || errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
                if (errno == EPIPE) {
                    std::cout << "server closed connection.\n";
                } else {
                    std::cout << "recv failed: " << std::strerror(errno) << "\n";
                }
                return;
            }
            break;
        }

        util::chomp(line);
        if (line.rfind("LIST-OK ", 0) == 0) {
            // 解析 n
            std::istringstream iss(line.substr(8));
            size_t n = 0; iss >> n;
            pretty_print_server_reply(line);
            for (size_t i = 0; i < n; ++i) {
                std::string row;
                while (true) {
                    bool got = net::readline_into(rbuf, server_fd, row);
                    if (!got) {
                        if (errno == 0 || errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
                        std::cout << "recv failed while LIST rows: " << std::strerror(errno) << "\n";
                        return;
                    }
                    break;
                }
                util::chomp(row);
                // 行格式：<user> <ip> <port>
                std::cout << "  " << row << "\n";
            }
        } else {
            pretty_print_server_reply(line);
        }
    }
}
