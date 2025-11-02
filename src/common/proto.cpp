#include "proto.hpp"
#include <string>

namespace proto {

const char* const CMD_REGISTER = "REGISTER";
const char* const CMD_LOGIN = "LOGIN";
const char* const CMD_LOGOUT = "LOGOUT";
const char* const CMD_LIST = "LIST";

const char* const REPLY_OK = "OK";
const char* const REPLY_LIST_OK = "LIST-OK";

const char* const ERR_UNKNOWN_COMMAND = "unknown_command";
const char* const ERR_DUP_USERNAME = "duplicate_username";
const char* const ERR_ALREADY_LOGGED_IN = "already_logged_in";
const char* const ERR_INVALID_PORT = "invalid_port";
const char* const ERR_NOT_LOGGED_IN = "not_logged_in";
const char* const ERR_NO_SUCH_USER = "no_such_user";
const char* const ERR_BAD_REQUEST = "bad_request";

}

std::string make_ok() {
    return std::string(REPLY_OK) + "\n";
}

std::string make_err(const std::string& code, const std::string& detail) {
    if (detail.empty()) {
        return std::string("ERR ") + code + "\n";
    }

    return std::string("ERR ") + code + ":" + detail + "\n";
}   // "ERR code[: detail]\n"

std::string make_list_ok_header(size_t n) {
    return std::string(REPLY_LIST_OK) + " " + std::to_string(n) + "\n";
}   // "LIST-OK n\n"
