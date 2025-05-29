//
// Created by Kotori on 2025/5/26.
//

#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H
#include "Session.h"

class CommandHandler {
public:
    // 传参类型待修改
    static void registerUser(int account, const std::string &password, const std::string &name, Session *session);

    static void login(int account, const std::string &password, Session *session);

    static void searchFriend(const std::string &info, Session *session);

    static void addFriendRequest(int account, const UserInfo &info, const std::string &msg, Session *session);

    static void searchFriendList(int account, Session *session);

    static void searchGroup(std::string info, Session *session);

    static void addGroupRequest(int group_account, std::string group_name, UserInfo info, std::string msg,
                                Session *session);

    static void searchGroupList(int account, Session *session);

    static void searchGroupMemberList(int account, Session *session);

    // 群主相关（创建群、增删群成员）
    // todo
};


#endif // COMMANDHANDLER_H
