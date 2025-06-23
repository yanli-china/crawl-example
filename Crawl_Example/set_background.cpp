#include <windows.h>
#include <gdiplus.h>
#include <shobjidl.h>  // IFileOpenDialog
#include <commdlg.h>
#include <string>
#include <iostream>

#pragma comment(lib, "gdiplus.lib")
//#pragma comment(lib, "Comdlg32.lib")
//#pragma comment(lib, "user32.lib")
//#pragma comment(lib, "ole32.lib")

using namespace Gdiplus;


// 初始化 GDI+
void InitGDIPlus(ULONG_PTR& token) {
    GdiplusStartupInput gdiStartupInput;
    GdiplusStartup(&token, &gdiStartupInput, nullptr);
}


// 获取 BMP 编码器 CLSID
bool GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0, size = 0;
    GetImageEncodersSize(&num, &size);
    if (size == 0) return false;

    ImageCodecInfo* pImageCodecInfo = (ImageCodecInfo*)malloc(size);
    if (pImageCodecInfo == nullptr) return false;

    GetImageEncoders(num, size, pImageCodecInfo);
    bool found = false;
    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[i].Clsid;
            found = true;
            break;
        }
    }

    free(pImageCodecInfo);
    return found;
}


// 图片转换
bool ConvertImageToBmp(const std::wstring& inputPath, const std::wstring& outputBmpPath) {
    Bitmap* image = Bitmap::FromFile(inputPath.c_str());
    if (!image || image->GetLastStatus() != Ok) {
        delete image;
        return false;
    }

    CLSID bmpClsid;
    if (!GetEncoderClsid(L"image/bmp", &bmpClsid)) {
        delete image;
        return false;
    }

    Status status = image->Save(outputBmpPath.c_str(), &bmpClsid, nullptr);
    delete image;
    return status == Ok;
}


// 设置桌面壁纸
bool SetDesktopWallpaper(const std::wstring& bmpPath) {
    return SystemParametersInfoW(
        SPI_SETDESKWALLPAPER,
        0,
        (PVOID)bmpPath.c_str(),
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
    );
}


// 打开文件对话框获取图片路径
std::wstring OpenImageFileDialog() {
    IFileOpenDialog* pFileOpen;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFileOpen));
    if (FAILED(hr)) return L"";

    COMDLG_FILTERSPEC filter[] = {
        {L"图片文件", L"*.jpg;*.jpeg;*.png;*.bmp"},
        {L"所有文件", L"*.*"},
    };

    pFileOpen->SetFileTypes(2, filter);
    pFileOpen->SetTitle(L"选择一张图片作为壁纸");

    hr = pFileOpen->Show(nullptr);
    if (SUCCEEDED(hr)) {
        IShellItem* pItem;
        hr = pFileOpen->GetResult(&pItem);
        if (SUCCEEDED(hr)) {
            PWSTR pszFilePath;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
            if (SUCCEEDED(hr)) {
                std::wstring path(pszFilePath);
                CoTaskMemFree(pszFilePath);
                pItem->Release();
                pFileOpen->Release();
                return path;
            }
            pItem->Release();
        }
    }
    pFileOpen->Release();
    return L"";
}


// WinMain: 启动 GUI 应用程序
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{

    // 初始化 COM 和 GDI+
    CoInitialize(nullptr);
    ULONG_PTR gdiplusToken;
    InitGDIPlus(gdiplusToken);

    // 选择图片
    std::wstring imagePath = OpenImageFileDialog();
    if (imagePath.empty()) {
        MessageBoxW(nullptr, L"没有选择图片。", L"提示", MB_OK | MB_ICONINFORMATION);
        GdiplusShutdown(gdiplusToken);
        CoUninitialize();
        return 0;
    }

    std::wstring tempBmpPath = L"C:\\Users\\1000260871\\Desktop\\Crawl_Example\\Crawl_Example\\wallpaper_temp_gui.bmp";

    // 转换图片
    if (!ConvertImageToBmp(imagePath, tempBmpPath)) {
        MessageBoxW(nullptr, L"图片转换为 BMP 失败。", L"错误", MB_OK | MB_ICONERROR);
        GdiplusShutdown(gdiplusToken);
        CoUninitialize();
        return 1;
    }

    // 设置壁纸
    if (!SetDesktopWallpaper(tempBmpPath)) {
        MessageBoxW(nullptr, L"设置壁纸失败！", L"错误", MB_OK | MB_ICONERROR);
    }
    else {
        DeleteFileW(tempBmpPath.c_str());  // 删除临时 BMP 文件
        MessageBoxW(nullptr, L"壁纸设置成功！", L"成功", MB_OK | MB_ICONINFORMATION);
    }

    GdiplusShutdown(gdiplusToken);
    CoUninitialize();
    return 0;
}
