#include "package.hpp"

std::set<ElementID> Package::freed_IDs;
std::set<ElementID> Package::assigned_IDs;

Package &Package::operator=(Package &&package) noexcept {
    id_ = package.id_;
    return (*this);
}

Package::Package() {
    if (Package::freed_IDs.empty()) {
        if (Package::assigned_IDs.empty()) {
            id_ = 1;
        } else {
            id_ = *(Package::assigned_IDs.rbegin()) + 1;
        }
    } else {
        id_ = *Package::freed_IDs.begin();
        Package::freed_IDs.erase(Package::freed_IDs.begin());
    }
    Package::assigned_IDs.insert(id_);
}

Package::~Package() {
    freed_IDs.insert(id_);
    assigned_IDs.erase(id_);
}
