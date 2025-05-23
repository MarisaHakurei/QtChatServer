//
// Created by Kotori on 2025/5/23.
//
#include "common.h"

//创建或打开数据库
SQLite::Database db("user.db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

std::map<int, int> user_map;

std::mutex map_mutex;

int main(int argc, char *argv[]) {
    /*创建好友关系、群组管理、用户、群组成员四张表*/
    db.exec(
        "CREATE TABLE IF NOT EXISTS friend (user1 INTEGER NOT NULL, user2 INTEGER NOT NULL, PRIMARY KEY (user1, user2));");
    db.exec(
        "CREATE TABLE IF NOT EXISTS group_table (group_account INTEGER PRIMARY KEY, group_name TEXT, create_time DATETIME, group_master INTEGER);");
    db.exec(
        "CREATE TABLE IF NOT EXISTS user (account INTEGER PRIMARY KEY AUTOINCREMENT, password VARCHAR(32), name VARCHAR(32), signature TEXT, online INT DEFAULT 0 NOT NULL, icon TEXT);");
    db.exec(
        "CREATE TABLE IF NOT EXISTS member (member_id INTEGER, group_account INTEGER, group_nickname TEXT);");

    /*端口号设置*/
    int default_port = 8888;
    int opt = 0;
    // 使用getopt解析命令行参数，optind是getopt处理的参数索引，初始化为1
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
            case 'p': // 指定端口号
                default_port = atoi(optarg);
                LOGINFO("port:%s\n", optarg);
                break;
            default: // 未知选项
                std::cerr << "Usage: " << argv[0] << " [-p port]" << std::endl;
                return RET_ERROR;
        }
    }

    /*声明服务端地址和客户端链接地址*/
    sockaddr_in server_addr{}, client_addr{};
    socklen_t client_len;

    /*初始化监听socket*/
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) {
        perror("Socket Error");
        return 0;
    }

    /*设置服务器地址结构*/
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = default_port;

    /*允许socket立即重用*/
    int val = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    /*绑定socket和端口*/
    if (bind(listen_fd, reinterpret_cast<const sockaddr *>(&server_addr), sizeof(server_addr)) == -1) {
        perror("Bind Error");
        return 0;
    }

    /*监听客户端请求,最多4条请求等待*/
    if (listen(listen_fd, 4) == -1) {
        perror("Listen Error");
        return 0;
    }

    /*接受客户端请求*/
    std::vector<std::thread> thread_pool;
    int connect_fd;
    while (true) {
        client_len = sizeof(client_addr);
        connect_fd = accept(listen_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_len);
        if (connect_fd < 0) {
            perror("Connect Error");
            return 0;
        }
        LOGINFO("Connect from %s:%hu...\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        std::lock_guard lock(map_mutex);
        user_map[connect_fd] = connect_fd;

        //处理客户端请求，未实现
        thread_pool.emplace_back();
    }
    close(listen_fd);
    return 0;
}
