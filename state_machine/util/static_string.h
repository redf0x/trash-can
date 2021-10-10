#ifndef STATIC_STRING_H
#define STATIC_STRING_H

#include "arrays.h"

#include <cstdlib>
#include <iostream>
#include <string>

template<std::size_t N>
class static_string
{
  public:
    constexpr static_string(const char (&chars)[N])
      : chars(to_stdarray(chars))
    {
    }

    constexpr static_string(const std::array<const char, N>& chars)
      : chars(chars)
    {
    }

    template<std::size_t M>
    constexpr static_string<N + M - 1> operator+(const static_string<M>& rhs) const
    {
        return join(resize<N - 1>(chars, '\0'), rhs.chars);
    }

    constexpr bool operator==(const static_string<N>& rhs) const
    {
        return are_equal(chars, rhs.chars);
    }

    constexpr std::size_t length() const
    {
        return N - 1;
    }

    template<std::size_t TargetLen>
    constexpr static_string<TargetLen + 1> change_length(char fill) const
    {
        constexpr std::array<const char, 1> string_end{'\0'};
        return join(resize<TargetLen>(resize<N - 1>(chars, fill), fill), string_end);
    }

    template<std::size_t M>
    friend class static_string;

    constexpr const char* data() const
    {
        return chars.data();
    }

  private:
    std::array<const char, N> chars;
};

#endif // STATIC_STRING_H
