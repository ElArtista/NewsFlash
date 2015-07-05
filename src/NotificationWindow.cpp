#include "NotificationWindow.hpp"
#include <thread>
#include <sstream>

const TCHAR* NotificationWindow::wndClassName = _T("NotificationWndClass");

NotificationWindow::NotificationWindow() : mHwnd(nullptr)
{
    // Check if window class is registered and register it if it isn't
    WNDCLASS wc;
    if (!GetClassInfo(GetModuleHandle(0), wndClassName, &wc))
        Register();

    // Spawn the window instance
    Create();
}

NotificationWindow::~NotificationWindow()
{
    SendMessage(mHwnd, WM_DESTROY, 0, 0);
}

bool NotificationWindow::Register()
{
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = UIElement::ProxyMsgHandler;
    wc.hInstance = GetModuleHandle(0);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszMenuName = 0;
    wc.hbrBackground = 0;
    wc.lpszClassName = wndClassName;
    return RegisterClassEx(&wc) != 0;
}

void NotificationWindow::Create()
{
    // Get desktop dimensions
    RECT desktopRect = {};
    GetWindowRect(GetDesktopWindow(), &desktopRect);

    // Calculate the window dimensions
    RECT rect = desktopRect;

    // The window styles that the window will use
    int wStyle = WS_BORDER | WS_VISIBLE;

    // Set the client area to the 80% of the desktop resolution
    rect.bottom = (LONG)(rect.bottom * 0.1);
    rect.right = rect.bottom;
    AdjustWindowRectEx(&rect, wStyle, TRUE, 0);

    mHwnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        wndClassName,
        _T(""),
        wStyle,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        200,
        200,
        0,
        0,
        GetModuleHandle(0),
        this
    );

    // Remove borders and stuff
    SetWindowLongPtr(mHwnd, GWL_STYLE, 0);
    //ShowWindow(mHwnd, SW_SHOW);
    UpdateWindow(mHwnd);
}

void NotificationWindow::Destroy()
{
    DestroyWindow(mHwnd);
    SetWindowLongPtr(mHwnd, GWLP_USERDATA, static_cast<LPARAM>(0));
}

void NotificationWindow::SetMessage(const std::string& msg)
{
    mMessage = msg;
}

void NotificationWindow::Show(bool s)
{
    ShowWindow(mHwnd, s ? SW_SHOW : SW_HIDE);
}

std::pair<int, int> NotificationWindow::GetPosition() const
{
    RECT rc;
    GetWindowRect(mHwnd, &rc);
    return std::make_pair(rc.left, rc.top);
}

void NotificationWindow::SetPosition(int x, int y)
{
    SetWindowPos(mHwnd, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

unsigned int NotificationWindow::GetAlpha() const
{
    BYTE alpha;
    GetLayeredWindowAttributes(mHwnd, nullptr, &alpha, nullptr);
    return alpha * 100 / 255;
}

void NotificationWindow::SetAlpha(unsigned int alpha)
{
    COLORREF col = {0};
    SetLayeredWindowAttributes(mHwnd, col, static_cast<BYTE>(alpha) * 255 / 100, LWA_ALPHA);
}

void NotificationWindow::OnPaint()
{
    // Begin Paint
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(mHwnd, &ps);

    // Get client dimensions
    RECT clientRect;
    GetClientRect(mHwnd, &clientRect);
    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    // Change device mapping mode for better font rendering
    SetMapMode(hdc, MM_TEXT);

    // Create memory bitmap
    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hBmp = CreateCompatibleBitmap(hdc, width, height);
    SelectObject(hMemDC, hBmp);

    // =
    // Actual draw operations start here

    // Clear the memory bitmap to color first
    HBRUSH hbrWhite = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    FillRect(hMemDC, &clientRect, hbrWhite); 

    // Draw the chessboard
    HBRUSH hbrGray = static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH));

    RECT bufferRect;
    for (int i = 0; i < 13; i++) 
    { 
        int x = (i * 80) % width;
        int y = ((i * 80) / height) * 40;
        SetRect(&bufferRect, x, y, x + 40, y + 40);
        FillRect(hMemDC, &bufferRect, hbrGray);
    }

    // Reset clientRect variable
    //GetClientRect(mHwnd, &clientRect);

    //
    // Draw the notification text in the center
    //

    // Calculate the positioning
    RECT textRect = clientRect;

    // Convert message to widechar
    std::wstringstream ws;
    ws << mMessage.c_str();
    std::wstring m = ws.str();

    // Draw the text
    int fontHeight = -MulDiv(90, GetDeviceCaps(hMemDC, LOGPIXELSY), 72);
    HFONT font = CreateFont(fontHeight, 0, 0, 0, FW_DONTCARE, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Consolas");
    HGDIOBJ oldFont = SelectObject(hMemDC, font);

    int prevBkMode = SetBkMode(hMemDC, TRANSPARENT);
    DrawText(hMemDC, m.c_str(), -1, &textRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
    SetBkMode(hMemDC, prevBkMode);

    SelectObject(hMemDC, oldFont);
    DeleteObject(font);

    // Actual draw operations end here
    // =

    // Copy memory context to the dc obtained by BeginPaint
    BitBlt(hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY);

    // Free allocated resources
    DeleteDC(hMemDC);
    DeleteObject(hBmp);

    // End Paint
    EndPaint(mHwnd, &ps);
}

LRESULT CALLBACK NotificationWindow::MessageHandler(HWND hh, UINT mm, WPARAM ww, LPARAM ll)
{
    switch (mm)
    {
        case WM_CREATE:
        {
            // Store the window handle
            mHwnd = hh;

            // Set opacity
            COLORREF col = {0};
            SetLayeredWindowAttributes(hh, col, 127, LWA_ALPHA);
            break;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
        {
            OnPaint();
            break;
        }
        case WM_DESTROY:
        {
            Destroy();
            break;
        }
        default:
            return DefWindowProc(hh, mm, ww, ll);
    }
    return 0;
}

