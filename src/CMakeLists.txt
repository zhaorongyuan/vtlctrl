# 添加可执行文件
add_executable(vtlctrl main.cpp)

# 链接第三方库 (ImGui)
target_link_libraries(vtlctrl PRIVATE imgui_lib)

# 链接 Windows 系统库
# ImGui DX11 后端依赖 d3d11 和 d3dcompiler
# dwmapi 通常用于窗口边框处理等
if(WIN32)
    target_link_libraries(vtlctrl PRIVATE
        d3d11
        d3dcompiler
        dwmapi
    )
endif()

# 如果你想让控制台窗口消失（发布为 GUI 程序），取消下面这行的注释：
# set_target_properties(vtlctrl PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")