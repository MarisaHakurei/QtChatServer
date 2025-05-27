//
// Created by Kotori on 2025/5/23.
//

#include "Session.h"
#include "CommandHandler.h"
#include "nlohmann/json_fwd.hpp"

Session::Session(int socket): statement_(nullptr) {
    is_login_ = false;
    account_ = -1;
    socket_ = socket;
}

Session::~Session() {
    if (socket_ != -1) {
        close(socket_);
        socket_ = -1;
    }
    if (is_login_ == true) {
        //todo
        is_login_ = false;
    }
    //...
}

void Session::sendMsg(nlohmann::json &json_msg) {
    const std::string msg = json_msg.dump();
    const uint32_t len = static_cast<uint32_t>(msg.length());
    const uint32_t net_len = htonl(len);

    /*消息格式：消息长度(4B) + 消息内容*/
    std::vector<char> message(sizeof(net_len) + len);
    memcpy(message.data(), &net_len, sizeof(net_len));
    memcpy(message.data() + sizeof(net_len), msg.data(), len);

    ssize_t sent = send(socket_, message.data(), message.size(), 0);
    if (sent <= 0) {
        perror("send failed");
        return;
    }
    LOGINFO("send success, len : %d , msg : %s\n", len, message.data());
}

void Session::sendMsg(int fd, nlohmann::json &json_msg) {
    const std::string msg = json_msg.dump();
    const uint32_t len = static_cast<uint32_t>(msg.length());
    const uint32_t net_len = htonl(len);

    /*消息格式：消息长度(4B) + 消息内容*/
    std::vector<char> message(sizeof(net_len) + len);
    memcpy(message.data(), &net_len, sizeof(net_len));
    memcpy(message.data() + sizeof(net_len), msg.data(), len);

    ssize_t sent = send(fd, message.data(), message.size(), 0);
    if (sent <= 0) {
        perror("send failed");
        return;
    }
    LOGINFO("send success, len : %d , msg : %s\n", len, message.data());
}

void Session::sendMsg(const std::vector<int> &fds, nlohmann::json &json_msg) {
    const std::string msg = json_msg.dump();
    const uint32_t len = static_cast<uint32_t>(msg.length());
    const uint32_t net_len = htonl(len);

    /*消息格式：消息长度(4B) + 消息内容*/
    std::vector<char> message(sizeof(net_len) + len);
    memcpy(message.data(), &net_len, sizeof(net_len));
    memcpy(message.data() + sizeof(net_len), msg.data(), len);

    for (auto fd: fds) {
        ssize_t sent = send(socket_, message.data(), message.size(), 0);
        if (sent <= 0) {
            perror("send failed");
            LOGINFO("send failed, fd : %d\n", fd);
            return;
        }
    }
    LOGINFO("send success, len : %d , msg : %s\n", len, message.data());
}

nlohmann::json Session::recvMsg() {
    nlohmann::json json_msg;
    uint32_t len = 0;
    std::vector<char> buffer(4);
    //先接收前四字节，代表长度
    while (true) {
        ssize_t ret = recv(socket_, buffer.data(), 4, 0);
        if (ret > 0) {
            break;
        }

        if (errno == EWOULDBLOCK || errno == EAGAIN)
            continue;
        json_msg["cmd"] = cmd_logout;
        return json_msg;
    }

    memcpy(&len, buffer.data(), 4);
    len = ntohl(len);

    std::vector<char> message(len);
    ssize_t msg_ret = recv(socket_, message.data(), message.size(), MSG_WAITALL);
    if (msg_ret != len) {
        perror("len != msg len");
    }
    LOGINFO("recv len : %lu, message : %s\n", msg_ret, message.data());

    try {
        json_msg = nlohmann::json::parse(message.data());
    } catch (nlohmann::json_abi_v3_11_2::detail::parse_error &e) {
        std::cerr << "Failed to parse JSON: " << std::endl;
        std::cout << "error json msg : " << message.data() << std::endl;
    }
    handleMsg(json_msg);
    return json_msg;
}

int Session::handleMsg(nlohmann::json &json_msg) {
    usleep(1000);
    int cmd = json_msg.at("cmd");
    switch (cmd) {
        case cmd_regist: {
            //注册命令
            std::string account = json_msg.at("account");
            std::string password = json_msg.at("password");
            std::string name = json_msg.at("name");
            try {
                int a = std::stoi(account, nullptr, 0);
                CommandHandler::registerUser(a, password, name, this);
            } catch (std::invalid_argument &e) {
                printf(e.what());
                return 0;
            }
            break;
        }
        case cmd_login: {
            //登录命令
            std::string account = json_msg.at("account");
            std::string password = json_msg.at("password");
            try {
                int a = std::stoi(account, nullptr, 0);
                CommandHandler::login(a, password, this);
            } catch (std::invalid_argument &e) {
                printf(e.what());
                return 0;
            }
            break;
        }
        case cmd_friend_search: {
            //好友查找（并非好友）
            std::string search_info = json_msg.at("search_info");
            CommandHandler::searchFriend(search_info, this);
            break;
        }
        case cmd_add_friend_request: {
            //添加好友请求
            int sender = json_msg.at("sender"); //发送请求的用户本身
            int account = json_msg.at("account"); //被添加的用户
            std::string msg = json_msg.at("msg");
            UserInfo user_info = {
                sender, "", json_msg.at("name"), json_msg.at("sig"),
                1, json_msg.at("icon")
            };
            CommandHandler::addFriendRequest(account, user_info, msg, this);
            break;
        }
        case cmd_add_friend_response: {
            //添加好友请求响应
            int sender = json_msg.at("sender");
            int account = json_msg.at("account");
            int fd = getFriendFd(sender);
            std::string res;
            if (json_msg.at("reply") == "yes") {
                res = "同意";
                //添加到好友表中
                SQLite::Statement query(db, "insert into friend value(?,?)");
                query.bind(1, sender);
                query.bind(2, account);
                query.exec();
                //发送更新后的好友列表
                CommandHandler::searchFriendList(sender, this);
                CommandHandler::searchFriendList(account, this);
            } else {
                res = "拒绝";
            }
            if (fd > 0) {
                //系统发送回执
                std::string system_msg = std::to_string(account) + res + "了你的好友请求\n";
                sendSystemMsg(fd, system_msg);
            }
            break;
        }
        case cmd_friend_list: {
            //获取好友列表
            usleep(400000);
            int account = json_msg.at("account");
            CommandHandler::searchFriendList(account, this);
            break;
        }
        case cmd_friend_chat: {
            //好友聊天
            int account = json_msg.at("account");
            int fd = getFriendFd(account);
            if (fd > 0) {
                sendMsg(fd, json_msg);
            } else {
                //todo
                //系统发送提示对方不在线
            }
            break;
        }
        case cmd_group_create: {
            //创建群聊
            int account = json_msg.at("account");
            int group_name = json_msg.at("group_name");
            //todo
            break;
        }
        case cmd_group_search: {
            //查询群聊
            std::string search_info = json_msg.at("search_info");
            CommandHandler::searchGroup(search_info, this);
            break;
        }
        case cmd_group_join_request: {
            //加群请求
            int group_account = json_msg.at("account");
            int sender = json_msg.at("sender");
            std::string group_name = json_msg.at("group_name");
            UserInfo info = {
                sender, "", json_msg.at("name"),
                json_msg.at("sig"), 1, json_msg.at("icon")
            };
            CommandHandler::addGroupRequest(group_account, group_name, info, json_msg.at("msg"), this);
            break;
        }
        case cmd_group_join_response: {
            std::string name = json_msg.at("name");
            int group_account = json_msg.at("account");
            int sender = json_msg.at("sender");
            int fd = getFriendFd(sender);
            //为什么这里要先发送响应给用户
            sendMsg(fd, json_msg);

            if (json_msg.at("reply") == "yes") {
                sendSystemMsg(fd, "群主同意了你的加群请求\n");
                SQLite::Statement query(db, "insert into member values(?,?,?)");
                query.bind(1, sender);
                query.bind(1, group_account);
                query.bind(3, name);
                query.exec();
                CommandHandler::searchGroupList(sender, this);
            }
            break;
        }
        case cmd_group_list: {
            int account = json_msg.at("account");
            CommandHandler::searchGroupList(account, this);
            break;
        }
        case cmd_group_chat: {
            int account = json_msg.at("account");
            std::vector<int> friend_list = getFriendList(account);
            std::vector<int> fds = getFriendListFd(friend_list);
            //发给群里除自己以外的所有人
            int self_fd = getSocket();
            auto it = find(fds.begin(), fds.end(), self_fd);
            if (it != fds.end()) {
                fds.erase(it);
                printf("skip self\n");
            }
            sendMsg(fds, json_msg);
            break;
        }
        case cmd_group_member_list: {
            int group_account = json_msg.at("account");
            CommandHandler::searchGroupMemberList(group_account, this);
            break;
        }
        case cmd_group_member_add: {
            int account = json_msg.at("account");
            std::string group_name = json_msg.at("group_name");
            //todo
            break;
        }
        case cmd_group_member_del: {
            int account = json_msg.at("account");
            std::string group_name = json_msg.at("group_name");
            //todo
            break;
        }
        case cmd_set_icon: {
            int account = json_msg.at("account");
            std::string icon = json_msg.at("icon");
            SQLite::Statement query(db, "UPDATE user SET icon=? WHERE account=?");
            query.bind(1, icon);
            query.bind(2, account);
            query.exec();
            break;
        }
        default:
            break;
    }
    return 0;
}

std::vector<int> Session::getGroupMember(int group_account) {
    SQLite::Statement query(db, "SELECT member_account FROM member WHERE group_account=?;");
    query.bind(1, group_account);
    std::vector<int> member_list;
    while (query.executeStep()) {
        int account = query.getColumn(0);
        member_list.push_back(account);
    }
    return member_list;
}

std::vector<int> Session::getGroupList(int account) {
    SQLite::Statement query(db, "SELECT group_account FROM member WHERE member_id=?;");
    query.bind(1, account);
    std::vector<int> group_list;
    while (query.executeStep()) {
        int group_account = query.getColumn(0);
        group_list.push_back(group_account);
    }
    return group_list;
}

std::vector<int> Session::getFriendList(int account) {
    SQLite::Statement query(db, R"(SELECT account 
       FROM user 
       WHERE account IN (
           SELECT user2 FROM friend WHERE user1 = ?
           UNION ALL
           SELECT user1 FROM friend WHERE user2 = ?
       ))");
    query.bind(1, account);
    query.bind(2, account);
    std::vector<int> friend_list;
    while (query.executeStep()) {
        int friend_account = query.getColumn(0);
        friend_list.push_back(friend_account);
    }
    return friend_list;
}

std::vector<int> Session::getFriendListFd(const std::vector<int> &friend_list) {
    std::vector<int> fds;
    for (auto friend_account: friend_list) {
        for (auto &[fd, account]: user_map) {
            if (account == friend_account) {
                fds.push_back(fd);
            }
        }
    }
    return fds;
}

int Session::getFriendFd(int friend_account) {
    for (auto &[fd, account]: user_map) {
        if (account == friend_account) {
            return fd;
        }
    }
    return -1;
}

void Session::sendSystemMsg(int fd, const std::string &msg) {
    nlohmann::json json_msg;
    json_msg["cmd"] = cmd_friend_chat;
    json_msg["sender"] = 10000;
    json_msg["msg"] = msg;
    sendMsg(fd, json_msg);
}

int Session::getGroupOwnFd(int group_account) {
    SQLite::Statement query(db, "SELECT group_own_fd FROM group_table WHERE group_account=?;");
    query.bind(1, group_account);
    while (query.executeStep()) {
        int group_own_fd = query.getColumn(0);
        return group_own_fd;
    }
    return -1;
}
