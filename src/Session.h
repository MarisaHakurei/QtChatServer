//
// Created by Kotori on 2025/5/23.
//

#ifndef SESSION_H
#define SESSION_H

#include "common.h"
#include "nlohmann/json_fwd.hpp"

class Session {
public:
    explicit Session(int socket);

    ~Session();

public:

    void sendMsg(nlohmann::json &json_msg);

    void sendMsg(int fd, nlohmann::json &json_msg);

    void sendMsg(const std::vector<int>& fds, nlohmann::json &json_msg);

    nlohmann::json recvMsg();

    int handleMsg(nlohmann::json &json_msg);

    //获取群成员账号
    std::vector<int> getGroupMember(int group_id);

    //获取群列表
    std::vector<int> getGroupList(int account);

    //获取好友列表
    std::vector<int> getFriendList(int account);

    //获取在线好友fd
    std::vector<int> getFriendListFd(std::vector<int> friend_list);

    int getFriendFd(int account);

    //发送系统消息
    void sendSystemMsg(int fd, std::string msg);

    //获取群主fd
    int getGroupOwnFd(int group_id);

public:
    //get set
    int getSocket() { return socket_; }
    int getAccount() { return account_; }
    void setAccount(int account) { account_ = account; }
    std::string getUserName() { return user_name_; }
    void setUserName(const std::string &user_name) { user_name_ = user_name; }
    bool getIsLogin() { return is_login_; }
    void setIsLogin(bool is_login) { is_login_ = is_login; }

    std::vector<int> getCurrentGroupList() { return group_list_; }
    void setCurrentGroupList(const std::vector<int> &group_list) { group_list_ = group_list; }
    std::vector<int> getCurrentFriendList() { return friend_list_; }
    void setCurrentFriendList(const std::vector<int> &friend_list) { friend_list_ = friend_list; }

private:
    //clear
private:
    int socket_;
    int account_; //账号
    std::string user_name_; //用户名
    bool is_login_; //登录状态
    std::vector<int> group_list_; //群列表
    std::vector<int> friend_list_; //好友列表
    UserInfo user_info_; //用户信息
    SQLite::Statement *statement_; //查询指针
};


#endif //SESSION_H
