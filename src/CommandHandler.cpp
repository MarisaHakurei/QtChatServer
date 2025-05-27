//
// Created by Kotori on 2025/5/26.
//

#include "CommandHandler.h"

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

void CommandHandler::login(int account, const std::string &password, Session *session) {
    nlohmann::json json_msg;
    json_msg["cmd"] = cmd_login;
    SQLite::Statement query(db, "select name,signature,icon from user where account=? and password=?");
    query.bind(1, account);
    query.bind(2, password);
    if (!query.executeStep()) {
        LOGINFO("账号或密码错误");
        json_msg["res"] = "no";
        json_msg["err"] = "账号或密码错误";
    } else {
        LOGINFO("登陆成功");
        json_msg["res"] = "yes";
        for (int i = 0; i < 3; i++) {
            json_msg["info"][i] = query.getColumn(i).getString();
        }
        //存储用户信息
        UserInfo user_info = {
            account, password, json_msg["info"][0], json_msg["info"][1],
            1, json_msg["info"][2]
        };
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

void CommandHandler::searchFriend(const std::string& info, Session *session) {
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

void CommandHandler::addFriendRequest(int account, UserInfo info, std::string msg, Session *session) {
    //test
}

void CommandHandler::searchFriendList(int account, Session *session) {
}

void CommandHandler::searchGroup(std::string info, Session *session) {
}

void CommandHandler::addGroupRequest(int group_account, std::string group_name, UserInfo info, std::string msg,
                                     Session *session) {
}

/*根据用户账号查询用户的群组列表*/
void CommandHandler::searchGroupList(int account, Session *session) {
}

/*根据群组id查询群成员列表*/
void CommandHandler::searchGroupMemberList(int account, Session *session) {
}
