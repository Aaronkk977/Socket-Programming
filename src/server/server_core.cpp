#include "server_core.hpp"
#include "state.hpp"
#include "proto.hpp"
#include "util.hpp"
#include "net.hpp"

#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <string>
#include <cerrno>
#include <unistd.h>

using namespace std;

// === global state ===
unordered_set<string> registered_users;
unordered_map<string, SessionInfo> online_users;
unordered_map<int, Conn> conns;

// ------------------------------------------------------------

void server_state_init() {
    registered_users.clear();
    online_users.clear();
    conns.clear();
}

// ------------------------------------------------------------
// 基本傳輸封裝
void send_ok(int fd) {
    net::sendall(fd, proto::make_ok().data(), proto::make_ok().size());
}

void send_err(int fd, const string& code, const string& detail) {
    string msg = proto::make_err(code, detail);
    net::sendall(fd, msg.data(), msg.size());
}

// LIST-OK n + n lines
void send_list(int fd) {
    string out = proto::make_list_ok_header(online_users.size());
    for (auto& [user, sess] : online_users) {
        out += user + " " + sess.ip + " " + to_string(sess.listen_port) + "\n";
    }
    net::sendall(fd, out.data(), out.size());
}

// ------------------------------------------------------------
// close and clean up connection
void close_conn(int fd) {
    auto it = conns.find(fd);
    if (it == conns.end()) return;
    auto& c = it->second; // Conn object

    if (!c.user.empty()) {
        util::log_info("user " + c.user + " disconnected");
        online_users.erase(c.user);
    }

    ::close(fd);
    conns.erase(it);
}

// ------------------------------------------------------------
// parse and execute command
void handle_command(int fd, const string& line) {
    auto& c = conns[fd];
    auto toks = util::tokenize(line);
    if (toks.empty()) return;

    const string& cmd = toks[0]; // the 1st token is the command

    if (cmd == proto::CMD_REGISTER) { // register command
        if (toks.size() != 2) {
            send_err(fd, proto::ERR_BAD_REQUEST, "usage REGISTER <username>");
            return;
        }
        const string& user = toks[1];
        if (registered_users.count(user)) { // check if the user is already registered (in registered_users)
            send_err(fd, proto::ERR_DUP_USERNAME);
            return;
        }
        registered_users.insert(user);
        send_ok(fd);
        util::log_info("registered " + user);
        return;
    }

    if (cmd == proto::CMD_LOGIN) { // login command
        if (toks.size() != 3) {
            send_err(fd, proto::ERR_BAD_REQUEST, "usage LOGIN <username> <port>");
            return;
        }
        const string& user = toks[1];
        const string& ps   = toks[2];
        uint16_t p;
        if (!util::parse_port(ps, p)) {
            send_err(fd, proto::ERR_INVALID_PORT);
            return;
        }
        if (!registered_users.count(user)) {
            send_err(fd, proto::ERR_NO_SUCH_USER);
            return;
        }
        if (online_users.count(user)) {
            send_err(fd, proto::ERR_ALREADY_LOGGED_IN);
            return;
        }

        c.user = user;
        online_users[user] = SessionInfo{c.ip, p, fd};
        send_ok(fd);
        util::log_info(user + " logged in from " + c.ip);
        return;
    }

    if (cmd == proto::CMD_LOGOUT) { // logout command
        if (c.user.empty()) {
            send_err(fd, proto::ERR_NOT_LOGGED_IN);
            return;
        }
        const string user = c.user; // capture before clearing
        online_users.erase(c.user);
        c.user.clear();
        send_ok(fd);
        util::log_info("logout " + user);
        return;
    }

    if (cmd == proto::CMD_LIST) { // list command
        send_list(fd);
        return;
    }

    // 其他未知指令
    send_err(fd, proto::ERR_UNKNOWN_COMMAND);
}
