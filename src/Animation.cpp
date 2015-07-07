#include "Animation.hpp"
#include <algorithm>

///==============================================================
///= Transition
///==============================================================
Transition::Transition(unsigned long duration, double initVal, double finalVal)
    : mDuration(duration),
      mInitVal(initVal),
      mFinalVal(finalVal)
{
}

auto Transition::GetDuration() const -> unsigned long { return mDuration; }
auto Transition::GetInitVal() const -> double { return mInitVal; }
auto Transition::GetFinalVal() const -> double { return mFinalVal; }

///==============================================================
///= Storyboard
///==============================================================
auto begin(const Storyboard& s) -> Storyboard::const_iterator
{
    return std::begin(s.mTransitions);
}

auto end(const Storyboard& s) -> Storyboard::const_iterator
{
    return std::end(s.mTransitions);
}

///==============================================================
///= Animation
///==============================================================
Animation::Animation(Storyboard s, UpdateCallback updateCb)
    : mStoryboard(s),
      mUpdateCb(updateCb)
{
}

auto Animation::GetStoryboard() const -> const Storyboard& { return mStoryboard; }
auto Animation::GetUpdateCallback() const -> const UpdateCallback& { return mUpdateCb; }

///==============================================================
///= AnimationHandle
///==============================================================
AnimationHandle::AnimationHandle(std::weak_ptr<CComPtr<IUIAnimationStoryboard>> w)
{
    this->w = w;
}

bool AnimationHandle::IsValid() const
{
    return !w.expired();
}

void AnimationHandle::Cancel()
{
    if (IsValid())
    {
        auto p = w.lock();
        auto cp = *p;
        cp->Abandon();
    }
}

///==============================================================
///= Animator
///==============================================================
Animator::Animator()
{
    HRESULT hr;

    // =- UIAnimationManager
    // CoCreate the IUIAnimationManager.
    hr = pAnimMgr.CoCreateInstance(CLSID_UIAnimationManager, 0, CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
        return;

    // =- UIAnimationTimer
    // CoCreate the IUIAnimationTimer.
    hr = pAnimTmr.CoCreateInstance(CLSID_UIAnimationTimer, 0, CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
        return;
    
    // =- UIAnimationManager <-> UIAnimationTimer linking
    // Attach the timer to the manager by calling IUIAnimationManager::SetTimerUpdateHandler(),
    // passing an IUIAnimationTimerUpdateHandler. You can get this interface by querying the IUIAnimationTimer.
    // TODO: use IID_PPV_ARGS macro
    IUIAnimationTimerUpdateHandler* pTmrUpdater;
    hr = pAnimMgr->QueryInterface(__uuidof(**(&pTmrUpdater)), reinterpret_cast<void**>(&pTmrUpdater));

    pAnimTmr->SetTimerUpdateHandler(pTmrUpdater, UI_ANIMATION_IDLE_BEHAVIOR_DISABLE);
    if (FAILED(hr))
        return;
    pTmrUpdater->Release();

    // =- UIAnimationTransitionLibrary
    // CoCreate the IUIAnimationTransitionLibrary.
    hr = pTransLib.CoCreateInstance(CLSID_UIAnimationTransitionLibrary, 0, CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
        return;

    // Fire the animator timer
    pAnimTmr->Enable();
}

Animator::~Animator()
{
    if (SUCCEEDED(pAnimTmr->IsEnabled()))
        pAnimTmr->Disable();
}

// Animates given window with a custom transition
AnimationHandle Animator::DoSampleAnimation(const Animation& animation)
{
    HRESULT hr;

    // Create all the animation variables by calling IUIAnimationManager::CreateAnimationVariable().
    // There is an initial value, and after that, values can only be set by the transition.
    IUIAnimationVariable* pAnimVar = nullptr;
    auto initVal = (*(begin(animation.GetStoryboard()))).GetInitVal();
    hr = pAnimMgr->CreateAnimationVariable(initVal, &pAnimVar);

    // Create and assosiate the event handler for the animation variable
    NotificationAnimationVariableChangeHandler* animVarEvHandler = new NotificationAnimationVariableChangeHandler;
    animVarEvHandler->SetUpdateCallbackAction(animation.GetUpdateCallback());
    pAnimVar->SetVariableChangeHandler(animVarEvHandler);
    animVarEvHandler->Release();

    // Create a storyboard by calling IUIAnimationManager::CreateStoryboard().
    // A storyboard is a storage that contains all the variables and their animation transitions.
    CComPtr<IUIAnimationStoryboard> pStoryboard = nullptr;
    hr = pAnimMgr->CreateStoryboard(&pStoryboard);
    IUIAnimationStoryboard* raw = pStoryboard.p;

    // Store the storyboard to the alive animations
    auto aliveAnim = std::make_shared<CComPtr<IUIAnimationStoryboard>>(pStoryboard);
    mAliveAnimations.push_back(aliveAnim);

    // Set the Storyboard event handler to take the end of the animation events
    NotificationAnimationEventHandler* notificationAnimEvHandler = new NotificationAnimationEventHandler;
    notificationAnimEvHandler->SetFinishCallback(
        [this, raw]()
        {
            auto a = std::find_if(std::begin(mAliveAnimations), std::end(mAliveAnimations),
                [raw](const std::shared_ptr<CComPtr<IUIAnimationStoryboard>>& cp) -> bool
                {
                    return (*cp).p == raw;
                }
            );
            if (a != std::end(mAliveAnimations))
                mAliveAnimations.erase(a);
        }
    );
    pStoryboard->SetStoryboardEventHandler(notificationAnimEvHandler);
    notificationAnimEvHandler->Release();

    // Call IUIAnimationTransitionLibrary methods to create standard transitions. 
    for (const auto& t : animation.GetStoryboard())
    {
        IUIAnimationTransition* pTransition = nullptr;
        hr = pTransLib->CreateLinearTransition(t.GetDuration() / 1000.0, t.GetFinalVal(), &pTransition);
        hr = pStoryboard->AddTransition(pAnimVar, pTransition);
        pTransition->Release();
    }
    pAnimVar->Release();

    // Get the current "animation time" by calling IUIAnimationTimer::GetTime(), 
    // then pass it to IUIAnimationStoryboard::Schedule(). This starts the animation.
    UI_ANIMATION_SECONDS secs = 0;
    pAnimTmr->GetTime(&secs);
    pStoryboard->Schedule(secs);

    // If animation timer was deactivated, activate him again
    if (FAILED(pAnimTmr->IsEnabled()))
        pAnimTmr->Enable();

    return AnimationHandle(aliveAnim);
}

///==============================================================
///= NotificationAnimationEventHandler
///==============================================================
NotificationAnimationEventHandler::NotificationAnimationEventHandler() { ref = 1; }
ULONG __stdcall NotificationAnimationEventHandler::AddRef() { return ++ref; }
ULONG __stdcall NotificationAnimationEventHandler::Release()
{
    ULONG nRef = --ref;
    if (nRef == 0)
        delete this;
    return nRef;
}

HRESULT __stdcall NotificationAnimationEventHandler::QueryInterface(const IID& id, void** p)
{
    if (id == __uuidof(IUnknown) ||
        id == __uuidof(IUIAnimationStoryboardEventHandler))
    {
        *p = this;
        AddRef();
        return NOERROR;
    }

    *p = nullptr;
    return E_NOINTERFACE;
}

HRESULT __stdcall NotificationAnimationEventHandler::OnStoryboardStatusChanged(
    IUIAnimationStoryboard* storyboard,
    UI_ANIMATION_STORYBOARD_STATUS newStatus,
    UI_ANIMATION_STORYBOARD_STATUS previousStatus
)
{
    UNREFERENCED_PARAMETER(storyboard);
    UNREFERENCED_PARAMETER(previousStatus);
    if (newStatus == UI_ANIMATION_STORYBOARD_FINISHED ||
        newStatus == UI_ANIMATION_STORYBOARD_CANCELLED ||
        newStatus == UI_ANIMATION_STORYBOARD_TRUNCATED)
    {
        if (mFinishCb)
            mFinishCb();
    }
    return S_OK;
}

HRESULT __stdcall NotificationAnimationEventHandler::OnStoryboardUpdated(
    IUIAnimationStoryboard* storyboard
)
{
    UNREFERENCED_PARAMETER(storyboard);
    return S_OK;
}

void NotificationAnimationEventHandler::SetFinishCallback(FinishCallback finishCb)
{
    mFinishCb = finishCb;
}

///==============================================================
///= NotificationAnimationVariableChangeHandler
///==============================================================
NotificationAnimationVariableChangeHandler::NotificationAnimationVariableChangeHandler() { ref = 1; }
ULONG __stdcall NotificationAnimationVariableChangeHandler::AddRef() { return ++ref; }
ULONG __stdcall NotificationAnimationVariableChangeHandler::Release()
{
    ULONG nRef = --ref;
    if (nRef == 0)
        delete this;
    return nRef;
}

HRESULT __stdcall NotificationAnimationVariableChangeHandler::QueryInterface(const IID& id, void** p)
{
    if (id == __uuidof(IUnknown) ||
        id == __uuidof(IUIAnimationVariableChangeHandler))
    {
        *p = this;
        AddRef();
        return NOERROR;
    }

    *p = nullptr;
    return E_NOINTERFACE;
}

HRESULT __stdcall NotificationAnimationVariableChangeHandler::OnValueChanged(
    IUIAnimationStoryboard *storyboard,
    IUIAnimationVariable   *variable,
    DOUBLE                 newValue,
    DOUBLE                 previousValue
)
{
    UNREFERENCED_PARAMETER(storyboard);
    UNREFERENCED_PARAMETER(variable);
    UNREFERENCED_PARAMETER(previousValue);

    if (updateCb)
        updateCb(newValue); 
    return S_OK;
}

void NotificationAnimationVariableChangeHandler::SetUpdateCallbackAction(UpdateCallback updateCb)
{
    this->updateCb = updateCb;
}
