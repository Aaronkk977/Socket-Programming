#include "net.hpp"
#include "proto.hpp"

#include <cerrno>
#include <cstring>
#include <string>
#include <algorithm>

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace {

// 從 rbuf 中切出第一個 '\n' 前的內容，去掉尾端的 '\r'
static bool cut_one_line(std::string& rbuf, std::string& out_line) {
    auto it = std::find(rbuf.begin(), rbuf.end(), '\n');
    if (it == rbuf.end()) return false;

    // [0, it) is one line, skip '\n'
    std::string one(rbuf.begin(), it);
    // remove '\r' at the end of the line
    if (!one.empty() && one.back() == '\r') one.pop_back();

    // remaining data is kept in rbuf
    rbuf.erase(rbuf.begin(), it + 1);
    out_line.swap(one); // = swap(out_line, one)
    return true;
}

} // anonymous

namespace net {

bool readline_into(std::string& rbuf, int fd, std::string& out_line) {
    // 1) if there is one line in the buffer, cut it out
    if (cut_one_line(rbuf, out_line)) return true;

    // 2) 從 socket 補讀一批資料（呼叫者應該在 select/poll 告知可讀時才進來）
    char buf[4096];
    ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
    if (n > 0) {
        rbuf.append(buf, static_cast<size_t>(n));

        // safety limit protection (avoid long line attack)
        if (rbuf.size() > proto::kMaxLine) {
            errno = EMSGSIZE;   // indicate abnormal data size to the caller
            return false;
        }

        // 2a) 補進來之後再嘗試切一行
        return cut_one_line(rbuf, out_line);
    }

    if (n == 0) {
        // 對端關閉連線（EOF）
        errno = EPIPE;
        return false;
    }

    // n < 0 : 出錯或被打斷
    if (errno == EINTR) {
        // 系統呼叫被訊號中斷，呼叫者稍後可重試
        return false;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        // 非阻塞且暫時無資料
        return false;
    }
    // 其他錯誤留給外層處理
    return false;
}

bool sendall(int fd, const void* data, size_t len) {
    const char* p = static_cast<const char*>(data);
    size_t left = len;
    while (left > 0) {
        ssize_t n = ::send(fd, p, left, 0);
        if (n > 0) {
            p    += n;
            left -= static_cast<size_t>(n);
            continue;
        }
        if (n == 0) {
            // send 回 0 幾乎不會發生；視同暫時不可寫
            errno = EAGAIN;
            return false;
        }
        // n < 0
        if (errno == EINTR) continue;                      // 重試
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 呼叫者應確保在可寫時再試；這裡直接回失敗
            return false;
        }
        return false; // 其他錯誤
    }
    return true;
}

bool send_line(int fd, const std::string& line_no_newline) {
    // 避免兩次系統呼叫，先組成一個含 '\n' 的緩衝
    std::string tmp;
    tmp.reserve(line_no_newline.size() + 1);
    tmp.append(line_no_newline);
    tmp.push_back('\n');
    return sendall(fd, tmp.data(), tmp.size());
}

std::string peer_ip(int fd) {
    sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    if (::getpeername(fd, reinterpret_cast<sockaddr*>(&ss), &slen) != 0) {
        return {};
    }
    char buf[INET6_ADDRSTRLEN] = {0};
    void* addr_ptr = nullptr;

    if (ss.ss_family == AF_INET) { // IPv4
        addr_ptr = &reinterpret_cast<sockaddr_in*>(&ss)->sin_addr;
    } else if (ss.ss_family == AF_INET6) { // IPv6
        addr_ptr = &reinterpret_cast<sockaddr_in6*>(&ss)->sin6_addr;
    } else {
        return {};
    }

    if (::inet_ntop(ss.ss_family, addr_ptr, buf, sizeof(buf)) == nullptr) { //network to presentation
        return {};
    }
    return std::string(buf);
}

bool set_nonblock(int fd) {
    int flags = ::fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) return false;
    return true;
}

} // namespace net
