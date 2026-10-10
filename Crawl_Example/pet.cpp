#include <windows.h>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>

// 全局变量
const wchar_t* PET_EMOJIS[] = { L"(*^ω^*)", L"(o^▽^o)", L"(≧◡≦)", L"(◕‿◕✿)", L"(｡♥‿♥｡)" };
int currentEmojiIdx = 0;
bool isDragging = false;
POINT lastMousePos;
RECT windowRect;

// 物理参数
float posX = 500, posY = 500;
float velX = 1.5f, velY = 1.0f;
const int PET_WIDTH = 150;
const int PET_HEIGHT = 80;

// 窗口过程函数
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        SetTimer(hwnd, 1, 30, NULL);
        SetTimer(hwnd, 2, 3000, NULL);
        return 0;
    }
    case WM_TIMER: {
        if (wParam == 1 && !isDragging) {
            // 更新物理位置: s = s0 + v * t
            posX += velX;
            posY += velY;

            // --- 动态获取当前窗口所在的显示器信息 ---
            // MONITOR_DEFAULTTONEAREST 确保即使窗口稍微出界也能找到最近的屏幕
            HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mi = { sizeof(MONITORINFO) };
            if (GetMonitorInfo(hMonitor, &mi)) {
                // 使用 rcWork (排除任务栏的工作区) 或 rcMonitor (整个屏幕)
                RECT area = mi.rcWork;

                // 碰撞检测：仅针对当前所在屏幕的边界
                if (posX <= area.left) {
                    velX = std::abs(velX); // 强制向右
                    posX = (float)area.left;
                }
                else if (posX >= area.right - PET_WIDTH) {
                    velX = -std::abs(velX); // 强制向左
                    posX = (float)(area.right - PET_WIDTH);
                }

                if (posY <= area.top) {
                    velY = std::abs(velY); // 强制向下
                    posY = (float)area.top;
                }
                else if (posY >= area.bottom - PET_HEIGHT) {
                    velY = -std::abs(velY); // 强制向上
                    posY = (float)(area.bottom - PET_HEIGHT);
                }
            }

            SetWindowPos(hwnd, NULL, (int)posX, (int)posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        else if (wParam == 2) {
            currentEmojiIdx = rand() % 5;
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SetBkMode(hdc, TRANSPARENT);

        HFONT hFont = CreateFont(32, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
            VARIABLE_PITCH, TEXT("Arial"));
        SelectObject(hdc, hFont);
        SetTextColor(hdc, RGB(255, 105, 180));

        TextOut(hdc, 10, 10, PET_EMOJIS[currentEmojiIdx], wcslen(PET_EMOJIS[currentEmojiIdx]));

        DeleteObject(hFont);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_LBUTTONDOWN: {
        isDragging = true;
        SetCapture(hwnd);
        GetCursorPos(&lastMousePos);
        GetWindowRect(hwnd, &windowRect);
        return 0;
    }
    case WM_MOUSEMOVE: {
        if (isDragging) {
            POINT currentPos;
            GetCursorPos(&currentPos);
            int dx = currentPos.x - lastMousePos.x;
            int dy = currentPos.y - lastMousePos.y;

            posX = (float)(windowRect.left + dx);
            posY = (float)(windowRect.top + dy);

            SetWindowPos(hwnd, NULL, (int)posX, (int)posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;
    }
    case WM_LBUTTONUP: {
        isDragging = false;
        ReleaseCapture();
        GetWindowRect(hwnd, &windowRect);
        return 0;
    }
    case WM_RBUTTONDOWN: {
        PostQuitMessage(0);
        return 0;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    srand((unsigned int)time(NULL));
    const wchar_t CLASS_NAME[] = L"DesktopPetClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME, L"C++ Pet",
        WS_POPUP,
        (int)posX, (int)posY, PET_WIDTH, PET_HEIGHT,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;
    SetLayeredWindowAttributes(hwnd, RGB(255, 255, 255), 0, LWA_COLORKEY);
    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}