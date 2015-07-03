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
    auto f = [rNw]() 
    {
        unsigned int prevAlpha = rNw->GetAlpha();
        prevAlpha += 5;
        if (prevAlpha < 100)
            rNw->SetAlpha(prevAlpha);
    };
    mAnimator.DoSampleAnimation(f);

    // Add it to the notifications' map
    mNotifications.insert(std::make_pair(id, std::move(nw)));
}

void NotificationDrawer::Clear()
{
    mNotifications.clear();
}
