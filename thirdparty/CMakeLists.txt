cmake_minimum_required(VERSION 3.12)

# 项目名称和版本
project(MyTools VERSION 1.0.0 LANGUAGES CXX)

# 设置 C++ 标准 (推荐 C++17 或更高)
set(CMAKE_CXX_STANDARD 17)
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