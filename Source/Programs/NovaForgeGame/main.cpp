// NovaForge Game — Standalone game client
#include "NF/Core/Core.h"
#include "NF/Game/Game.h"
#include "NF/Renderer/Renderer.h"
#include "NF/UI/UI.h"
#include "NF/Input/Input.h"
#ifdef _WIN32
#  include "NF/Input/Win32InputAdapter.h"
#  include <windows.h>
#  include <chrono>
#endif

#ifdef _WIN32
static NF::InputSystem* g_input = nullptr;
static NF::Win32InputAdapter* g_adapter = nullptr;
static int g_w = 1920, g_h = 1080;

static void paintGame(HDC hdc, int w, int h) {
    RECT full{0,0,w,h};
    HBRUSH bg = CreateSolidBrush(RGB(5,8,12)); FillRect(hdc,&full,bg); DeleteObject(bg);
    SetBkMode(hdc,TRANSPARENT);
    HFONT font = CreateFontA(14,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE,"Consolas");
    HFONT old = (HFONT)SelectObject(hdc,font);
    // Crosshair
    int cx=w/2, cy=h/2, cs=10;
    HPEN pen = CreatePen(PS_SOLID,1,RGB(200,200,200));
    HPEN oldpen=(HPEN)SelectObject(hdc,pen);
    MoveToEx(hdc,cx-cs,cy,nullptr); LineTo(hdc,cx+cs,cy);
    MoveToEx(hdc,cx,cy-cs,nullptr); LineTo(hdc,cx,cy+cs);
    SelectObject(hdc,oldpen); DeleteObject(pen);
    // HUD bars
    RECT hpBar{20,h-60,220,h-44};
    HBRUSH hpBg = CreateSolidBrush(RGB(40,20,20)); FillRect(hdc,&hpBar,hpBg); DeleteObject(hpBg);
    RECT hpFill{20,h-60,120,h-44};
    HBRUSH hpFg = CreateSolidBrush(RGB(180,30,30)); FillRect(hdc,&hpFill,hpFg); DeleteObject(hpFg);
    SetTextColor(hdc,RGB(220,220,220));
    TextOutA(hdc,22,h-58,"HP",2);
    RECT enBar{20,h-38,220,h-22};
    HBRUSH enBg = CreateSolidBrush(RGB(20,20,50)); FillRect(hdc,&enBar,enBg); DeleteObject(enBg);
    RECT enFill{20,h-38,160,h-22};
    HBRUSH enFg = CreateSolidBrush(RGB(30,80,200)); FillRect(hdc,&enFill,enFg); DeleteObject(enFg);
    TextOutA(hdc,22,h-36,"EN",2);
    // Version watermark
    SetTextColor(hdc,RGB(50,50,50));
    std::string verText = std::string("NovaForge v") + NF::NF_VERSION_STRING;
    TextOutA(hdc,w-180,h-20,verText.c_str(),static_cast<int>(verText.size()));
    SelectObject(hdc,old); DeleteObject(font);
}

static LRESULT CALLBACK GameWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_adapter) g_adapter->processMessage(hwnd,msg,wParam,lParam);
    switch(msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps; HDC hdc=BeginPaint(hwnd,&ps);
        HDC mem=CreateCompatibleDC(hdc);
        HBITMAP bmp=CreateCompatibleBitmap(hdc,g_w,g_h);
        auto* old=SelectObject(mem,bmp);
        paintGame(mem,g_w,g_h);
        BitBlt(hdc,0,0,g_w,g_h,mem,0,0,SRCCOPY);
        SelectObject(mem,old); DeleteObject(bmp); DeleteDC(mem);
        EndPaint(hwnd,&ps); return 0;
    }
    case WM_SIZE: g_w=LOWORD(lParam); g_h=HIWORD(lParam); InvalidateRect(hwnd,nullptr,FALSE); return 0;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd,msg,wParam,lParam);
}
#endif

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    NF::coreInit();
    NF_LOG_INFO("Main","=== NovaForge Game ===");
    NF_LOG_INFO("Main", std::string("Version: ") + NF::NF_VERSION_STRING);

    NF::Renderer renderer;
    if (!renderer.init(1920, 1080)) {
        NF_LOG_ERROR("Main","Failed to initialize renderer"); return 1;
    }

    NF::InputSystem input;
    input.init();

#ifdef _WIN32
    g_input = &input;
    NF::Win32InputAdapter adapter(input);
    g_adapter = &adapter;

    WNDCLASSEXW wc{};
    wc.cbSize=sizeof(wc); wc.style=CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc=GameWndProc; wc.hInstance=GetModuleHandleW(nullptr);
    wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);
    wc.hbrBackground=(HBRUSH)(COLOR_WINDOW+1);
    wc.lpszClassName=L"NovaForgeGameWnd";
    RegisterClassExW(&wc);
    RECT wr{0,0,1280,720}; AdjustWindowRect(&wr,WS_OVERLAPPEDWINDOW,FALSE);
    std::wstring gameTitle = std::wstring(L"NovaForge  v") + std::wstring(NF::NF_VERSION_STRING, NF::NF_VERSION_STRING + strlen(NF::NF_VERSION_STRING));
    HWND hwnd=CreateWindowExW(0,L"NovaForgeGameWnd",gameTitle.c_str(),
        WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,
        wr.right-wr.left,wr.bottom-wr.top,nullptr,nullptr,wc.hInstance,nullptr);
    ShowWindow(hwnd,SW_SHOW); UpdateWindow(hwnd);
    NF_LOG_INFO("Main","Game window created");
#endif

    NF_LOG_INFO("Main","Game ready — entering main loop");
    auto last=std::chrono::high_resolution_clock::now();
    bool running=true;
    while(running){
#ifdef _WIN32
        MSG msg{};
        while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){
            if(msg.message==WM_QUIT){running=false;break;}
            TranslateMessage(&msg); DispatchMessageW(&msg);
        }
        if(!running) break;
        auto now=std::chrono::high_resolution_clock::now();
        float dt=std::chrono::duration<float>(now-last).count();
        last=now; if(dt>0.1f)dt=0.1f;
        input.update();
        InvalidateRect(hwnd,nullptr,FALSE);
        Sleep(16);
#else
        running=false;
#endif
    }

#ifdef _WIN32
    g_input=nullptr; g_adapter=nullptr;
#endif
    input.shutdown();
    renderer.shutdown();
    NF::coreShutdown();
    return 0;
}
