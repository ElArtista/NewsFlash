#include "NotificationService.hpp"

void NotificationService::Run()
{
    // Create the NotificationDrawer
    mDrawer = std::make_unique<NotificationDrawer>();

    // Create the message window that will receive the notification create events
    CreateMsgWnd();

    // Run the message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void NotificationService::RunAsync()
{
    auto l = [this]() { this->Run(); };
    std::thread t(l);
    t.detach();
}

void NotificationService::Stop()
{
    // Destroy all Notification windows
    mDrawer->Clear();

    // Destroy the NotificationDrawer
    mDrawer.reset(nullptr);

    // Close the message loop by sending WM_DESTROY to the message window
    SendMessage(mHMsgWnd, WM_DESTROY, 0, 0);
}

void NotificationService::ShowNotification(const std::string& msg, unsigned int lifetime)
{
    NotificationData* data = new NotificationData;
    data->msg = msg;
    data->lifetime = lifetime;
    PostMessage(mHMsgWnd, WM_SPAWN_NOTIFICATION, 0, reinterpret_cast<LPARAM>(data));
}

void NotificationService::CreateMsgWnd()
{
    // The dummy window class name
    const TCHAR* dummyWndClassName = _T("DummyClassName");

    // Check if window class is registered and register it if it isn't
    WNDCLASS wc;
    if (!GetClassInfo(GetModuleHandle(0), dummyWndClassName, &wc))
    {
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = UIElement::ProxyMsgHandler;
        wc.hInstance = GetModuleHandle(0);
        wc.lpszClassName = dummyWndClassName;
        RegisterClassEx(&wc);
    }

    // Create the message only window
    mHMsgWnd = CreateWindowEx(0, dummyWndClassName, _T(""), 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, this);
}

const UINT NotificationService::WM_SPAWN_NOTIFICATION = WM_USER + 77;

LRESULT NotificationService::MessageHandler(HWND hh, UINT mm, WPARAM ww, LPARAM ll)
{
    switch (mm)
    {
        case WM_SPAWN_NOTIFICATION:
        {
            // Fetch the notification data
            NotificationData* data = reinterpret_cast<NotificationData*>(ll);

            // Create and store the notification
            mDrawer->SpawnNotification(data->msg, data->lifetime);
            
            // Delete unused notification data
            delete data;
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hh, mm, ww, ll);
    }
    return 0;
}

