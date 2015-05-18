#ifndef _UIELEMENT_HPP_
#define _UIELEMENT_HPP_

#include <tchar.h>
#include <Windows.h>

class UIElement
{
    protected:
        /// The real message handler that will be overriden by all the subclasses
        virtual LRESULT CALLBACK MessageHandler(HWND hh, UINT mm, WPARAM ww, LPARAM ll) = 0;
    
    public:
        /// The static proxy message handler that will be used when registering a window class
        static LRESULT CALLBACK ProxyMsgHandler(HWND hh, UINT mm, WPARAM ww, LPARAM ll);
};

#endif
