//
// Created by Hyperbook on 15.01.2023.
//

#ifndef NETSIM_NODES_HPP
#define NETSIM_NODES_HPP

#include "config.hpp"
#include <memory>
#include <utility>
#include <vector>
#include <optional>
#include <map>
#include "package.hpp"
#include "storage_types.hpp"
#include "types.hpp"
#include "helpers.hpp"

enum class ReceiverType {
    WORKER,
    STOREHOUSE
};

class IPackageReceiver {
    /**
     * Interfejs dla obiektów, które mogą odbierać paczki (Worker, Storehouse)
     */
public:
    virtual void receive_package(Package &&p) = 0;

    virtual ElementID get_id() const { return id_; };

    virtual IPackageStockpile::const_iterator begin() const = 0;

    virtual IPackageStockpile::const_iterator end() const = 0;

    virtual IPackageStockpile::const_iterator cbegin() const = 0;

    virtual IPackageStockpile::const_iterator cend() const = 0;

#if (defined EXERCISE_ID && EXERCISE_ID != EXERCISE_ID_NODES)
    virtual ReceiverType get_receiver_type() const = 0;
#endif

    virtual ~IPackageReceiver() = default;

protected:
    ElementID id_;
};

class Storehouse : public IPackageReceiver {
public:
    Storehouse(ElementID id,
               std::unique_ptr<IPackageStockpile> ptr = std::make_unique<PackageQueue>(PackageQueueType::LIFO)) {
        id_ = id;
        stockpile_ = std::move(ptr);
    };

#if (defined EXERCISE_ID && EXERCISE_ID != EXERCISE_ID_NODES)
    ReceiverType get_receiver_type() const override { return ReceiverType::STOREHOUSE; }
#endif

    void receive_package(Package &&p) override {
        stockpile_->push(std::move(p));
    }

    IPackageStockpile::const_iterator begin() const override {
        return stockpile_->begin();
    }

    IPackageStockpile::const_iterator end() const override {
        return stockpile_->end();
    }

    IPackageStockpile::const_iterator cbegin() const override {
        return stockpile_->cbegin();
    }

    IPackageStockpile::const_iterator cend() const override {
        return stockpile_->cend();
    }

private:
    std::unique_ptr<IPackageStockpile> stockpile_;
};


class ReceiverPreferences {
    /**
     * klasa ReceiverPreferences zawiera preferencje nadawcy do przesyłania paczek
     */
public:
    /**
     * Typ prefernces_t
     * - std::map<IPackageReceiver *, double> - mapa z preferencjami, gdzie klucz to wskaźnik na odbiorcę,
     * a wartość to prawdopodobieństwo przesyłania paczki do tego odbiorcy
     */
    using preferences_t = std::map<IPackageReceiver *, double>;
    using const_iterator = preferences_t::const_iterator;

    ReceiverPreferences(ProbabilityGenerator generator = probability_generator) : generator_(std::move(generator)) {};

    void add_receiver(IPackageReceiver *receiver);

    void remove_receiver(IPackageReceiver *receiver);

    const_iterator begin() const { return preferences_.begin(); }

    const_iterator end() const { return preferences_.end(); }

    const_iterator cbegin() const { return preferences_.cbegin(); }

    const_iterator cend() const { return preferences_.cend(); }


    /**
     * metoda choose_receiver() zwraca wskaźnik na odbiorcę, który został wylosowany zgodnie z preferencjami.
     * Polega to na wylosowaniu wartości z przedziału [0,1] i sprawdzeniu, do którego odbiorcy należy.
     *
     * @return wskaźnik na odbiorcę
     */
    IPackageReceiver *choose_receiver();

    const preferences_t &get_preferences() const { return preferences_; };

    void set_preferences(preferences_t preferences) { preferences_ = std::move(preferences); };

private:
    ProbabilityGenerator generator_;
    preferences_t preferences_;
};

class PackageSender {
public:
    PackageSender(PackageSender &&) = default;

    PackageSender() = default;

    /**
     * @brief Metoda send_package() wysyła paczkę z bufora do odbiorcy
     */
    void send_package();

    /**
     * @brief Metoda get_sending_buffer() zwraca odnośnik na paczkę
     * @return referencja na paczkę
     */
    const std::optional<Package> &get_sending_buffer() const { return sending_buffer_; };

    ReceiverPreferences receiver_preferences_;
protected:
    /**
     * @brief Przekazywanie paczki do bufora. Usuwa paczkę z kolejki paczek i wrzuca ją do bufora
     * @param p - paczka do przekazania
     */
    void push_package(Package &&p) { sending_buffer_ = std::move(p); };
    std::optional<Package> sending_buffer_ = std::nullopt;
};

class Ramp : public PackageSender {
public:
    Ramp(ElementID id, TimeOffset di) : di_(di) { id_ = id; };

    /**
     * @brief Metoda deliver_goods() wywoływana jest przez symulację, w punkcie "Dostawa".
     * pozwala ona rampie zorientować się, kiedy powinna “wytworzyć” półprodukt
     * (na podstawie argumentu di typu TimeOffset przekazanego w konstruktorze klasy Ramp reprezentującego okres pomiędzy dostawami).
     * @param t - bieżący czas symulacji
     */
    void deliver_goods(Time t);

    TimeOffset get_delivery_interval() const { return di_; };

    ElementID get_id() const { return id_; };

private:
    /**
     * @brief Czas między dostawami
     */
    TimeOffset di_;
    ElementID id_;
};

class Worker : public IPackageReceiver, public PackageSender {
    /**
     * Klasa Worker reprezentuje pracownika, który może odbierać paczki i wysyłać je do kolejnego odbiorcy.
     * @field pd_ - czas przetwarzania paczki
     * @field package_processing_start_ - czas rozpoczęcia przetwarzania paczki
     * @field package_queue_ - kolejka paczek
     */
public:
    Worker(ElementID id, TimeOffset pd, std::unique_ptr<IPackageQueue> packageQueue) : pd_(pd) {
        id_ = id;
        package_queue_ = std::move(packageQueue);
    };

    IPackageStockpile::const_iterator begin() const override { return package_queue_->begin(); }

    IPackageStockpile::const_iterator end() const override { return package_queue_->end(); }

    IPackageStockpile::const_iterator cbegin() const override { return package_queue_->cbegin(); }

    IPackageStockpile::const_iterator cend() const override { return package_queue_->cend(); }

#if (defined EXERCISE_ID && EXERCISE_ID != EXERCISE_ID_NODES)
    ReceiverType get_receiver_type() const override { return ReceiverType::WORKER; };
#endif

    /**
     * @brief Metoda do_work() wywoływana jest przez symulację, w punkcie "Przetworzenie".
     * pozwala ona pracownikowi zorientować się, kiedy powinien zakończyć pracę nad aktualnie przetwarzaną paczką.
     * Na początku ustawia package_processing_start_time_ na bieżący czas symulacji, w celu odliczania czasu.
     * @param t - bieżący czas symulacji
     */
    void do_work(Time t);

    /**
     * @brief Metoda receive_package() pozwala pracownikowi odebrać paczkę.
     * @param p - paczka do odebrania
     */
    void receive_package(Package &&p) override { package_queue_->push(std::move(p)); }

    TimeOffset get_processing_duration() const { return pd_; };

    Time get_package_processing_start_time() const { return package_processing_start_time_; };

    IPackageQueue* get_queue() const { return package_queue_.get(); };

private:
    TimeOffset pd_;
    Time package_processing_start_time_ = 0;
    std::unique_ptr<IPackageQueue> package_queue_;

    std::optional<Package> current_package_ = std::nullopt;
};

#endif //NETSIM_NODES_HPP
