//
// Created by Kotori on 2025/5/23.
//

#ifndef COMMON_H
#define COMMON_H
#include "DeThread.h"
#include "infos.h"

// 线程相关
#include <pthread.h>
#include <mutex>
#include <thread>

// 网络相关
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <cassert>
// c++
#include <iostream>
#include <map>
// 数据库
#include <SQLiteCpp/SQLiteCpp.h>
// json
#include <nlohmann/json.hpp>

#define RET_OK 0
#define RET_ERROR (-1)
#define RET_AGAIN (-2) // 重新读取
#define RET_EXIT (-3)  // 客户端退出

#ifndef FILENAME
#define FILENAME (__FILE__)
#endif

#ifndef FILEFUNCTION
#define FILEFUNCTION (__FUNCTION__)
#endif

#ifndef FILELINE
#define FILELINE (__LINE__)
#endif

#define ChatLog std::cout << "[ " << FILENAME << ":" << FILELINE << "][" << FILEFUNCTION << "]"

#define LOGINFO(format, ...)                                                                 \
    {                                                                                        \
        printf("[ %s : %d] [%s]>>" format, FILENAME, FILELINE, FILEFUNCTION, ##__VA_ARGS__); \
    }

#endif //COMMON_H
