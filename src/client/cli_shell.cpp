#pragma once
#include <string>
#include <cstdint>

// 啟動互動式 shell，阻塞直到使用者 quit。
// 參數：server_fd 已連線的 TCP fd。
void run_cli_shell(int server_fd);

// 將本地命令（如 "login alice 5001"）轉換為協定行字串（含語法/port 檢查）。
// 合法回 true 並輸出 line_no_newline（不含 \n）；錯誤回 false 並輸出錯誤訊息到 err_msg。
bool build_request_line(const std::string& user_input,
                        std::string& line_no_newline,
                        std::string& err_msg);

// （可選）顯示用：將 server 回覆字串做友善格式化印出
void pretty_print_server_reply(const std::string& line);

