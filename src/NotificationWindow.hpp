#ifndef _MAIN_WINDOW_HPP_
#define _MAIN_WINDOW_HPP_

#include <string>
#include "UIElement.hpp"

class NotificationWindow : public UIElement
{
    public:
        /// Constructor
        NotificationWindow();

        /// Destructor
        ~NotificationWindow();

        /// Sets the notification message
        void SetMessage(const std::string& msg);

    private:
        /// Registers the window class used to create notification windows
        bool Register();

        /// Spawns the notification window represented by this object instance
        void Create();

        /// Destroys the window coupled with this object instance
        void Destroy();

        /// Called by WM_PAINT message in the WndProc to do the window drawing
        void OnPaint();

        /// The WndProc
        LRESULT CALLBACK MessageHandler(HWND hh, UINT mm, WPARAM ww, LPARAM ll);

        /// Handle to current window
        HWND mHwnd;

        /// The class name associated with the notification windows
        static const TCHAR* wndClassName;
};

#endif

