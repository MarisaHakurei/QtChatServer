//
// Created by Kotori on 2025/5/26.
//

#include "CommandHandler.h"

void CommandHandler::registerUser(int account, std::string password, std::string name, Session *session) {
    
}

void CommandHandler::login(int account, std::string password, Session *session) {
}

void CommandHandler::searchFriend(std::string info, Session *session) {
}

void CommandHandler::addFriendRequest(int account, UserInfo info, std::string msg, Session *session) {
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


