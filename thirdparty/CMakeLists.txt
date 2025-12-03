# 定义 ImGui 源文件目录变量，方便后续引用
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")

# 列出所有需要编译的 ImGui 源文件
# 包括核心文件和 Win32/DX11 的后端实现
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
    ${IMGUI_DIR}/backends/imgui_impl_dx11.cpp
)

# 创建静态库目标
add_library(imgui_lib STATIC ${IMGUI_SOURCES})

# 设置头文件包含路径
# PUBLIC 关键字意味着链接这个库的目标 (如 main) 也能自动找到这些头文件
target_include_directories(imgui_lib PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

# Windows 平台特定设置
if(WIN32)
    # 消除一些不必需的 MSVC 警告
    target_compile_definitions(imgui_lib PUBLIC _CRT_SECURE_NO_WARNINGS)
endif()