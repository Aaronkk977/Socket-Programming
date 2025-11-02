#pragma once
#include <string>
#include <vector>

namespace proto {

// === 基本常數 ===
inline constexpr size_t kMaxLine = 4096;     // 一行最大長度
inline constexpr int    kMinPort = 1024;
inline constexpr int    kMaxPort = 65535;

// === 指令（Client->Server） ===
extern const char* const CMD_REGISTER; // "REGISTER"
extern const char* const CMD_LOGIN;    // "LOGIN"
extern const char* const CMD_LOGOUT;   // "LOGOUT"
extern const char* const CMD_LIST;     // "LIST"

// === 回覆前綴（Server->Client） ===
extern const char* const REPLY_OK;       // "OK"
extern const char* const REPLY_LIST_OK;  // "LIST-OK"

// === 錯誤碼（Server->Client, ERR <code>[: detail]） ===
extern const char* const ERR_UNKNOWN_COMMAND;     // "unknown_command"
extern const char* const ERR_DUP_USERNAME;        // "duplicate_username"
extern const char* const ERR_ALREADY_LOGGED_IN;   // "already_logged_in"
extern const char* const ERR_INVALID_PORT;        // "invalid_port"
extern const char* const ERR_NOT_LOGGED_IN;       // "not_logged_in"
extern const char* const ERR_NO_SUCH_USER;        // "no_such_user"
extern const char* const ERR_BAD_REQUEST;         // "bad_request"

// === 便利函式（產生統一格式的回覆字串；之後也可換成加密封包） ===
std::string make_ok();                                   // "OK\n"
std::string make_err(const std::string& code,
                     const std::string& detail = "");    // "ERR code[: detail]\n"
std::string make_list_ok_header(size_t n);               // "LIST-OK n\n"

} // namespace proto

