//
// Created by Kotori on 2025/5/26.
//

#include "CommandHandler.h"
#include <stdexcept>
#include <string>
#include "Common.h"
#include "SQLiteCpp/Statement.h"
#include "nlohmann/json_fwd.hpp"

/* 注册用户 */
void CommandHandler::registerUser(int account, const std::string &password, const std::string &name, Session *session) {
    nlohmann::json json_msg;
    json_msg["cmd"] = cmd_regist;
    SQLite::Statement query(db, "insert into user values (?,?,?,'hello',0,':/Icons/src/QQIcon/icon.jpg')");
    query.bind(1, account);
    query.bind(2, password);
    query.bind(3, name);
    int res = query.exec();
    if (res == 1) {
        LOGINFO("account=%d,注册成功..\n", account);
        json_msg["res"] = "yes";
        /*添加到系统管理员好友表中*/
        SQLite::Statement query1(db, "insert into friend values(10000,?)");
        query1.bind(1, account);
        query1.exec();
    } else {
        LOGINFO("account=%d,注册失败..\n", account);
        json_msg["res"] = "no";
        json_msg["err"] = "账号已存在";
    }
    session->sendMsg(json_msg);
}

/* 登录 */
void CommandHandler::login(int account, const std::string &password, Session *session) {
    nlohmann::json json_msg;
    json_msg["cmd"] = cmd_login;
    SQLite::Statement query(db, "select name,signature,icon from user where account=? and password=?");
    query.bind(1, account);
    query.bind(2, password);
    if (!query.executeStep()) {
        printf("账号或密码错误");
        json_msg["res"] = "no";
        json_msg["err"] = "账号或密码错误";
    } else {
        printf("登录成功");
        json_msg["res"] = "yes";
        for (int i = 0; i < 3; i++) {
            json_msg["info"][i] = query.getColumn(i).getString();
        }
        // 存储用户信息
        UserInfo user_info = {account, password, json_msg["info"][0], json_msg["info"][1], 1, json_msg["info"][2]};
        session->setUserInfo(user_info);
        session->setAccount(account);
        session->setIsLogin(true);
        user_map[session->getSocket()] = account;

        SQLite::Statement query1(db, "update user set online = 1 where account=?");
        query1.bind(1, account);
        query1.exec();
    }
    session->sendMsg(json_msg);
}

/* 查询好友 */
void CommandHandler::searchFriend(const std::string &info, Session *session) {
    /*info代表account或者name*/
    nlohmann::json json_msg;
    int account = 0;
    int count = 0;
    try {
        account = std::stoi(info);
    } catch (const std::invalid_argument &e) {
        account = 0;
    }
    json_msg["cmd"] = cmd_friend_search;
    SQLite::Statement query(db, "select account, name from user where account=? or name=?");
    query.bind(1, account);
    query.bind(2, info);
    while (query.executeStep()) {
        /*account唯一但可能重名*/
        json_msg["msg_list"][count]["account"] = query.getColumn(0).getString();
        json_msg["msg_list"][count]["name"] = query.getColumn(1).getString();
        count++;
    }
    json_msg["count"] = count;
    session->sendMsg(json_msg);
}

/* 添加好友 */
void CommandHandler::addFriendRequest(int account, const UserInfo &info, const std::string &msg, Session *session) {
    nlohmann::json json_msg;
    json_msg["cmd"] = cmd_add_friend_request;
    // 判断是否已经是好友
    SQLite::Statement query(db, "select * from friend where (user1=? and user2=?) or (user1=? and user2=?)");
    query.bind(1, info.account); // sender
    query.bind(2, account);
    query.bind(3, account);
    query.bind(4, info.account);
    if (query.executeStep()) {
        json_msg["res"] = "no";
        json_msg["err"] = "已经是好友";
        session->sendMsg(json_msg);
    } else {
        json_msg["sender"] = info.account;
        int fd = session->getFriendFd(account); // 判断对方是否在线
        if (fd <= 0) {
            json_msg["res"] = "no";
            json_msg["err"] = "对方不在线";
            session->sendMsg(json_msg);
        } else {
            json_msg["res"] = "yes";
            json_msg["send_msg"] = msg;
            json_msg["name"] = info.name;
            json_msg["sig"] = info.sig;
            json_msg["icon"] = info.icon;
            session->sendMsg(fd, json_msg);
        }
    }
}


/* 根据账号查找好友列表 */
void CommandHandler::searchFriendList(int account, Session *session) {
    nlohmann::json json_msg;
    int count = 0;
    json_msg["cmd"] = cmd_friend_list;
    std::cout << __LINE__ << std::endl;
    SQLite::Statement query(db, "select account,name,signature,online,icon from user where account in \
    (select user2 from friend where user1=? union select user1 from friend where user2=?)");
    std::cout << __LINE__ << std::endl;
    query.bind(1, account);
    query.bind(2, account);
    while (query.executeStep()) {
        json_msg["msg_list"][count]["account"] = query.getColumn(0).getInt();
        json_msg["msg_list"][count]["name"] = query.getColumn(1).getString();
        json_msg["msg_list"][count]["sig"] = query.getColumn(2).getString();
        json_msg["msg_list"][count]["online"] = query.getColumn(3).getInt();
        json_msg["msg_list"][count]["icon"] = query.getColumn(4).getString();
        count++;
    }
    json_msg["count"] = count;
    if (account == session->getAccount()) {
        session->sendMsg(json_msg);
    } else {
        session->sendMsg(session->getFriendFd(account), json_msg);
    }
    return;
}

/* 查找已有的群组列表 */
void CommandHandler::searchGroup(std::string info, Session *session) {
    /* info可能是群名或者昵称 */
    nlohmann::json json_msg;
    int account = 0;
    int count = 0;
    try {
        account = std::stoi(info);
    } catch (std::invalid_argument &e) {
        account = 0;
    }
    json_msg["cmd"] = cmd_group_search;
    SQLite::Statement query(db, "select group_account,group_name \
         from group_table \
         where group_account=? or group_name=?");
    query.bind(1, account);
    query.bind(2, info);
    while (query.executeStep()) {
        json_msg["msg_list"][count]["account"] = query.getColumn(0).getString();
        json_msg["msg_list"][count]["name"] = query.getColumn(1).getString();
        count++;
    }
    json_msg["count"] = count;
    session->sendMsg(json_msg);
    return;
}

/* 加群请求 */
void CommandHandler::addGroupRequest(int group_account, std::string group_name, UserInfo info, std::string msg,
                                     Session *session) {
    nlohmann::json json_msg;
    json_msg["cmd"] = cmd_group_join_request;
    // 判断是否在群里
    SQLite::Statement query(db, "select * from member where \
        group_account=? and member_account=?");
    query.bind(1, group_account);
    query.bind(2, info.account);
    if (query.executeStep()) {
        json_msg["res"] = "no";
        json_msg["err"] = "已经在群里";
        session->sendMsg(json_msg);
        return;
    } else {
        // 判断群主是否在线
        int fd = session->getGroupOwnFd(group_account);
        if (fd <= 0) {
            json_msg["res"] = "no";
            json_msg["err"] = "群主不在线";
            session->sendMsg(json_msg);
            return;
        } else {
            json_msg["res"] = "yes";
            json_msg["msg"] = msg;
            json_msg["name"] = info.name;
            json_msg["sig"] = info.sig;
            json_msg["icon"] = info.icon;
            session->sendMsg(fd, json_msg);
        }
    }
}

/*根据账号查询群组列表*/
void CommandHandler::searchGroupList(int account, Session *session) {
    nlohmann::json json_msg;
    json_msg["cmd"] = cmd_group_list;
    int count = 0;
    std::cout << __LINE__ << std::endl;
    SQLite::Statement query(db, "select group_account, group_name \
        from group_table \
        where group_account in (select group_account from member where member_account=?)");
    std::cout << __LINE__ << std::endl;
    query.bind(1, account);
    while (query.executeStep()) {
        json_msg["msg_list"][count]["account"] = query.getColumn(0).getString();
        json_msg["msg_list"][count]["name"] = query.getColumn(0).getString();
        count++;
    }
    json_msg["count"] = count;
    if (account == session->getAccount())
        session->sendMsg(json_msg);
    else
        session->sendMsg(session->getFriendFd(account), json_msg);
    return;
}

/*根据群组id查询群成员列表*/
void CommandHandler::searchGroupMemberList(int account, Session *session) {
    nlohmann::json json_msg;
    int count = 0;
    json_msg["cmd"] = cmd_group_member_list;
    SQLite::Statement query(db, "select member_account,group_nickname from member \
        group_account=?");
    query.bind(1, account);
    while (query.executeStep()) {
        json_msg["msg_list"][count]["account"] = query.getColumn(0).getString();
        json_msg["msg_list"][count]["name"] = query.getColumn(1).getString();
        count++;
    }
    json_msg["count"] = count;
    session->sendMsg(json_msg);
}
