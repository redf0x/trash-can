#include <functional>
#include <iostream>
#include <tuple>
#include <variant>

namespace state_machine
{

template<typename... States>
class state_machine
{
  public:
    template<typename State>
    void transition()
    {
        currentState = &std::get<State>(states);
    }

    template<typename Event>
    void handle(const Event& event)
    {
        auto passEventToState = [this, &event](auto statePtr) {
            statePtr->handle(event).execute(*this);
        };

        std::visit(passEventToState, currentState);
    }

  private:
    std::tuple<States...>    states;
    std::variant<States*...> currentState{&std::get<0>(states)};
};

template<typename State>
struct transition_to
{
    template<typename Machine>
    void execute(Machine& machine)
    {
        machine.template transition<State>();
    }
};

struct nothing
{
    template<typename Machine>
    void execute(Machine&)
    {
    }
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

struct OpenState;
struct ClosedState;

struct ClosedState
{
    sm::transition_to<OpenState> handle(const OpenEvent&) const
    {
        cout << "Open" << std::endl;
        return {};
    }

    sm::nothing handle(const CloseEvent&) const
    {
        cout << "Cannot close; Already closed." << std::endl;
        return {};
    }
};

struct OpenState
{
    sm::nothing handle(const OpenEvent&) const
    {
        cout << "Cannot open. Already open." << std::endl;
        return {};
    }

    sm::transition_to<ClosedState> handle(const CloseEvent&) const
    {
        cout << "Closing" << std::endl;
        return {};
    }
};

int main()
{
    sm::state_machine<ClosedState, OpenState> sm;

    sm.handle(OpenEvent{});
    sm.handle(CloseEvent{});
    sm.handle(CloseEvent{});
    sm.handle(OpenEvent{});

    return 0;
}
