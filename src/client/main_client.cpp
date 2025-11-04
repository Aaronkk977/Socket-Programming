#include "cli_shell.hpp"
#include "util.hpp"
#include "proto.hpp"
#include "net.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int connect_to_server(const std::string& ip, uint16_t port) {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::fprintf(stderr, "socket() failed: %s\n", std::strerror(errno));
        return -1;
    }
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = ::htons(port);
    if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1) {
        std::fprintf(stderr, "invalid ip: %s\n", ip.c_str());
        ::close(fd);
        return -1;
    }
    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        std::fprintf(stderr, "connect() failed: %s\n", std::strerror(errno));
        ::close(fd);
        return -1;
    }
    return fd;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1;
    }
    std::string ip = argv[1];
    uint16_t port = 0;
    if (!util::parse_port(argv[2], port, proto::kMinPort, proto::kMaxPort)) {
        std::fprintf(stderr, "invalid port: %s (range %d-%d)\n",
                     argv[2], proto::kMinPort, proto::kMaxPort);
        return 1;
    }

    int fd = connect_to_server(ip, port);
    if (fd < 0) return 2;

    run_cli_shell(fd);

    ::close(fd);
    return 0;
}
