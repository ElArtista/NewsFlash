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
#ifndef _NOTIFICATION_SERVICE_HPP_
#define _NOTIFICATION_SERVICE_HPP_

#include <string>
#include <thread>
#include <unordered_map>
#include <memory>
#include "UIElement.hpp"
#include "NotificationDrawer.hpp"

struct NotificationData
{
    std::string msg;
    unsigned int lifetime;
};

class NotificationService : public UIElement
{
    public:
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

        /// The handle of the message window that is used to spawn the notifications
        HWND mHMsgWnd;

        /// The drawer that manages the lifetime, position and animations of the notifications
        std::unique_ptr<NotificationDrawer> mDrawer;

        /// The type of the message that is used when spawning a notification
        static const UINT WM_SPAWN_NOTIFICATION;
};

#endif // ! _NOTIFICATION_HOLDER_HPP_
