cmake_minimum_required(VERSION 3.28)
project(QtChatServer)

set(CMAKE_CXX_STANDARD 17)

# 添加第三方库子目录
add_subdirectory(thirdparty/json-develop)
add_subdirectory(thirdparty/SQLiteCpp)

# 声明可执行文件
add_executable(${PROJECT_NAME}
        src/main.cpp
        src/Common.cpp
        src/Common.h
        src/infos.h
        src/DeThread.h
        src/Session.cpp
        src/Session.h
        src/CommandHandler.cpp
        src/CommandHandler.h
)

# 显式链接目标（关键改动）
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        pthread
        SQLiteCpp          # SQLiteCpp 的头文件路径自动传递给主工程
        SQLite::SQLite3    # 直接使用 SQLite3 的现代 CMake 目标
        nlohmann_json::nlohmann_json  # JSON 库的正确目标名
)
