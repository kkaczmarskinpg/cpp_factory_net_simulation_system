//
// Created by Hyperbook on 15.01.2023.
//

#ifndef NETSIM_FACTORY_HPP
#define NETSIM_FACTORY_HPP

#include <stdexcept>
#include "types.hpp"
#include "nodes.hpp"

template<class Node>
class NodeCollection {
public:
    using container_t = typename std::list<Node>;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    void add(Node &&node);

    void remove_by_id(ElementID id);

    NodeCollection<Node>::iterator find_by_id(ElementID id);

    NodeCollection<Node>::const_iterator find_by_id(ElementID id) const;

    NodeCollection<Node>::iterator begin() { return nodes_.begin(); }

    NodeCollection<Node>::iterator end() { return nodes_.end(); }

    NodeCollection<Node>::const_iterator begin() const { return nodes_.begin(); }

    NodeCollection<Node>::const_iterator end() const { return nodes_.end(); }

    NodeCollection<Node>::const_iterator cbegin() const { return nodes_.cbegin(); }

    NodeCollection<Node>::const_iterator cend() const { return nodes_.cend(); }

private:
    container_t nodes_;
};

class Factory {
public:
    void add_ramp(Ramp &&ramp) { ramps_.add(std::move(ramp)); }

    void remove_ramp(ElementID id) { ramps_.remove_by_id(id); };

    NodeCollection<Ramp>::iterator find_ramp_by_id(ElementID id) { return ramps_.find_by_id(id); }

    NodeCollection<Ramp>::const_iterator find_ramp_by_id(ElementID id) const { return ramps_.find_by_id(id); };

    NodeCollection<Ramp>::const_iterator ramp_cbegin() const { return ramps_.cbegin(); }

    NodeCollection<Ramp>::const_iterator ramp_cend() const { return ramps_.cend(); }

    void add_worker(Worker &&worker) { workers_.add(std::move(worker)); }

    void remove_worker(ElementID id) { remove_receiver(workers_, id); };

    NodeCollection<Worker>::iterator find_worker_by_id(ElementID id) { return workers_.find_by_id(id); }

    NodeCollection<Worker>::const_iterator find_worker_by_id(ElementID id) const { return workers_.find_by_id(id); };

    NodeCollection<Worker>::const_iterator worker_cbegin() const { return workers_.cbegin(); };

    NodeCollection<Worker>::const_iterator worker_cend() const { return workers_.cend(); };

    void add_storehouse(Storehouse &&storehouse) { storehouses_.add(std::move(storehouse)); }

    void remove_storehouse(ElementID id) { remove_receiver(storehouses_, id); };

    NodeCollection<Storehouse>::iterator find_storehouse_by_id(ElementID id) { return storehouses_.find_by_id(id); };

    NodeCollection<Storehouse>::const_iterator find_storehouse_by_id(ElementID id) const { return storehouses_.find_by_id(id); };

    NodeCollection<Storehouse>::const_iterator storehouse_cbegin() const { return storehouses_.cbegin(); }

    NodeCollection<Storehouse>::const_iterator storehouse_cend() const { return storehouses_.cend(); }

    bool is_consistent() const;

    void do_deliveries(Time t);

    void do_package_passing();

    void do_work(Time t);

private:

    template<class Node>
    void remove_receiver(NodeCollection<Node> &collection, ElementID id);

    NodeCollection<Ramp> ramps_;
    NodeCollection<Worker> workers_;
    NodeCollection<Storehouse> storehouses_;

};

enum class ElementType {
    RAMP,
    WORKER,
    STOREHOUSE,
    LINK
};

struct ParsedLineData {
    ElementType type;
    std::map<std::string, std::string> data;
};

const std::map<std::string, ElementType> ELEMENT_TYPE_NAMES = {
        {"LOADING_RAMP", ElementType::RAMP},
        {"WORKER", ElementType::WORKER},
        {"STOREHOUSE", ElementType::STOREHOUSE},
        {"LINK", ElementType::LINK}
};

const std::map<ElementType, std::vector<std::string>> REQUIRED_FIELDS = {
        {ElementType::RAMP, {"id", "delivery-interval"}},
        {ElementType::WORKER, {"id", "processing-time", "queue-type"}},
        {ElementType::STOREHOUSE, {"id"}},
        {ElementType::LINK, {"src", "dest"}}
};

const std::map<std::string, PackageQueueType> QUEUE_TYPE_NAMES = {
        {"FIFO", PackageQueueType::FIFO},
        {"LIFO", PackageQueueType::LIFO}
};

const std::map<ReceiverType, std::string> RECEIVER_TYPE_NAMES = {
        {ReceiverType::WORKER, "worker"},
        {ReceiverType::STOREHOUSE, "storehouse"}
};

const std::map<ReceiverType, std::string> RECEIVER_TYPE_NAMES_IO = {
        {ReceiverType::WORKER, "worker"},
        {ReceiverType::STOREHOUSE, "store"}
};
/**
 * @brief Parsuje linię (odczytuje dane i zwraca je w postaci struktury składającej się z typu i mapy danych)
 * @note Linia przyjmuje poniższy format: \n
 * TAG {key=pair}xN \n
 * gdzie TAG to jeden z czterech możliwych typów elementów (LOADING_RAMP, WORKER, STOREHOUSE, LINK), przykładowo: \n
 * LOADING_RAMP id=1 delivery-interval=3
 * @param line Linia do przetworzenia
 * @return ParsedLineData struct z danymi
 */
ParsedLineData parse_line(const std::string &line);

Factory load_factory_structure(std::istream &is);

void save_factory_structure(const Factory &factory, std::ostream &os);

#endif //NETSIM_FACTORY_HPP
