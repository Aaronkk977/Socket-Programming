# Socket-Programming

## File Structure

```
cn2025-chat/
├── README.md
├── CMakeLists.txt
├── include/
│   ├── proto.hpp            # 訊息格式、常數、錯誤碼
│   ├── net.hpp              # socket/addr/IO 工具
│   ├── util.hpp             # 解析、字串、log、校驗
│   ├── crypto.hpp           # OpenSSL 包裝（Phase 2）
│   ├── thread_pool.hpp      # pthread worker pool（Phase 2）
│   └── state.hpp            # 使用者/群組/會話狀態資料結構
├── src/
│   ├── server/
│   │   ├── main_server.cpp   # 進入點：accept + 事件loop/分派
│   │   ├── server_core.cpp   # 解析請求、執行指令、寫回覆
│   │   ├── directory.cpp     # 使用者註冊/登入狀態表 & 查詢 (not implemented)
│   │   ├── group.cpp         # 群聊（relay）與群組key管理 (Phase 2)
│   │   ├── file_serv.cpp     # 檔案中繼（需要時）(Phase 2)
│   │   └── secure_srv.cpp    # 與 client 的握手/密鑰協議（Phase 2）
│   ├── client/
│   │   ├── main_client.cpp   # 進入點：CLI 互動、REPL
│   │   ├── cli_shell.cpp     # 指令解析、help/usage
│   │   ├── peer_chat.cpp     # P2P 連線、收發（雙執行緒或 select） (Phase 2)
│   │   ├── file_cli.cpp      # 檔案上傳/下載 (Phase 2)
│   │   └── secure_cli.cpp    # 與 server/peer 的握手/密鑰協議 (Phase 2)
│   └── common/
│       ├── proto.cpp
│       ├── net.cpp
│       ├── util.cpp
│       ├── crypto.cpp (Phase 2)
│       └── thread_pool.cpp (Phase 2)
└── scripts/
    ├── run_server.sh
    └── run_client.sh
```

## Build
```bash

mkdir -p build && cd build
cmake .. && make -j

```


## User Command

```bash
help # shows all commands

register <username> 

login <username> <listen_port>

logout

list  # list all logged in users: username address port_number

quit  # disconnects from the server
```

## Error Format
```bash
ERROR <code>[: detail]
```
- unknown_command

- duplicate_username

- already_logged_in

- invalid_port

----------------- not implemented ------------------

- not_logged_in

- no_such_user

- bad_request
