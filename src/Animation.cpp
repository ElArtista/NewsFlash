#include "Animation.hpp"
#include <algorithm>

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

    // Allocate the manager event handler
    NotificationAnimationManagerEventHandler* animMgrEvHandler = new NotificationAnimationManagerEventHandler;

    // Implement an IUIAnimationManagerEventHandler event handler and pass it to IUIAnimationManager::SetManagerEventHandler.
    hr = pAnimMgr->SetManagerEventHandler(animMgrEvHandler);

    // =- UIAnimationTimer
    // CoCreate the IUIAnimationTimer.
    hr = pAnimTmr.CoCreateInstance(CLSID_UIAnimationTimer, 0, CLSCTX_INPROC_SERVER);
    if (FAILED(hr))
        return;
    
    // Allocate the timer event handler
    timeEvHandler = new NotificationAnimationTimeEventHandler;

    // Pass the IUIAnimationTimerEventHandler event handler implementation using IUIAnimationTimer::SetTimerEventHandler.
    hr = pAnimTmr->SetTimerEventHandler(timeEvHandler);

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
    pAnimTmr->Disable();
}

// Animates given window with a custom transition
void Animator::DoSampleAnimation(std::function<void()> cbAction)
{
    HRESULT hr;

    // Store the animation callback action
    timeEvHandler->AddCallbackAction(cbAction);

    // Create all the animation variables by calling IUIAnimationManager::CreateAnimationVariable().
    // There is an initial value, and after that, values can only be set by the transition.
    IUIAnimationVariable* pAnimVar = nullptr;
    hr = pAnimMgr->CreateAnimationVariable(0.0f, &pAnimVar);

    // Create a storyboard by calling IUIAnimationManager::CreateStoryboard().
    // A storyboard is a storage that contains all the variables and their animation transitions.
    IUIAnimationStoryboard* pStoryboard = nullptr;
    hr = pAnimMgr->CreateStoryboard(&pStoryboard);

    // Call IUIAnimationTransitionLibrary methods to create standard transitions. 
    // The application here creates a linear transition variable (CreateLinearTransition) which represents the X axis in a sinusoid oscillation,
    // and a sinusoidal transition variable (CreateSinusoidalTransitionFromRange) which represents the Y axis.
    IUIAnimationTransition* pTransition = nullptr;
    hr = pTransLib->CreateLinearTransition(10, 100, &pTransition);
    hr = pStoryboard->AddTransition(pAnimVar, pTransition);
    pTransition->Release();

    // Get the current "animation time" by calling IUIAnimationTimer::GetTime(), 
    // then pass it to IUIAnimationStoryboard::Schedule(). This starts the animation.
    UI_ANIMATION_SECONDS secs = 0;
    if (FAILED(pAnimTmr->GetTime(&secs)))
        return;
    hr = pStoryboard->Schedule(secs);
}

///==============================================================
///= NotificationAnimationManagerEventHandler
///==============================================================
NotificationAnimationManagerEventHandler::NotificationAnimationManagerEventHandler() { ref = 1; }
ULONG __stdcall NotificationAnimationManagerEventHandler::AddRef() { return ++ref; }
ULONG __stdcall NotificationAnimationManagerEventHandler::Release()
{
    ULONG nRef = --ref;
    if (nRef == 0)
        delete this;
    return nRef;
}

HRESULT __stdcall NotificationAnimationManagerEventHandler::QueryInterface(const IID& id, void** p)
{
    if (id == __uuidof(IUnknown) ||
        id == __uuidof(IUIAnimationManagerEventHandler))
    {
        *p = this;
        AddRef();
        return NOERROR;
    }

    *p = nullptr;
    return E_NOINTERFACE;
}

HRESULT __stdcall NotificationAnimationManagerEventHandler::OnManagerStatusChanged(
    UI_ANIMATION_MANAGER_STATUS newStatus,
    UI_ANIMATION_MANAGER_STATUS previousStatus
)
{
    UNREFERENCED_PARAMETER(previousStatus);
    if (newStatus == UI_ANIMATION_MANAGER_IDLE)
    {
        // When the animation is done, you are notified. Recreate stuff to restart the animation.
    }
    return S_OK;
}

///==============================================================
///= NotificationAnimationTimeEventHandler
///==============================================================
NotificationAnimationTimeEventHandler::NotificationAnimationTimeEventHandler() { ref = 1; }
ULONG __stdcall NotificationAnimationTimeEventHandler::AddRef() { return ++ref; }
ULONG __stdcall NotificationAnimationTimeEventHandler::Release()
{
    ULONG nRef = --ref;
    if (nRef == 0)
        delete this;
    return nRef;
}

HRESULT __stdcall NotificationAnimationTimeEventHandler::QueryInterface(const IID& id,void**p)
{
    if (id == __uuidof(IUnknown) ||
        id == __uuidof(IUIAnimationTimerEventHandler))
    {
        *p = this;
        AddRef();
        return NOERROR;
    }

    *p = nullptr;
    return E_NOINTERFACE;
}

HRESULT __stdcall NotificationAnimationTimeEventHandler::OnPostUpdate()
{
    return S_OK;
}

HRESULT __stdcall NotificationAnimationTimeEventHandler::OnPreUpdate()
{
    // Your event handler OnPreUpdate is called when the variables are changed. 
    // Use IUIAnimationVariable::GetValue() to query the values of the variables, then use them to draw the objects based on their value.
    
    // Request the y variable value
    /*
    DOUBLE v0 = 0;
    if (amv[0])
        amv[0]->GetValue(&v0);
    */
    for (auto& a : cbActions)
        a();

    return S_OK;
}

HRESULT __stdcall NotificationAnimationTimeEventHandler::OnRenderingTooSlow(UINT32 framesPerSecond)
{
    UNREFERENCED_PARAMETER(framesPerSecond);
    return E_NOTIMPL;
}

void NotificationAnimationTimeEventHandler::AddCallbackAction(std::function<void()> cbAct)
{
    cbActions.push_back(cbAct);
}

