#pragma once
#include <string>
#include <cstddef>

// forward-declare sockaddr_in 用不到; 這裡只放 I/O 介面
namespace net {

// 讀 socket fd，將資料累積到 rbuf；若取到一行（去掉 '\n'）放到 out_line 並回 true。
// 回 false 表示目前尚未構成一整行；若對端關閉/出錯，請用 errno/讀取長度判斷於外層處理。
bool readline_into(std::string& rbuf, int fd, std::string& out_line);

// 一次把 data 全部送出（可能多次 send）。成功回 true，失敗回 false。
bool sendall(int fd, const void* data, size_t len);

// 文字便捷：自動附 '\n'
bool send_line(int fd, const std::string& line_no_newline);

// 取得對端 IP（點分字串）。失敗回空字串。
std::string peer_ip(int fd);

// （Phase 2 會用）設為 non-blocking；成功回 true。
bool set_nonblock(int fd);

} // namespace net
