#pragma once
#include <string>

// 在 server 啟動時初始化全域狀態（必要時）
void server_state_init();

// 事件迴圈在收到一行指令後，呼叫此函式處理。
void handle_command(int fd, const std::string& line);

// 封裝回傳
void send_ok(int fd);                                   // "OK\n"
void send_err(int fd, const std::string& code,
              const std::string& detail = "");          // "ERR code[: detail]\n"

// LIST 回傳
void send_list(int fd);                                 // "LIST-OK n\n<rows>"

