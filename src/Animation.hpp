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
#ifndef _ANIMATION_HPP_
#define _ANIMATION_HPP_

#include <UIAnimation.h>
#include <atlbase.h>
#include <list>
#include <vector>
#include <functional>
#include <memory>

// Alias of the update callback signature for convenience
using UpdateCallback = std::function<void(double)>;

class Transition
{
    public:
        /// Constructor
        Transition(unsigned long duration, double initVal, double finalVal);

        /// Retrieves the transition duration in milliseconds
        unsigned long GetDuration() const;

        /// Retrieves the initial value of the animation variable
        double GetInitVal() const;

        /// Retrieves the final value of the animation variable
        double GetFinalVal() const;

    private:
        /// The transition duration in millisecconds
        unsigned long mDuration;

        /// The initial value
        double mInitVal;

        /// The final value
        double mFinalVal;

        /// TODO: Add interpolator type
};

class Storyboard
{
    public:
        /// Constructor that takes n+ Transitions
        template<class... Transitions>
        Storyboard(Transition x, Transitions... xs) : Storyboard(xs...)
        {
            mTransitions.push_front(x);
        };

        /// Iterator access for use in for range loops
        using const_iterator = std::list<Transition>::const_iterator;
        friend auto begin(const Storyboard&) -> const_iterator;
        friend auto end(const Storyboard&) -> const_iterator;
    private:
        /// No argument Constructor, hidden prevent Storyboards created without Transitions
        Storyboard() = default;

        /// The holder of the Transitions
        std::list<Transition> mTransitions;
};

/// Const access iterators for Storyboard
auto begin(const Storyboard&) -> Storyboard::const_iterator;
auto end(const Storyboard&) -> Storyboard::const_iterator;

class Animation
{
    public:
        /// Constructor
        Animation(Storyboard s, UpdateCallback updateCb);

        /// Retrieves the Animation Storyboard that contains the Animation's Transitions
        const Storyboard& GetStoryboard() const;

        /// Retrieves the Animation's UpdtateCallback that is called when the animation value changes
        const UpdateCallback& GetUpdateCallback() const;

    private:
        /// The UpdateCallback holder
        UpdateCallback mUpdateCb;

        /// The Storyboard holder
        Storyboard mStoryboard;
};

class AnimationHandle
{
    public:
        /// Constructor
        AnimationHandle(std::weak_ptr<CComPtr<IUIAnimationStoryboard>> w);

        /// Checks if the current handle is still valid
        bool IsValid() const;

        /// Cancels the Animation that the handle points to if handle is still valid
        void Cancel();

    private:
        /// Weak reference to the object that represents a running Animation
        std::weak_ptr<CComPtr<IUIAnimationStoryboard>> w;
};

class Animator
{
    public:
        /// Constructor
        Animator();

        /// Destructor
        ~Animator();

        /// Disable copying
        Animator(const Animator&) = delete;
        Animator& operator=(const Animator&) = delete;

        /// Schedules a sample animation
        AnimationHandle DoSampleAnimation(const Animation& a);

    private:
        // The holder of the UIAnimationManager
        CComPtr<IUIAnimationManager> pAnimMgr;

        // The holder of the UIAnimationTimer
        CComPtr<IUIAnimationTimer> pAnimTmr;

        // The holder of the UITransitionLibrary
        CComPtr<IUIAnimationTransitionLibrary> pTransLib;

        // Keeps the alive animation instances as handles
        std::vector<std::shared_ptr<CComPtr<IUIAnimationStoryboard>>> mAliveAnimations;
};

using FinishCallback = std::function<void()>;

class NotificationAnimationEventHandler : public IUIAnimationStoryboardEventHandler
{
    public:
        /// Constructor
        NotificationAnimationEventHandler();

        /// IUnknown Interface implementation
        ULONG __stdcall AddRef();
        ULONG __stdcall Release();
        HRESULT __stdcall QueryInterface(const IID& id, void** p);

        /// IUIAnimationStoryboardEventHandler Interface implementation
        HRESULT __stdcall OnStoryboardStatusChanged(
            IUIAnimationStoryboard* storyboard,
            UI_ANIMATION_STORYBOARD_STATUS newStatus,
            UI_ANIMATION_STORYBOARD_STATUS previousStatus
        );
        HRESULT __stdcall OnStoryboardUpdated(
            IUIAnimationStoryboard* storyboard
        );

        // Sets the callback to be called when the animation ends
        void SetFinishCallback(FinishCallback finishCb);

    private:
        /// Holder of the finish callback
        FinishCallback mFinishCb;

        /// Reference counter of current object
        unsigned long ref;
};

class NotificationAnimationVariableChangeHandler : public IUIAnimationVariableChangeHandler
{
    public:
        /// Constructor
        NotificationAnimationVariableChangeHandler();

        /// IUnknown Interface implementation
        ULONG __stdcall AddRef();
        ULONG __stdcall Release();
        HRESULT __stdcall QueryInterface(const IID& id, void** p);

        /// IUIAnimationVariableChangeHandler Interface Implementation
        HRESULT __stdcall OnValueChanged(
            IUIAnimationStoryboard *storyboard,
            IUIAnimationVariable   *variable,
            DOUBLE                 newValue,
            DOUBLE                 previousValue
        );

        /// Sets the callback function that is called when the animation ticks
        void SetUpdateCallbackAction(UpdateCallback updateCb);

    private:
        /// Holder of the update callback
        UpdateCallback updateCb;

        /// Reference counter of current object
        unsigned long ref;
};

#endif // ! _ANIMATION_HPP_
