#include "NotificationDrawer.hpp"

unsigned long NotificationDrawer::sNWIdGen = 0;

void NotificationDrawer::SpawnNotification(const std::string& msg, unsigned int lifetime)
{
    // Create the id for the notification
    unsigned long id = sNWIdGen++;

    // Create the notification instance
    std::unique_ptr<NotificationWindow> nw = std::make_unique<NotificationWindow>();

    // Set its properties
    nw->SetMessage(msg);

    // Schedule spawn animation
    NotificationWindow* rNw = nw.get();
    auto f1 = [rNw](double p)
    {
        rNw->SetAlpha(p);
    };
    nw->SetAlpha(0);

    Transition t1(1000, 0, 100);
    Transition t2(2000, 100, 0);
    Storyboard s(t1, t2);
    Animation a(s, f1);

    mAnimator.DoSampleAnimation(a);

    // Add it to the notifications' map
    mNotifications.insert(std::make_pair(id, std::move(nw)));
}

void NotificationDrawer::Clear()
{
    mNotifications.clear();
}
