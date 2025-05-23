cmake_minimum_required(VERSION 3.28)
project(QtChatServer)

set(CMAKE_CXX_STANDARD 17)

#链接第三方库
add_subdirectory(thirdparty/json-develop)
add_subdirectory(thirdparty/SQLiteCpp)

add_executable(QtChatServer
        src/main.cpp
        src/common.cpp
        src/common.h
        src/infos.h
        src/DeThread.h
        src/Session.cpp
        src/Session.h)

target_link_libraries(${PROJECT_NAME}
        PRIVATE pthread SQLiteCpp sqlite3 nlohmann_json
)

