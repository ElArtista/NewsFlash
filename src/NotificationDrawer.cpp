#include "NotificationDrawer.hpp"
#include <thread>

template<typename Callable>
void runAfter(Callable c, unsigned long millisec)
{
    std::thread t(
        [=]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(millisec));
            c();
        }
    );
    t.detach();
}

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
        rNw->SetAlpha(static_cast<unsigned int>(p));
    };
    nw->SetAlpha(0);

    Transition t1(1000, 0, 100);
    Transition t2(2000, 100, 0);
    Storyboard s(t1, t2);
    Animation a(s, f1);

    mAnimator.DoSampleAnimation(a);

    // Add it to the notifications' map
    mNotifications.insert(std::make_pair(id, std::move(nw)));

    runAfter(
        [this, id]()
        {
            auto n = mNotifications.find(id);
            mNotifications.erase(n);
        },
        lifetime
    );
}

void NotificationDrawer::Clear()
{
    mNotifications.clear();
}
