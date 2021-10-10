#include "types/resolve.h"
#include "types/types.h"
#include "types/util.h"
#include "util/static_string.h"

#include <functional>
#include <iostream>
#include <tuple>
#include <type_traits>
#include <variant>

template<typename T>
void debug(T&&)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
}

#define STRINGIFY_IMPL(TYPE)                                                     \
    [[maybe_unused]] static constexpr auto stringify(state_machine::types<TYPE>) \
    {                                                                            \
        return static_string{#TYPE};                                             \
    }

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

    constexpr static types<States...> get_state_types()
    {
        return {};
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

template<typename State>
static constexpr auto stringify(types<transition_to<State>>)
{
    return static_string{"transition_to<"} + stringify(types<State>{}) + static_string{">"};
}

struct nothing
{
    template<typename Machine, typename State, typename Event>
    void execute(Machine&, State&, const Event&)
    {
    }
};

static constexpr auto stringify(types<nothing>)
{
    return static_string{"nothing"};
}

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
static constexpr auto stringify(types<maybe<Action>>)
{
    return static_string{"maybe<"} + stringify(types<Action>{}) + static_string{">"};
}

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

STRINGIFY_IMPL(OpenEvent)
STRINGIFY_IMPL(CloseEvent)
STRINGIFY_IMPL(LockEvent)
STRINGIFY_IMPL(UnlockEvent)
STRINGIFY_IMPL(ClosedState)
STRINGIFY_IMPL(OpenState)
STRINGIFY_IMPL(LockedState)

struct Header
{
};

struct simple_stringifier
{
    constexpr auto operator()(state_machine::types<Header>) const
    {
        return static_string{""};
    }

    template<typename T>
    constexpr auto operator()(state_machine::types<T> type) const
    {
        return stringify(type);
    }
};

template<std::size_t width>
struct constant_width_stringifier
{
    constexpr auto operator()(state_machine::types<Header>) const
    {
        return static_string{""}.template change_length<width>(' ');
    }

    template<typename T>
    constexpr auto operator()(state_machine::types<T> type) const
    {
        return stringify(type).template change_length<width>(' ');
    }
};

template<typename Stringifier, typename State>
class generate_row
{
  public:
    constexpr generate_row(Stringifier str, state_machine::types<State>)
      : str(str)
    {
    }

    constexpr auto operator()(state_machine::types<State> state) const
    {
        return str(state);
    }

    template<typename Event>
    constexpr auto operator()(state_machine::types<Event>) const
    {
        auto action = state_machine::resolve_action{}(
          state_machine::types<state_machine::types<State, Event>>{});
        return static_string{" | "} + str(action);
    }

  private:
    const Stringifier str;
};

template<typename Stringifier>
class generate_row<Stringifier, Header>
{
  public:
    constexpr generate_row(Stringifier str, state_machine::types<Header>)
      : str(str)
    {
    }

    constexpr auto operator()(state_machine::types<Header> header) const
    {
        return str(header);
    }

    template<typename Event>
    constexpr auto operator()(state_machine::types<Event> event) const
    {
        return static_string{" | "} + str(event);
    }

  private:
    const Stringifier str;
};

template<typename Stringifier, typename... Events>
class generate_table
{
  public:
    constexpr generate_table(Stringifier str, sm::types<Events...>)
      : str(str)
    {
    }

    template<typename State>
    constexpr auto operator()(sm::types<State> state) const
    {
        return (sm::types<State, Events...>{} | sm::map_and_join{generate_row{str, state}})
               + static_string{"\n"};
    }

  private:
    const Stringifier str;
};

template<std::size_t X>
struct maximum
{
    template<std::size_t Y>
    constexpr auto operator+(maximum<Y>) const
    {
        return maximum<std::max(X, Y)>{};
    }

    static constexpr auto value()
    {
        return X;
    }
};

struct calculate_max_length
{
    template<typename T>
    constexpr auto operator()(sm::types<T> type)
    {
        return maximum<stringify(type).length()>{};
    }
};

template<typename... StateTypes, typename... EventTypes>
constexpr auto generate_transition_table(sm::types<StateTypes...> states,
                                         sm::types<EventTypes...> events)
{
    constexpr simple_stringifier stringifier;
    constexpr auto               result =
      (sm::types<Header>{} + states) | sm::map_and_join{generate_table{stringifier, events}};

    return result;
}

template<typename... StateTypes, typename... EventTypes>
constexpr auto generate_pretty_transition_table(sm::types<StateTypes...> states,
                                                sm::types<EventTypes...> events)
{
    constexpr auto actions = (states * events) | sm::map_and_join(sm::resolve_action{});
    constexpr auto max_width =
      (states + events + actions) | sm::map_and_join(calculate_max_length{});
    constexpr constant_width_stringifier<max_width.value()> stringifier{};
    constexpr auto                                          result =
      (sm::types<Header>{} + states) | sm::map_and_join{generate_table{stringifier, events}};

    return result;
}

int main()
{
    using SM = sm::state_machine<ClosedState, OpenState, LockedState>;

    std::cout << generate_transition_table(
                   SM::get_state_types(),
                   sm::types<OpenEvent, CloseEvent, LockEvent, UnlockEvent>{})
                   .data()
              << std::endl;
    std::cout << generate_pretty_transition_table(
                   SM::get_state_types(),
                   sm::types<OpenEvent, CloseEvent, LockEvent, UnlockEvent>{})
                   .data()
              << std::endl;

    SM sm{ClosedState{}, OpenState{}, LockedState{0}};

    sm.handle(LockEvent{1234});
    sm.handle(UnlockEvent{2});
    sm.handle(UnlockEvent{1234});

    return 0;
}
