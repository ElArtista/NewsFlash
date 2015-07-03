/*********************************************************************************************************************/
/*                                                  /===-_---~~~~~~~~~------____                                     */
/*                                                 |===-~___                _,-'                                     */
/*                  -==\\                         `//~\\   ~~~~`---.___.-~~                                          */
/*              ______-==|                         | |  \\           _-~`                                            */
/*        __--~~~  ,-/-==\\                        | |   `\        ,'                                                */
/*     _-~       /'    |  \\                      / /      \      /                                                  */
/*   .'        /       |   \\                   /' /        \   /'                                                   */
/*  /  ____  /         |    \`\.__/-~~ ~ \ _ _/'  /          \/'                                                     */
/* /-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`                                                      */
/*                   \_|      /        _)   ;  ),   __--~~                                                           */
/*                     '~~--_/      _-~/-  / \   '-~ \                                                               */
/*                    {\__--_/}    / \\_>- )<__\      \                                                              */
/*                    /'   (_/  _-~  | |__>--<__|      |                                                             */
/*                   |0  0 _/) )-~     | |__>--<__|     |                                                            */
/*                   / /~ ,_/       / /__>---<__/      |                                                             */
/*                  o o _//        /-~_>---<__-~      /                                                              */
/*                  (^(~          /~_>---<__-      _-~                                                               */
/*                 ,/|           /__>--<__/     _-~                                                                  */
/*              ,//('(          |__>--<__|     /                  .----_                                             */
/*             ( ( '))          |__>--<__|    |                 /' _---_~\                                           */
/*          `-)) )) (           |__>--<__|    |               /'  /     ~\`\                                         */
/*         ,/,'//( (             \__>--<__\    \            /'  //        ||                                         */
/*       ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'                                          */
/*     `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/                                                     */
/*   ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~                                                       */
/*    ;'( ')/ ,)(                              ~~~~~~~~~~                                                            */
/*   ' ') '( (/                                                                                                      */
/*     '   '  `                                                                                                      */
/*********************************************************************************************************************/
#ifndef _ANIMATION_HPP_
#define _ANIMATION_HPP_

#include <UIAnimation.h>
#include <atlbase.h>
#include <vector>
#include <functional>

class NotificationAnimationTimeEventHandler;

class Animator
{
    public:
        /// Constructor
        Animator();

        /// Destructor
        ~Animator();

        /// Disable copying
        Animator(const Animator&) = delete;
        Animator& operator=(const Animator&) = delete;

        /// Schedules a sample animation with given function as a callback
        void DoSampleAnimation(std::function<void()> cbAction);

    private:
        // The holder of the UIAnimationManager
        CComPtr<IUIAnimationManager> pAnimMgr;

        // The holder of the UIAnimationTimer
        CComPtr<IUIAnimationTimer> pAnimTmr;

        // The holder of the UITransitionLibrary
        CComPtr<IUIAnimationTransitionLibrary> pTransLib;

        // The timer event handler implementation
        NotificationAnimationTimeEventHandler* timeEvHandler;
};

class NotificationAnimationManagerEventHandler : public IUIAnimationManagerEventHandler
{
    public:
        /// Constructor
        NotificationAnimationManagerEventHandler();

        /// IUnknown Interface implementation
        ULONG __stdcall AddRef();
        ULONG __stdcall Release();
        HRESULT __stdcall QueryInterface(const IID& id, void** p);

        /// IUIAnimationManagerEventHandler
        HRESULT __stdcall OnManagerStatusChanged(
            UI_ANIMATION_MANAGER_STATUS newStatus,
            UI_ANIMATION_MANAGER_STATUS previousStatus
        );

    private:
        /// Reference counter of current object
        unsigned long ref;
};

class NotificationAnimationTimeEventHandler : public IUIAnimationTimerEventHandler
{
    public:
        /// Constructor
        NotificationAnimationTimeEventHandler();

        /// IUnknown Interface implementation
        ULONG __stdcall AddRef();
        ULONG __stdcall Release();
        HRESULT __stdcall QueryInterface(const IID& id, void** p);

        /// IUIAnimationTimerEventHandler Interface Implementation
        HRESULT __stdcall OnPostUpdate();
        HRESULT __stdcall OnPreUpdate();
        HRESULT __stdcall OnRenderingTooSlow(UINT32 framesPerSecond);

        /// Appends a callback function that is called when the animation ticks
        void AddCallbackAction(std::function<void()> cbAction);

    private:
        /// Holder of the callback functions
        std::vector<std::function<void()>> cbActions;

        /// Reference counter of current object
        unsigned long ref;
};

#endif // ! _ANIMATION_HPP_
