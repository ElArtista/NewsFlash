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
#ifndef _NOTIFICATION_DRAWER_HPP_
#define _NOTIFICATION_DRAWER_HPP_

#include <string>
#include <unordered_map>
#include <memory>
#include <deque>
#include <list>
#include "NotificationWindow.hpp"
#include "Animation.hpp"

// Abstracting the id type
using NotificationId = unsigned long;

// Type that holds weak handles to the running animations of a Notification
using AnimationMap = std::unordered_map<std::string, AnimationHandle>;

class Notification
{
    public:
        /// Constructor
        explicit Notification(const std::string& msg, int initX, int initY);

        /// Destructor
        ~Notification();

        /// Sets the Animator object to use for the various window animations of the Notification
        void SetAnimator(Animator* a);

        /// Retrieves the notification position
        std::pair<int, int> GetPosition() const;

        /// Changes notification position gradually using generated animation from animator
        void SetPosition(int newX, int newY);

    private:
        /// The representation of the Notification as a Window
        std::unique_ptr<NotificationWindow> mNotificationWindow;

        /// The object that animates the Notification in its various actions
        Animator* mAnimator;

        /// Caches various animation weak handles
        AnimationMap mAnimMap;
};

class NotificationDrawer
{
    public:
        /// Creates a notification window with the given properties
        void SpawnNotification(const std::string& msg, unsigned int lifetime);

        /// Clears drawer from all the notifications
        void Clear();

    private:
        /// The container that holds the notification window instances that are alive
        std::unordered_map<NotificationId, std::unique_ptr<Notification>> mNotifications;

        /// The queue that keeps the currently visible notification id, in the order they are visible
        std::deque<NotificationId> mVisibleList;

        /// The Animator that schedules the various animation effects
        Animator mAnimator;

        /// The id value generator
        static NotificationId sNWIdGen;
};

#endif // ! _NOTIFICATION_DRAWER_HPP_

