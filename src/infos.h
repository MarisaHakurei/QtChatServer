//
// Created by Kotori on 2025/5/23.
//

#ifndef INFOS_H
#define INFOS_H
#include <string>

/*用户信息*/
typedef struct user_info {
    int account{0}; // 账号
    std::string password; // 密码
    std::string name; // 用户名
    std::string sig; // 签名
    int online{0}; // 是否在线
    std::string icon; // 图标
} UserInfo;

/*登录信息*/
typedef struct login_info {
    std::string cmd; // 命令
    int account{0}; // 账号
    std::string password; // 密码
    std::string name; // 用户名
} LoginInfo;

/*发送信息*/
typedef struct send_info {
    std::string cmd; // 命令
    std::string info; // 信息内容
} SendInfo;

#endif // INFOS_H
