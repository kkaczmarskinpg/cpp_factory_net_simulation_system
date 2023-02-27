#ifndef NETSIM_PACKAGE_HPP
#define NETSIM_PACKAGE_HPP

/**
 * plik nagłówkowy "package.hpp" zawierający definicję klasy Package
*/

#include <set>
#include "types.hpp"

class Package {
public:
    Package();

    Package(ElementID id) : id_(id) {};

    Package(Package &&package) = default;

    Package(const Package &package) = delete;

    Package &operator=(Package &&package) noexcept;

    Package &operator=(const Package &package) = delete;

    ElementID get_id() const { return id_; };

    ~Package();

private:
    ElementID id_;
    static std::set<ElementID> freed_IDs;
    static std::set<ElementID> assigned_IDs;
};

#endif //NETSIM_PACKAGE_HPP