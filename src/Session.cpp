//
// Created by Kotori on 2025/5/23.
//

#include "Session.h"

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

void Session::sendMsg(const std::vector<int>& fds, nlohmann::json &json_msg) {
    const std::string msg = json_msg.dump();
    const uint32_t len = static_cast<uint32_t>(msg.length());
    const uint32_t net_len = htonl(len);

    /*消息格式：消息长度(4B) + 消息内容*/
    std::vector<char> message(sizeof(net_len) + len);
    memcpy(message.data(), &net_len, sizeof(net_len));
    memcpy(message.data() + sizeof(net_len), msg.data(), len);

    for (auto fd : fds) {
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
    nlohmann::json recv_msg;

}

int Session::handleMsg(nlohmann::json &json_msg) {
}

std::vector<int> Session::getGroupMember(int group_id) {
}

std::vector<int> Session::getGroupList(int account) {
}

std::vector<int> Session::getFriendList(int account) {
}

std::vector<int> Session::getFriendListFd(std::vector<int> friend_list) {
}

int Session::getFriendFd(int account) {
}

void Session::sendSystemMsg(int fd, std::string msg) {
}

int Session::getGroupOwnFd(int group_id) {
}
