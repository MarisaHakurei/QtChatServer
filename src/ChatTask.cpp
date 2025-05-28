//
// Created by Kotori on 2025/5/28.
//

#include "ChatTask.h"

void taskThread(int client_fd) {
    try {
        printf("-------------------- new connect --------------------\n");
        Session session(client_fd);
        nlohmann::json json_msg;

        while (true) {
            json_msg = session.recvMsg();
            if (json_msg["cmd"] == cmd_logout) {
                std::cout << "用户" << std::to_string(client_fd) << "下线" << std::endl;
                if (user_map.size() == 1) {
                    user_map.erase(client_fd);
                    close(client_fd);
                    return;
                }
                if (user_map.find(client_fd) != user_map.end()) {
                    int account = user_map.at(client_fd);
                    SQLite::Statement query(db,"update user set online=0 where account=?");
                    query.bind(1,account);
                    query.exec();

                    nlohmann::json message;
                    message["account"] = std::to_string(account);
                    message["cmd"] = "logout";
                    
                    std::cout << "用户" << std::to_string(user_map.at(client_fd)) << "下线" << std::endl;
                    user_map.erase(client_fd);
                }
                close(client_fd);
                return;
            }
        }
    } catch (std::exception &e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
        close(client_fd);
        return ;
    }
}
