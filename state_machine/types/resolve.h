#ifndef RESOLVE_H
#define RESOLVE_H

#include "types.h"

#include <utility>

namespace state_machine
{

struct resolve_action
{
    template<typename State, typename Event>
    constexpr auto operator()(types<State, Event>)
    {
        using Action = decltype(std::declval<State>().handle(std::declval<Event>()));
        return types<Action>{};
    }

    template<typename State, typename Event>
    constexpr auto operator()(types<types<State, Event>>)
    {
        return (*this)(types<State, Event>{});
    }
};

} // namespace state_machine

#endif // RESOLVE_H
