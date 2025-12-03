# 收集所有源码文件
# GLOB_RECURSE 会自动把 gui/ 和 core/ 下的 .cpp/.h 都加进来
# 注意：在生产环境中通常建议手动列出文件，但个人项目用 GLOB 很方便
file(GLOB_RECURSE APP_SOURCES 
    "*.cpp" 
    "*.h"
)

# --- 创建可执行文件 ---
# WIN32 选项：表示这是一个 GUI 程序，不会弹出黑色控制台
add_executable(MyApp WIN32 ${APP_SOURCES})

# --- 链接 ImGui ---
# 只需要这一行！头文件路径、DirectX 库都会自动引入
target_link_libraries(MyApp PRIVATE imgui_static)

# 设置 IDE 中的文件夹结构
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${APP_SOURCES})