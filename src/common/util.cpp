#include "util.hpp"
#include "proto.hpp"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace util {

void chomp(std::string& s) { // remove \r and \n
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) {
        s.pop_back();
    }
}

std::vector<std::string> tokenize(const std::string& s) { // split by space
    std::stringstream iss(s);
    std::vector<std::string> out;
    
    std::string token;
    while(iss >> token) {
        out.push_back(token);
    }

    return out;
}

bool parse_port(const std::string& s, uint16_t& port,
    int min_port = 1024, int max_port = 65535){ // parse port, return true if valid (1024-65535 by default)
    
    // check if empty
    if (s.empty()) {
        return false;
    }
    // check if all digits
    for (char c : s) {
        if (!isdigit(c)) {
            return false;
        }
    }
    // convert to int
    long long val = 0;
    try {
        val = std::stoll(s);
    } catch (const std::invalid_argument& e) {
        return false;
    } catch (const std::out_of_range& e) {
        return false;
    }
    // check if in range
    if (val < min_port || val > max_port) {
        return false;
    }
    // convert to uint16_t
    port = static_cast<uint16_t>(val);
    return true;
}

// simple log (stderr output)
static inline void vlog(const char* level, const std::string& msg) {
    std::fprintf(stderr, "[%s] %s\n", level, msg.c_str());
}

void log_info(const std::string& msg) {
    vlog("INFO", msg);
}

void log_warn(const std::string& msg) {
    vlog("WARN", msg);
}

void log_err (const std::string& msg) {
    vlog("ERR", msg);
}

} // namespace util