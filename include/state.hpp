#pragma once
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>

struct Conn {
    int         fd;
    std::string ip;            // TCP 對端 IP
    std::string rbuf;          // 行緩衝
    std::string user;          // 已登入使用者（未登入為空字串）
    bool        alive = true;
};

struct SessionInfo {
    std::string ip;            // 用於 LIST 顯示
    uint16_t    listen_port;   // Client 在 LOGIN 時上報的 P2P 監聽埠（Phase 2 會用）
    int         fd;            // 與 server 的 TCP fd
};

// 伺服器全域狀態（先用 extern，在 server_core.cpp 定義）
extern std::unordered_set<std::string> registered_users;   // 已註冊
extern std::unordered_map<std::string, SessionInfo> online;// 線上（user -> session）
extern std::unordered_map<int, Conn> conns;                // fd -> 連線

// 清理連線（關閉 fd、移除 conns、必要時從 online 下線）
void close_conn(int fd);

