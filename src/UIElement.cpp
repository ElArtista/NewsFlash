#include "UIElement.hpp"

LRESULT CALLBACK UIElement::ProxyMsgHandler(HWND hh, UINT mm, WPARAM ww, LPARAM ll)
{
    UIElement* pElem = 0;

    if (mm == WM_NCCREATE)
    {
        // Get the pointer to the window from lpCreateParams which was set in CreateWindowEx
        SetWindowLongPtr(hh, GWLP_USERDATA, (LONG_PTR)((LPCREATESTRUCT)ll)->lpCreateParams);
    }

    // Get the pointer to the window
    pElem = (UIElement*) GetWindowLongPtr(hh, GWLP_USERDATA);

    // If we have the pointer, use the message handler of the window, else use DefWindowProc
    if (pElem)
        return pElem->MessageHandler(hh, mm, ww, ll);
    else
        return DefWindowProc(hh, mm, ww, ll);
}
