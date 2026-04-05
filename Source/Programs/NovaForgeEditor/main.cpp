// NovaForge Editor — Development tool
#include "NF/Core/Core.h"
#include "NF/Editor/Editor.h"
#include "NF/Input/Input.h"
#ifdef _WIN32
#  include "NF/Input/Win32InputAdapter.h"
#  include <windows.h>
#  include <chrono>
#endif

#ifdef _WIN32
static NF::EditorApp* g_editor    = nullptr;
static NF::Win32InputAdapter* g_inputAdapter = nullptr;
static int g_clientW = 1280, g_clientH = 800;

static void paintEditorGDI(HDC hdc, int w, int h) {
    // dark background
    RECT full{0, 0, w, h};
    HBRUSH bg = CreateSolidBrush(RGB(30,30,30));
    FillRect(hdc, &full, bg); DeleteObject(bg);

    SetBkMode(hdc, TRANSPARENT);
    HFONT font = CreateFontA(13,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,
        DEFAULT_PITCH|FF_DONTCARE,"Consolas");
    HFONT oldFont = (HFONT)SelectObject(hdc, font);

    // ── Menu bar (28px) ──────────────────────────────────────
    RECT menuR{0,0,w,28};
    HBRUSH mb = CreateSolidBrush(RGB(45,45,48)); FillRect(hdc,&menuR,mb); DeleteObject(mb);
    SetTextColor(hdc, RGB(220,220,220));
    const char* menus[] = {"  File","  Edit","  View","  Graph","  Code","  Help"};
    int mx = 4;
    for (auto* m : menus){ TextOutA(hdc,mx,7,m,(int)strlen(m)); mx+=80; }

    // ── Toolbar (28px) ───────────────────────────────────────
    RECT tbR{0,28,w,56};
    HBRUSH tb = CreateSolidBrush(RGB(37,37,38)); FillRect(hdc,&tbR,tb); DeleteObject(tb);
    SetTextColor(hdc, RGB(180,180,180));
    const char* tools[] = {" Select"," Move"," Rotate"," Scale","  |  "," Play"," Pause"," Stop"};
    int tx = 4;
    for (auto* t : tools){ TextOutA(hdc,tx,35,t,(int)strlen(t)); tx+=68; }

    // ── Left panel — Hierarchy (250px) ───────────────────────
    int leftW=250, rightW=300, topBar=56, botBarH=24, botPanelH=200;
    RECT leftR{0,topBar,leftW,h-botBarH};
    HBRUSH lbr = CreateSolidBrush(RGB(37,37,38)); FillRect(hdc,&leftR,lbr); DeleteObject(lbr);
    RECT leftHdr{0,topBar,leftW,topBar+20};
    HBRUSH lhbr = CreateSolidBrush(RGB(50,50,52)); FillRect(hdc,&leftHdr,lhbr); DeleteObject(lhbr);
    SetTextColor(hdc,RGB(200,200,200));
    TextOutA(hdc,8,topBar+4,"Hierarchy",9);

    // ── Right panel — Inspector (300px) ──────────────────────
    RECT rightR{w-rightW,topBar,w,h-botBarH};
    HBRUSH rbr = CreateSolidBrush(RGB(37,37,38)); FillRect(hdc,&rightR,rbr); DeleteObject(rbr);
    RECT rightHdr{w-rightW,topBar,w,topBar+20};
    HBRUSH rhbr = CreateSolidBrush(RGB(50,50,52)); FillRect(hdc,&rightHdr,rhbr); DeleteObject(rhbr);
    SetTextColor(hdc,RGB(200,200,200));
    TextOutA(hdc,w-rightW+8,topBar+4,"Inspector",9);

    // ── Bottom panel — Console ────────────────────────────────
    int botPanelY = h - botBarH - botPanelH;
    RECT botR{leftW, botPanelY, w-rightW, h-botBarH};
    HBRUSH bbr = CreateSolidBrush(RGB(25,25,25)); FillRect(hdc,&botR,bbr); DeleteObject(bbr);
    RECT botHdr{leftW, botPanelY, w-rightW, botPanelY+20};
    HBRUSH bhbr = CreateSolidBrush(RGB(50,50,52)); FillRect(hdc,&botHdr,bhbr); DeleteObject(bhbr);
    SetTextColor(hdc,RGB(200,200,200));
    TextOutA(hdc,leftW+8,botPanelY+4,"Console",7);

    // ── Center viewport ───────────────────────────────────────
    RECT vpR{leftW, topBar, w-rightW, botPanelY};
    HBRUSH vpbr = CreateSolidBrush(RGB(20,20,20)); FillRect(hdc,&vpR,vpbr); DeleteObject(vpbr);
    SetTextColor(hdc,RGB(55,55,55));
    const char* vpTxt = "[ Viewport  —  NovaForge 3D View ]";
    SIZE ts; GetTextExtentPoint32A(hdc,vpTxt,(int)strlen(vpTxt),&ts);
    TextOutA(hdc, leftW+((w-rightW-leftW)-ts.cx)/2,
                  topBar+((botPanelY-topBar)-ts.cy)/2, vpTxt,(int)strlen(vpTxt));

    // ── Status bar ────────────────────────────────────────────
    RECT sbR{0,h-botBarH,w,h};
    HBRUSH sbbr = CreateSolidBrush(RGB(0,120,215)); FillRect(hdc,&sbR,sbbr); DeleteObject(sbbr);
    SetTextColor(hdc,RGB(255,255,255));
    TextOutA(hdc,8,h-botBarH+5,"NovaForge Editor  v0.1.0  |  Ready",35);

    // ── Dividers ──────────────────────────────────────────────
    HPEN pen = CreatePen(PS_SOLID,1,RGB(60,60,60));
    HPEN oldPen = (HPEN)SelectObject(hdc,pen);
    auto line=[&](int x1,int y1,int x2,int y2){
        MoveToEx(hdc,x1,y1,nullptr); LineTo(hdc,x2,y2);
    };
    line(leftW,topBar,leftW,h-botBarH);
    line(w-rightW,topBar,w-rightW,h-botBarH);
    line(leftW,botPanelY,w-rightW,botPanelY);
    line(0,28,w,28);
    line(0,topBar,w,topBar);
    SelectObject(hdc,oldPen); DeleteObject(pen);

    SelectObject(hdc,oldFont); DeleteObject(font);
}

static LRESULT CALLBACK EditorWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (g_inputAdapter)
        g_inputAdapter->processMessage(hwnd, msg, wParam, lParam);
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        HDC mem = CreateCompatibleDC(hdc);
        HBITMAP bmp = CreateCompatibleBitmap(hdc, g_clientW, g_clientH);
        auto* old = SelectObject(mem, bmp);
        paintEditorGDI(mem, g_clientW, g_clientH);
        BitBlt(hdc, 0,0, g_clientW, g_clientH, mem, 0,0, SRCCOPY);
        SelectObject(mem,old); DeleteObject(bmp); DeleteDC(mem);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_SIZE:
        g_clientW = LOWORD(lParam); g_clientH = HIWORD(lParam);
        InvalidateRect(hwnd,nullptr,FALSE);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd); return 0;
    case WM_DESTROY:
        PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
#endif // _WIN32

int main(int argc, char* argv[]) {
    (void)argc; (void)argv;

    NF::coreInit();
    NF_LOG_INFO("Main", "=== NovaForge Editor ===");
    NF_LOG_INFO("Main", std::string("Version: ") + NF::NF_VERSION_STRING);

    NF::EditorApp editor;
    std::string execPath = (argc > 0) ? argv[0] : ".";
    if (!editor.init(1280, 800, execPath)) {
        NF_LOG_ERROR("Main", "Failed to initialize editor");
        return 1;
    }

    NF::InputSystem input;
    input.init();

#ifdef _WIN32
    g_editor = &editor;
    NF::Win32InputAdapter inputAdapter(input);
    g_inputAdapter = &inputAdapter;
    NF_LOG_INFO("Main", "Win32 input adapter initialized");

    WNDCLASSEXW wc{};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = EditorWndProc;
    wc.hInstance     = GetModuleHandleW(nullptr);
    wc.hCursor       = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"NovaForgeEditorWnd";
    RegisterClassExW(&wc);

    RECT wr{0,0,1280,800};
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);
    HWND hwnd = CreateWindowExW(0, L"NovaForgeEditorWnd",
        L"NovaForge Editor  v0.1.0",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        wr.right-wr.left, wr.bottom-wr.top,
        nullptr, nullptr, wc.hInstance, nullptr);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    NF_LOG_INFO("Main", "Editor window created (1280x800)");
#endif

    NF_LOG_INFO("Main", "Editor ready — entering main loop");

    auto lastTime = std::chrono::high_resolution_clock::now();
    bool running = true;
    while (running) {
#ifdef _WIN32
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (!running) break;
#else
        running = false;
#endif
        auto now = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float>(now - lastTime).count();
        lastTime = now;
        if (dt > 0.1f) dt = 0.1f;

        input.update();
        editor.update(dt, input);

#ifdef _WIN32
        InvalidateRect(hwnd, nullptr, FALSE);
        Sleep(16);
#endif
    }

#ifdef _WIN32
    g_editor = nullptr;
    g_inputAdapter = nullptr;
#endif
    input.shutdown();
    editor.shutdown();
    NF::coreShutdown();
    return 0;
}
