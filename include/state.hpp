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

// server global state (use extern, define in server_core.cpp)
extern std::unordered_set<std::string> registered_users;   // registered users
extern std::unordered_map<std::string, SessionInfo> online_users;// online users (user -> session)
extern std::unordered_map<int, Conn> conns;                // fd -> connection

// clean up connection (close fd, remove conns, optionally remove from online)
void close_conn(int fd);

