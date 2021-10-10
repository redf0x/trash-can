#ifndef TYPES_H
#define TYPES_H

#include <cstddef>

namespace state_machine
{

template<typename... Ts>
struct types
{
};

template<typename... Lhs, typename... Rhs>
constexpr auto operator+(types<Lhs...>, types<Rhs...>)
{
    return types<Lhs..., Rhs...>{};
}

template<typename Lhs, typename... Rhs>
constexpr auto operator*(types<Lhs>, types<Rhs...>)
{
    return types<types<Lhs, Rhs>...>{};
}

template<typename... Lhs, typename Rhs>
constexpr auto operator*(types<Lhs...>, Rhs rhs)
{
    return ((types<Lhs>{} * rhs) + ...);
}

template<typename... Ts, typename Operation>
constexpr auto operator|(types<Ts...>, Operation op)
{
    return op(types<Ts>{}...);
}

template<typename... Ts>
constexpr std::size_t size(types<Ts...>)
{
    return sizeof...(Ts);
}

} // namespace state_machine

#endif // TYPES_H
