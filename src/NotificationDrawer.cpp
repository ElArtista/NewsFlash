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

Notification::Notification(const std::string& msg, int initX, int initY)
    : mAnimator(nullptr)
{
    mNotificationWindow = std::make_unique<NotificationWindow>();
    mNotificationWindow->SetMessage(msg);
    mNotificationWindow->SetPosition(initX, initY);
    mNotificationWindow->Show(true);
}

Notification::~Notification()
{
    for (auto& p : mAnimMap)
        p.second.Cancel();
    mAnimMap.clear();
}

void Notification::SetAnimator(Animator* a)
{
    mAnimator = a;
}

std::pair<int, int> Notification::GetPosition() const
{
    return mNotificationWindow->GetPosition();
}

void Notification::SetPosition(int newX, int newY)
{
    // Cancel all previous active reposition animations and schedule new ones
    auto prevAnimX = mAnimMap.find("xrepos");
    if (prevAnimX != std::end(mAnimMap))
    {
        prevAnimX->second.Cancel();
        mAnimMap.erase(prevAnimX);
    }
    auto prevAnimY = mAnimMap.find("yrepos");
    if (prevAnimY != std::end(mAnimMap))
    {
        prevAnimY->second.Cancel();
        mAnimMap.erase(prevAnimY);
    }

    // Get non owning pointer, for passing to Animator cb
    NotificationWindow* rNw = mNotificationWindow.get();

    // Schedule move animation on X axis
    auto fx = [rNw](double p)
    {
        auto y = rNw->GetPosition().second;
        rNw->SetPosition(static_cast<int>(p), y);
    };
    Transition tx(1000, rNw->GetPosition().first, newX);
    Storyboard sx(tx);
    Animation ax(sx, fx);
    auto waX = mAnimator->DoSampleAnimation(ax);

    // Add X reposition on AnimationMap
    mAnimMap.insert(std::make_pair("xrepos", waX));

    // Schedule move animation on Y axis
    auto fy = [rNw](double p)
    {
        auto x = rNw->GetPosition().first;
        rNw->SetPosition(x, static_cast<int>(p));
    };
    Transition ty(1000, rNw->GetPosition().second, newY);
    Storyboard sy(ty);
    Animation ay(sy, fy);
    auto waY = mAnimator->DoSampleAnimation(ay);

    // Add Y reposition on AnimationMap
    mAnimMap.insert(std::make_pair("yrepos", waY));
}

NotificationId NotificationDrawer::sNWIdGen = 0;

void NotificationDrawer::SpawnNotification(const std::string& msg, unsigned int lifetime)
{
    (void) msg;

    // Create the id for the notification
    NotificationId id = sNWIdGen++;

    // Calculate and set the initial NotificationWindow position
    int xPos = 1200;
    int yPos = 0;
    if (!mVisibleList.empty())
    {
        auto fId = mVisibleList.front();
        auto front = mNotifications.find(fId);
        if (front != std::end(mNotifications))
            yPos = front->second->GetPosition().second - 250;
    }

    // Create the notification instance
    std::unique_ptr<Notification> notification = std::make_unique<Notification>(std::to_string(id), xPos, yPos);
    notification->SetAnimator(&mAnimator);

    // Add it to the notifications' map
    mNotifications.insert(std::make_pair(id, std::move(notification)));

    // Schedule spawn animation
    /*
    auto f1 = [rNw](double p) { rNw->SetAlpha(static_cast<unsigned int>(p)); };
    Transition t1(2500, 0, 100);
    Storyboard s(t1);
    Animation a(s, f1);
    auto fadeInAnim = mAnimator.DoSampleAnimation(a);
    mNotificationAnimations[id].insert(std::make_pair("spawn", fadeInAnim));
    */

    // Push notification id to the visible list
    mVisibleList.push_front(id);

    // Remove the last element of the queue if the size of the queue
    // reaches the visible notification limit
    if (mVisibleList.size() > 8)
        mVisibleList.pop_back();

    // Reschedule the animations according to the new positions
    for (std::size_t i = 0; i < mVisibleList.size(); ++i)
    {
        auto n = mNotifications.find(mVisibleList[i]);
        if (n != std::end(mNotifications))
        {
            Notification* rN = n->second.get();
            rN->SetPosition(xPos, i * 250);
        }
    }

    // Schedule the notification killer
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
    mVisibleList.clear();
}

