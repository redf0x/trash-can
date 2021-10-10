#ifndef ARRAYS_H
#define ARRAYS_H

#include <array>

template<typename T, std::size_t N, std::size_t... Idx>
constexpr std::array<T, N> to_stdarray(T (&arr)[N], std::index_sequence<Idx...>)
{
    return {arr[Idx]...};
}

template<typename T, std::size_t N>
constexpr std::array<T, N> to_stdarray(T (&arr)[N])
{
    return to_stdarray(arr, std::make_index_sequence<N>());
}

template<typename T,
         std::size_t LeftSize,
         std::size_t RightSize,
         std::size_t... LeftIdx,
         std::size_t... RightIdx>
constexpr std::array<T, LeftSize + RightSize> join(const std::array<T, LeftSize>&  lhs,
                                                   const std::array<T, RightSize>& rhs,
                                                   std::index_sequence<LeftIdx...>,
                                                   std::index_sequence<RightIdx...>)
{
    return {lhs[LeftIdx]..., rhs[RightIdx]...};
}

template<typename T, std::size_t LeftSize, std::size_t RightSize>
constexpr std::array<T, LeftSize + RightSize> join(const std::array<T, LeftSize>&  lhs,
                                                   const std::array<T, RightSize>& rhs)
{
    return join(lhs,
                rhs,
                std::make_index_sequence<LeftSize>(),
                std::make_index_sequence<RightSize>());
}

template<std::size_t NewSize, typename T, std::size_t OldSize, std::size_t... Idxs>
constexpr std::array<T, NewSize> resize(const std::array<T, OldSize>& arr,
                                        std::remove_const_t<T>        defaultValue,
                                        std::index_sequence<Idxs...>)
{
    return {((Idxs < OldSize) ? arr[Idxs] : defaultValue)...};
}

template<std::size_t NewSize, typename T, std::size_t OldSize>
constexpr std::array<T, NewSize> resize(const std::array<T, OldSize>& arr,
                                        std::remove_const_t<T>        defaultValue)
{
    constexpr std::size_t minSize = std::min(OldSize, NewSize);
    return resize<NewSize>(arr, defaultValue, std::make_index_sequence<minSize>());
}

template<typename T, std::size_t N, std::size_t... Idx>
constexpr bool are_equal(const std::array<T, N>& lhs,
                         const std::array<T, N>& rhs,
                         std::index_sequence<Idx...>)
{
    return ((lhs[Idx] == rhs[Idx]) && ...);
}

template<typename T, std::size_t N>
constexpr bool are_equal(const std::array<T, N>& lhs, const std::array<T, N>& rhs)
{
    return are_equal(lhs, rhs, std::make_index_sequence<N>());
}

#endif // ARRAYS_H
