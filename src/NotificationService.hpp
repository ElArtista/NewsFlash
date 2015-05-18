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
#ifndef _NOTIFICATION_HOLDER_HPP_
#define _NOTIFICATION_HOLDER_HPP_

#include <string>
#include <thread>
#include <unordered_map>
#include <memory>
#include "UIElement.hpp"
#include "NotificationWindow.hpp"

struct NotificationData
{
    std::string msg;
    unsigned int lifetime;
};

using NotificationWindowPtr = std::unique_ptr<NotificationWindow>;

class NotificationService : public UIElement
{
    public:
        /// Constructor
        NotificationService();

        /// Destructor
        ~NotificationService();

        /// Starts syncronous operation of the notification service
        void Run();

        /// Starts asyncronous operation of the notification service
        void RunAsync();

        /// Stops the operation of the notification service
        void Stop();

        /// Spawns notification window with the given message and lifetime in milliseconds
        void ShowNotification(const std::string& msg, unsigned int lifetime);

    private:
        /// Creates the message only window that will assist spawning the notifications
        void CreateMsgWnd();

        /// WndProc used by the message window that spawns the notifications
		LRESULT CALLBACK MessageHandler(HWND hh, UINT mm, WPARAM ww, LPARAM ll);

        /// The id of the thread that the notification creator window was created
        //DWORD mLoopThreadId;

        /// The handle of the message window that is used to spawn the notifications
        HWND mHMsgWnd;

        /// The container that holds the notification window instances
        std::unordered_map<UINT_PTR, std::unique_ptr<NotificationWindow>> mNotifications;

        /// The type of the message that is used when spawning a notification
        static const UINT WM_SPAWN_NOTIFICATION;

        /// The timer id value generator
        static UINT_PTR sTmrIdGen;
};

#endif // ! _NOTIFICATION_HOLDER_HPP_
