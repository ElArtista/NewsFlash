#include "NotificationWindow.hpp"
#include <thread>

const TCHAR* NotificationWindow::wndClassName = _T("NotificationWndClass");

NotificationWindow::NotificationWindow()
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
    Destroy();
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
    ShowWindow(mHwnd, SW_SHOW);
    UpdateWindow(mHwnd);
}

void NotificationWindow::Destroy()
{
    DestroyWindow(mHwnd);
    SetWindowLongPtr(mHwnd, GWLP_USERDATA, static_cast<LPARAM>(0));
}

void NotificationWindow::SetMessage(const std::string& msg)
{
    (void) msg;
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

    // Change dc measurements to logical units
    SetMapMode(hdc, MM_ANISOTROPIC); 
    SetWindowExtEx(hdc, 100, 100, NULL); 
    SetViewportExtEx(hdc, clientRect.right, clientRect.bottom, NULL); 

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

    for (int i = 0; i < 13; i++) 
    { 
        int x = (i * 40) % 100; 
        int y = ((i * 40) / 100) * 20; 
        SetRect(&clientRect, x, y, x + 20, y + 20); 
        FillRect(hMemDC, &clientRect, hbrGray); 
    }  
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
        case WM_PAINT:
        {
            OnPaint();
            break;
        }
        default:
            return DefWindowProc(hh, mm, ww, ll);
    }
    return 0;
}

