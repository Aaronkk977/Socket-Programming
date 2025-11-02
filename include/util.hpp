#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace util {

// 移除行尾 '\r' '\n'
void chomp(std::string& s);

// 以空白切詞（不支援引號；Phase 2 再擴充）
std::vector<std::string> tokenize(const std::string& s);

// 解析埠號，合法回 true 並輸出 port
bool parse_port(const std::string& s, uint16_t& port,
                int min_port = 1024, int max_port = 65535);

// 簡單 log（之後可換成來源/等級）
void log_info(const std::string& msg);
void log_warn(const std::string& msg);
void log_err (const std::string& msg);

} // namespace util
