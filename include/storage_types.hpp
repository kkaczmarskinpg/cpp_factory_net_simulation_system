#ifndef NETSIM_STORAGE_TYPES_HPP
#define NETSIM_STORAGE_TYPES_HPP

/**
 * plik nagłówkowy "storage_types.hpp" zawierający definicje klas IPackageStockpile, IPackageQueue
 * i PackageQueue oraz typu wyliczeniowego PackageQueueType
*/

#include <list>
#include "package.hpp"
#include "types.hpp"
#include <cstddef>

enum class PackageQueueType {
    FIFO,
    LIFO
};


class IPackageStockpile {
    /*!
     * klasa IPackageStockpile jest klasą abstrakcyjną, która zawiera metody do obsługi magazynu paczek
     */
public:
    using const_iterator = std::list<Package>::const_iterator;

    virtual void push(Package &&) = 0;

    virtual bool empty() const = 0;

    virtual size_t size() const = 0;

    virtual const_iterator cbegin() const = 0;

    virtual const_iterator cend() const = 0;

    virtual const_iterator begin() const = 0;

    virtual const_iterator end() const = 0;

    virtual ~IPackageStockpile() {};
};

class IPackageQueue : public IPackageStockpile {
    /*!
     * IPackageQueue
     * - klasa abstrakcyjna, która dziedziczy po IPackageStockpile (umożliwia obsługę magazynu paczek)
     * - zawiera metodę pop() - usuwa pierwszy element z kolejki i zwraca go
     * - służy jako interfejs dla klasy PackageQueue
     * - umożliwia dodawania i usuwanie elementów z kolejki (dla Worker)
     */
public:
    virtual Package pop() = 0;

    virtual PackageQueueType get_queue_type() const = 0;

    virtual ~IPackageQueue() {};
};

class PackageQueue : public IPackageQueue {
    /*!
     * PackageQueue
     * - klasa dziedzicząca po IPackageQueue
     * - zawiera prywatne pole typu PackageQueueType - określa typ kolejki (FIFO lub LIFO)
     * - zawiera prywatne pole typu std::list<Package> - przechowuje paczki
     * - zawiera konstruktor, który przyjmuje jako argument typ kolejki
     * - zawiera metody push() i pop() - dodają i usuwają elementy z kolejki
     * - służy do obsługi kolejek paczek u robotników
     */
public:
    PackageQueue(PackageQueueType type) : type_(type) {};

    void push(Package &&) override;

    bool empty() const override;

    size_t size() const override { return queue_.size(); }

    const_iterator cbegin() const override { return queue_.cbegin(); };

    const_iterator cend() const override { return queue_.cend(); }

    const_iterator begin() const override { return queue_.cbegin(); }

    const_iterator end() const override { return queue_.cend(); }

    Package pop() override;

    PackageQueueType get_queue_type() const override { return type_; };

    ~PackageQueue() override;

private:
    PackageQueueType type_;
    std::list<Package> queue_;
};

#endif //NETSIM_STORAGE_TYPES_HPP