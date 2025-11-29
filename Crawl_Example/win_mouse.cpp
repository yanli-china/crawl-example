#include <windows.h>
#include <vector>
#include <string>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool isDrawing = false;
std::vector<POINT> points;

COLORREF drawColor = RGB(255, 0, 0); // 初始为红色
POINT currentMousePos = { 0, 0 };

// 保存绘图为 BMP 图片
void SaveBitmap(HWND hwnd, const wchar_t* filename) {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int width = rect.right;
    int height = rect.bottom;

    HDC hdcWindow = GetDC(hwnd);
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcWindow, width, height);
    SelectObject(hdcMemDC, hbmScreen);
    BitBlt(hdcMemDC, 0, 0, width, height, hdcWindow, 0, 0, SRCCOPY);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = -height; // top-down bitmap
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    DWORD dwBmpSize = ((width * 24 + 31) / 32) * 4 * height;
    HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
    char* lpbitmap = (char*)GlobalLock(hDIB);

    GetDIBits(hdcWindow, hbmScreen, 0, height, lpbitmap, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    DWORD dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = dwSizeofDIB;
    bmfHeader.bfType = 0x4D42;

    DWORD dwBytesWritten;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    CloseHandle(hFile);
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    DeleteObject(hbmScreen);
    DeleteDC(hdcMemDC);
    ReleaseDC(hwnd, hdcWindow);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"MouseDrawWindow";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_CROSS);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"绘图 + 保存 + 坐标 + 颜色切换", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

void AddPoint(LPARAM lParam) {
    POINT p;
    p.x = LOWORD(lParam);
    p.y = HIWORD(lParam);
    points.push_back(p);
}

void CycleColor() {
    static int state = 0;
    state = (state + 1) % 3;
    if (state == 0) drawColor = RGB(255, 0, 0);
    if (state == 1) drawColor = RGB(0, 255, 0);
    if (state == 2) drawColor = RGB(0, 0, 255);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_LBUTTONDOWN:
        isDrawing = true;
        points.clear();
        AddPoint(lParam);
        break;

    case WM_MOUSEMOVE:
        currentMousePos.x = LOWORD(lParam);
        currentMousePos.y = HIWORD(lParam);
        if (isDrawing) {
            AddPoint(lParam);
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_LBUTTONUP:
        isDrawing = false;
        AddPoint(lParam);
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_KEYDOWN:
        if (wParam == 'S') {
            SaveBitmap(hwnd, L"output.bmp");
            MessageBox(hwnd, L"已保存为 output.bmp", L"保存成功", MB_OK);
        }
        else if (wParam == 'C') {
            CycleColor();
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 绘制路径
        if (points.size() >= 2) {
            HPEN hPen = CreatePen(PS_SOLID, 2, drawColor);
            HGDIOBJ oldPen = SelectObject(hdc, hPen);

            MoveToEx(hdc, points[0].x, points[0].y, NULL);
            for (size_t i = 1; i < points.size(); ++i) {
                LineTo(hdc, points[i].x, points[i].y);
            }

            SelectObject(hdc, oldPen);
            DeleteObject(hPen);
        }

        // 计算文本区域大小（大概估算）
        const int textX = 10;
        const int textY = 10;
        const int textWidth = 150;
        const int textHeight = 20;

        // 擦除文本区域：填充背景色
        RECT rect = { textX, textY, textX + textWidth, textY + textHeight };
        HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW)); // 和窗口背景色一致
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        // 绘制鼠标坐标文本
        std::wstring coordText = L"坐标: (" + std::to_wstring(currentMousePos.x) + L", " + std::to_wstring(currentMousePos.y) + L")";
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);
        TextOut(hdc, textX, textY, coordText.c_str(), (int)coordText.length());

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}
