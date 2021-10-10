#ifndef UTIL_H
#define UTIL_H

#include "types.h"

namespace state_machine
{

template<typename Operation>
class map_and_join
{
  public:
    constexpr map_and_join(Operation op)
      : operation(op)
    {
    }

    template<typename... Ts>
    constexpr auto operator()(types<Ts>... rhs)
    {
        return (operation(rhs) + ...);
    }

  private:
    Operation operation;
};

} // namespace state_machine

#endif // UTIL_H
