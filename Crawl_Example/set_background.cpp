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


// ��ʼ�� GDI+
void InitGDIPlus(ULONG_PTR& token) {
    GdiplusStartupInput gdiStartupInput;
    GdiplusStartup(&token, &gdiStartupInput, nullptr);
}


// ��ȡ BMP ������ CLSID
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


// ͼƬת��
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


// ���������ֽ
bool SetDesktopWallpaper(const std::wstring& bmpPath) {
    return SystemParametersInfoW(
        SPI_SETDESKWALLPAPER,
        0,
        (PVOID)bmpPath.c_str(),
        SPIF_UPDATEINIFILE | SPIF_SENDCHANGE
    );
}


// ���ļ��Ի����ȡͼƬ·��
std::wstring OpenImageFileDialog() {
    IFileOpenDialog* pFileOpen;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pFileOpen));
    if (FAILED(hr)) return L"";

    COMDLG_FILTERSPEC filter[] = {
        {L"ͼƬ�ļ�", L"*.jpg;*.jpeg;*.png;*.bmp"},
        {L"�����ļ�", L"*.*"},
    };

    pFileOpen->SetFileTypes(2, filter);
    pFileOpen->SetTitle(L"ѡ��һ��ͼƬ��Ϊ��ֽ");

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


// WinMain: ���� GUI Ӧ�ó���
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{

    // ��ʼ�� COM �� GDI+
    CoInitialize(nullptr);
    ULONG_PTR gdiplusToken;
    InitGDIPlus(gdiplusToken);

    // ѡ��ͼƬ
    std::wstring imagePath = OpenImageFileDialog();
    if (imagePath.empty()) {
        MessageBoxW(nullptr, L"û��ѡ��ͼƬ��", L"��ʾ", MB_OK | MB_ICONINFORMATION);
        GdiplusShutdown(gdiplusToken);
        CoUninitialize();
        return 0;
    }

    std::wstring tempBmpPath = L"C:\\Users\\1000260871\\Desktop\\Crawl_Example\\Crawl_Example\\wallpaper_temp_gui.bmp";

    // ת��ͼƬ
    if (!ConvertImageToBmp(imagePath, tempBmpPath)) {
        MessageBoxW(nullptr, L"ͼƬת��Ϊ BMP ʧ�ܡ�", L"����", MB_OK | MB_ICONERROR);
        GdiplusShutdown(gdiplusToken);
        CoUninitialize();
        return 1;
    }

    // ���ñ�ֽ
    if (!SetDesktopWallpaper(tempBmpPath)) {
        MessageBoxW(nullptr, L"���ñ�ֽʧ�ܣ�", L"����", MB_OK | MB_ICONERROR);
    }
    else {
        DeleteFileW(tempBmpPath.c_str());  // ɾ����ʱ BMP �ļ�
        MessageBoxW(nullptr, L"��ֽ���óɹ���", L"�ɹ�", MB_OK | MB_ICONINFORMATION);
    }

    GdiplusShutdown(gdiplusToken);
    CoUninitialize();
    return 0;
}
