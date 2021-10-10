#include <functional>
#include <iostream>
#include <tuple>
#include <variant>

namespace state_machine
{

template<typename Event, typename Action>
struct on
{
    Action handle(const Event&) const
    {
        return Action{};
    }
};

template<typename... States>
class state_machine
{
  public:
    state_machine() = default;
    state_machine(States... states)
      : states(std::move(states)...)
    {
    }

    template<typename State>
    State& transition()
    {
        State& state = std::get<State>(states);
        currentState = &state;
        return state;
    }

    template<typename Event>
    void handle(const Event& event)
    {
        handle_by(event, *this);
    }

    template<typename Event, typename Machine>
    void handle_by(const Event& event, Machine& machine)
    {
        auto passEventToState = [&machine, &event](auto statePtr) {
            auto action = statePtr->handle(event);
            action.execute(machine, *statePtr, event);
        };

        std::visit(passEventToState, currentState);
    }

  private:
    std::tuple<States...>    states;
    std::variant<States*...> currentState{&std::get<0>(states)};
};

template<typename TargetState>
struct transition_to
{
    template<typename Machine, typename State, typename Event>
    void execute(Machine& machine, State& prevState, const Event& event)
    {
        leave(prevState, event);
        TargetState& newState = machine.template transition<TargetState>();
        enter(newState, event);
    }

  private:
    void leave(...) {}

    template<typename State, typename Event>
    auto leave(State& state, const Event& event) -> decltype(state.on_leave(event))
    {
        return state.on_leave(event);
    }

    void enter(...) {}

    template<typename State, typename Event>
    auto enter(State& state, const Event& event) -> decltype(state.on_enter(event))
    {
        return state.on_enter(event);
    }
};

struct nothing
{
    template<typename Machine, typename State, typename Event>
    void execute(Machine&, State&, const Event&)
    {
    }
};

template<typename... Actions>
struct one_of
{
    template<typename T>
    one_of(T&& arg)
      : options(std::forward<T>(arg))
    {
    }

    template<typename Machine, typename State, typename Event>
    void execute(Machine& machine, State& state, const Event& event)
    {
        std::visit(
          [&machine, &state, &event](auto& action) {
              action.execute(machine, state, event);
          },
          options);
    }

  private:
    std::variant<Actions...> options;
};

template<typename Action>
struct maybe : public one_of<Action, nothing>
{
    using one_of<Action, nothing>::one_of;
};

template<typename Action>
struct by_default
{
    template<typename Event>
    Action handle(const Event&) const
    {
        return Action{};
    }
};

template<typename... Handlers>
struct will : Handlers...
{
    using Handlers::handle...;
};

} // namespace state_machine

using namespace std;
namespace sm = state_machine;

struct OpenEvent
{
};

struct CloseEvent
{
};

struct LockEvent
{
    uint32_t newKey;
};

struct UnlockEvent
{
    uint32_t key;
};

struct OpenState;
struct ClosedState;
struct LockedState;

struct ClosedState
  : public sm::will<sm::by_default<sm::nothing>,
                    sm::on<LockEvent, sm::transition_to<LockedState>>,
                    sm::on<OpenEvent, sm::transition_to<OpenState>>>
{
};

struct OpenState
  : public sm::will<sm::by_default<sm::nothing>, sm::on<CloseEvent, sm::transition_to<ClosedState>>>
{
};

struct LockedState : public sm::by_default<sm::nothing>
{
    using sm::by_default<sm::nothing>::handle;

    LockedState(uint32_t key)
      : key(key)
    {
    }

    void on_enter(const LockEvent& e)
    {
        key = e.newKey;
    }

    sm::maybe<sm::transition_to<ClosedState>> handle(const UnlockEvent& e)
    {
        if (e.key == key)
        {
            return sm::transition_to<ClosedState>{};
        }
        return sm::nothing{};
    }

  private:
    uint32_t key;
};

int main()
{
    sm::state_machine<ClosedState, OpenState, LockedState> sm{ClosedState{},
                                                              OpenState{},
                                                              LockedState{0}};

    sm.handle(LockEvent{1234});
    sm.handle(UnlockEvent{2});
    sm.handle(UnlockEvent{1234});

    return 0;
}
