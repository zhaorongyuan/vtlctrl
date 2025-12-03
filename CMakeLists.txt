# CMake 最低版本要求
cmake_minimum_required(VERSION 3.15)
# 设置项目名称和使用的语言
project(vtlctrl VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准 (这里使用 C++14)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 开启 Unicode 支持 (Windows 应用程序通常需要)
add_definitions(-DUNICODE -D_UNICODE)

# 添加子目录
add_subdirectory(thirdparty)
add_subdirectory(src)
#add_subdirectory(config)