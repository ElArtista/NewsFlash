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
///= Storyboard
///==============================================================
Animation::Animation(Storyboard s, UpdateCallback updateCb)
    : mStoryboard(s),
      mUpdateCb(updateCb)
{
}

auto Animation::GetStoryboard() const -> const Storyboard& { return mStoryboard; }
auto Animation::GetUpdateCallback() const -> const UpdateCallback& { return mUpdateCb; }

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
void Animator::DoSampleAnimation(const Animation& animation)
{
    HRESULT hr;

    // Create all the animation variables by calling IUIAnimationManager::CreateAnimationVariable().
    // There is an initial value, and after that, values can only be set by the transition.
    IUIAnimationVariable* pAnimVar = nullptr;
    hr = pAnimMgr->CreateAnimationVariable(0.0f, &pAnimVar);

    // Create and assosiate the event handler for the animation variable
    NotificationAnimationVariableChangeHandler* animVarEvHandler = new NotificationAnimationVariableChangeHandler;
    animVarEvHandler->SetUpdateCallbackAction(animation.GetUpdateCallback());
    pAnimVar->SetVariableChangeHandler(animVarEvHandler);

    // Create a storyboard by calling IUIAnimationManager::CreateStoryboard().
    // A storyboard is a storage that contains all the variables and their animation transitions.
    IUIAnimationStoryboard* pStoryboard = nullptr;
    hr = pAnimMgr->CreateStoryboard(&pStoryboard);

    // Call IUIAnimationTransitionLibrary methods to create standard transitions. 
    for (const auto& t : animation.GetStoryboard())
    {
        IUIAnimationTransition* pTransition = nullptr;
        hr = pTransLib->CreateLinearTransition(t.GetDuration() / 1000.0, t.GetFinalVal(), &pTransition);
        hr = pStoryboard->AddTransition(pAnimVar, pTransition);
        pTransition->Release();
    }

    // Get the current "animation time" by calling IUIAnimationTimer::GetTime(), 
    // then pass it to IUIAnimationStoryboard::Schedule(). This starts the animation.
    UI_ANIMATION_SECONDS secs = 0;
    if (FAILED(pAnimTmr->GetTime(&secs)))
        return;
    pStoryboard->Schedule(secs);

    // If animation timer was deactivated, activate him again
    if (FAILED(pAnimTmr->IsEnabled()))
        pAnimTmr->Enable();
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
