# CMake 最低版本要求
cmake_minimum_required(VERSION 3.15)
# 设置项目名称和使用的语言
project(vtlctrl VERSION 1.0 LANGUAGES CXX)

# 设置 C++ 标准 (这里使用 C++14)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 开启文件夹支持 (在 VS 中可以看到逻辑文件夹结构)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 输出目录设置 (把 exe 和 dll 都在 bin 目录下生成)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- 核心结构 ---
# 1. 先添加第三方库 (ImGui)
add_subdirectory(vendor)

# 2. 再添加主程序
add_subdirectory(src)