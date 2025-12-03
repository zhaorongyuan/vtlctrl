// Dear ImGui: Windows API + DirectX 11 独立示例应用程序

// 了解 Dear ImGui:
// - 常见问题解答        https://dearimgui.com/faq
// - 入门指南            https://dearimgui.com/getting-started
// - 文档                https://dearimgui.com/docs (与本地 docs/ 文件夹相同).
// - 简介、链接以及更多内容请见 imgui.cpp 顶部

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>

// 数据
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// 辅助函数的前向声明
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// 主代码
int main(int, char**)
{
    // 使进程感知 DPI 并获取主显示器的缩放比例
    ImGui_ImplWin32_EnableDpiAwareness();
    //
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    // 创建应用程序窗口
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 示例", WS_OVERLAPPEDWINDOW, 100, 100, (int)(720 * main_scale), (int)(500 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

    // 初始化 Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // 显示窗口
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // 设置 Dear ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // 启用键盘控制
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // 启用游戏手柄控制

    // 设置 Dear ImGui 样式
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // 设置缩放
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // 烘焙固定的样式缩放。(在我们要动态样式缩放的解决方案之前，更改此项需要重置样式 + 再次调用此函数)
    style.FontScaleDpi = main_scale;        // 设置初始字体缩放。(使用 io.ConfigDpiScaleFonts=true 使这变得不必要。我们把两者都留在这里用于文档目的)

    // 设置平台/渲染器后端
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // 加载字体
    // - 如果没有加载字体，dear imgui 将使用默认字体。您也可以加载多个字体并使用 ImGui::PushFont()/PopFont() 来选择它们。
    // - AddFontFromFileTTF() 将返回 ImFont*，因此如果您需要在多个字体中进行选择，可以存储它。
    // - 如果文件无法加载，该函数将返回 nullptr。请在您的应用程序中处理这些错误（例如，使用断言，或显示错误并退出）。
    // - 在您的 imconfig 文件中使用 '#define IMGUI_ENABLE_FREETYPE' 以使用 Freetype 进行更高质量的字体渲染。
    // - 阅读 'docs/FONTS.md' 以获取更多说明和详细信息。如果您喜欢默认字体但希望它能更好地缩放，请考虑使用同一作者的 'ProggyVector'！
    // - 请记住，在 C/C++ 中，如果您想在字符串字面量中包含反斜杠 \，您需要写入双反斜杠 \\ ！
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);

    // 我们的状态
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // 主循环
    bool done = false;
    while (!done)
    {
        // 轮询并处理消息（输入、窗口大小调整等）
        // 请参阅下面的 WndProc() 函数，了解我们要向 Win32 后端分派事件。
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // 处理窗口最小化或屏幕锁定
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // 处理窗口大小调整（我们不直接在 WM_SIZE 处理程序中调整大小）
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // 开始 Dear ImGui 帧
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // // 1. 显示演示窗口（大多数示例代码都在 ImGui::ShowDemoWindow() 中！您可以浏览其代码以了解更多关于 Dear ImGui 的信息！）。
        // if (show_demo_window)
        //     ImGui::ShowDemoWindow(&show_demo_window);

        // // 2. 显示我们自己创建的一个简单窗口。我们使用 Begin/End 对来创建一个命名窗口。
        // {
        //     static float f = 0.0f;
        //     static int counter = 0;
        //
        //     ImGui::Begin("你好，世界！");                          // 创建一个名为 "你好，世界！" 的窗口并向其追加内容。
        //
        //     ImGui::Text("这是一些有用的文本。");                   // 显示一些文本（您也可以使用格式字符串）
        //     ImGui::Checkbox("演示窗口", &show_demo_window);      // 编辑存储我们窗口打开/关闭状态的布尔值
        //     ImGui::Checkbox("另一个窗口", &show_another_window);
        //
        //     ImGui::SliderFloat("浮点数", &f, 0.0f, 1.0f);            // 使用滑块编辑一个从 0.0f 到 1.0f 的浮点数
        //     ImGui::ColorEdit3("清除颜色", (float*)&clear_color); // 编辑代表颜色的 3 个浮点数
        //
        //     if (ImGui::Button("按钮"))                            // 按钮被点击时返回 true（大多数小部件在编辑/激活时返回 true）
        //         counter++;
        //     ImGui::SameLine();
        //     ImGui::Text("计数器 = %d", counter);
        //
        //     ImGui::Text("应用程序平均耗时 %.3f ms/帧 (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        //     ImGui::End();
        // }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;
            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }



        // // 3. 显示另一个简单窗口。
        // if (show_another_window)
        // {
        //     ImGui::Begin("另一个窗口", &show_another_window);   // 传递指向我们要控制的布尔变量的指针（窗口将有一个关闭按钮，点击时会清除该布尔值）
        //     ImGui::Text("来自另一个窗口的问候！");
        //     if (ImGui::Button("关闭我"))
        //         show_another_window = false;
        //     ImGui::End();
        // }

        // 渲染
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // 呈现 (Present)
        HRESULT hr = g_pSwapChain->Present(1, 0);   // 开启垂直同步呈现
        //HRESULT hr = g_pSwapChain->Present(0, 0); // 关闭垂直同步呈现
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // 清理
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// 辅助函数

bool CreateDeviceD3D(HWND hWnd)
{
    // 设置交换链
    // 这是一个基本设置。最佳情况是使用例如 DXGI_SWAP_EFFECT_FLIP_DISCARD 并以不同方式处理全屏模式。请参阅 #8979 了解建议。
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // 如果硬件不可用，请尝试高性能 WARP 软件驱动程序。
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// 前向声明来自 imgui_impl_win32.cpp 的消息处理程序
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 消息处理程序
// 您可以读取 io.WantCaptureMouse, io.WantCaptureKeyboard 标志来判断 dear imgui 是否想要使用您的输入。
// - 当 io.WantCaptureMouse 为真时，不要将鼠标输入数据分发给您的主应用程序，或者清除/覆盖您的鼠标数据副本。
// - 当 io.WantCaptureKeyboard 为真时，不要将键盘输入数据分发给您的主应用程序，或者清除/覆盖您的键盘数据副本。
// 通常，您总是可以将所有输入传递给 dear imgui，并根据这两个标志对您的应用程序隐藏它们。
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // 队列调整大小
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // 禁用 ALT 应用程序菜单
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}