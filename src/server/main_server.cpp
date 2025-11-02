// 只放你會實作的 function 介面（main、事件迴圈的私有 helper）
int main(int argc, char** argv);

// 初始化 listen socket，回傳 fd（錯誤丟例外或回 -1 視你的風格）
int setup_listen_socket(uint16_t port);

// 處理新連線（accept 並加入 conns）
void accept_new_conn(int listen_fd);

// 處理既有 fd 可讀（讀取、切行、呼叫 handle_command；遇 EOF/錯誤要 close_conn）
void handle_readable_fd(int fd);
