// src/server/main_server.cpp
#include "server_core.hpp"
#include "state.hpp"
#include "net.hpp"
#include "util.hpp"
#include "proto.hpp"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// ---- 前置宣告（與先前約定相同） ----
int  setup_listen_socket(uint16_t port);
void accept_new_conn(int listen_fd);
void handle_readable_fd(int fd);

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "Usage: %s <bind_port>\n", argv[0]);
        return 1;
    }
    uint16_t port = 0;
    {
        std::string ps = argv[1];
        if (!util::parse_port(ps, port, proto::kMinPort, proto::kMaxPort)) {
            std::fprintf(stderr, "Invalid port: %s (range %d-%d)\n",
                         ps.c_str(), proto::kMinPort, proto::kMaxPort);
            return 1;
        }
    }

    server_state_init();
    int listen_fd = setup_listen_socket(port);
    if (listen_fd < 0) {
        std::fprintf(stderr, "Failed to setup listen socket on %u\n", port);
        return 1;
    }
    util::log_info("server listening on port " + std::to_string(port));

    // ---- select() event loop ----
    while (true) {
        fd_set readfds;
        FD_ZERO(&readfds);

        int maxfd = listen_fd;
        FD_SET(listen_fd, &readfds);

        // 把所有連線 fd 放進去
        for (const auto& kv : conns) {
            FD_SET(kv.first, &readfds);
            if (kv.first > maxfd) maxfd = kv.first;
        }

        int nready = ::select(maxfd + 1, &readfds, nullptr, nullptr, nullptr);
        if (nready < 0) {
            if (errno == EINTR) continue; // 被訊號打斷就重來
            util::log_err("select error: " + std::string(std::strerror(errno)));
            break;
        }

        // 新連線
        if (FD_ISSET(listen_fd, &readfds)) {
            accept_new_conn(listen_fd);
            if (--nready <= 0) continue;
        }

        // 既有連線的可讀事件
        // 需要小心遍歷時 close_conn 會改動 conns，因此複製一份 keys 再處理
        std::vector<int> fds;
        fds.reserve(conns.size());
        for (const auto& kv : conns) fds.push_back(kv.first);

        for (int fd : fds) {
            if (FD_ISSET(fd, &readfds)) {
                handle_readable_fd(fd);
                if (--nready <= 0) break;
            }
        }
    }

    ::close(listen_fd);
    return 0;
}

int setup_listen_socket(uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        util::log_err("socket() failed: " + std::string(std::strerror(errno)));
        return -1;
    }

    int yes = 1;
    ::setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
    addr.sin_port        = ::htons(port);

    if (::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        util::log_err("bind() failed: " + std::string(std::strerror(errno)));
        ::close(fd);
        return -1;
    }
    if (::listen(fd, 128) < 0) {
        util::log_err("listen() failed: " + std::string(std::strerror(errno)));
        ::close(fd);
        return -1;
    }
    return fd;
}

void accept_new_conn(int listen_fd) {
    sockaddr_in cli{};
    socklen_t   len = sizeof(cli);
    int cfd = ::accept(listen_fd, reinterpret_cast<sockaddr*>(&cli), &len);
    if (cfd < 0) {
        util::log_warn("accept() failed: " + std::string(std::strerror(errno)));
        return;
    }

    Conn c{};
    c.fd   = cfd;
    c.ip   = net::peer_ip(cfd);
    c.alive = true;

    conns[cfd] = std::move(c);
    util::log_info("new connection fd=" + std::to_string(cfd) + " from " + conns[cfd].ip);
}

void handle_readable_fd(int fd) {
    auto it = conns.find(fd);
    if (it == conns.end()) return;

    std::string line;
    bool got = net::readline_into(it->second.rbuf, fd, line);
    if (got) {
        util::chomp(line);               // 保險：去除尾端 \r\n
        if (!line.empty() && line.size() <= proto::kMaxLine) {
            handle_command(fd, line);    // 交給核心邏輯
        } else {
            // 太長或空行，視為錯誤請求
            send_err(fd, proto::ERR_BAD_REQUEST, "empty_or_too_long");
        }
        return;
    }

    // 沒拿到一整行：檢查是否斷線或錯誤
    if (errno == EPIPE) {
        util::log_info("peer closed (fd=" + std::to_string(fd) + ")");
        close_conn(fd);
        return;
    }
    if (errno == EMSGSIZE) { // rbuf 過長（可能攻擊）
        util::log_warn("buffer too large, closing fd=" + std::to_string(fd));
        close_conn(fd);
        return;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR || errno == 0) {
        // 暫時無事可做
        return;
    }

    // 其他錯誤：關閉
    util::log_warn("recv error on fd=" + std::to_string(fd) + ": " + std::string(std::strerror(errno)));
    close_conn(fd);
}
