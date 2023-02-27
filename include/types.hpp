#ifndef NETSIM_TYPES_HPP
#define NETSIM_TYPES_HPP

/**
 * plik nagłówkowy "types.hpp" zawierający definicję aliasu ElementID
*/
#include <list>
#include <functional>

using ElementID = int;
using Time = int;
using TimeOffset = int;
using ProbabilityGenerator = std::function<double()>;

#endif //NETSIM_TYPES_HPP