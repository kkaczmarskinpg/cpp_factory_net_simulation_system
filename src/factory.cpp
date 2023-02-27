//
// Created by Hyperbook on 15.01.2023.
//

#include <istream>
#include <ostream>
#include <sstream>
#include "factory.hpp"

template<class Node>
void NodeCollection<Node>::add(Node &&node) {
    nodes_.push_back(std::move(node));
}

template void NodeCollection<Ramp>::add(Ramp &&ramp);

template void NodeCollection<Worker>::add(Worker &&worker);

template void NodeCollection<Storehouse>::add(Storehouse &&sender);

template<class Node>
typename NodeCollection<Node>::iterator NodeCollection<Node>::find_by_id(ElementID id) {
    return std::find_if(nodes_.begin(), nodes_.end(), [&id](const Node &node) { return node.get_id() == id; });
}

template NodeCollection<Ramp>::iterator NodeCollection<Ramp>::find_by_id(ElementID id);

template NodeCollection<Worker>::iterator NodeCollection<Worker>::find_by_id(ElementID id);

template NodeCollection<Storehouse>::iterator NodeCollection<Storehouse>::find_by_id(ElementID id);

template<class Node>
typename NodeCollection<Node>::const_iterator NodeCollection<Node>::find_by_id(ElementID id) const {
    return std::find_if(nodes_.begin(), nodes_.end(), [&id](const Node &node) { return node.get_id() == id; });
}

template NodeCollection<Ramp>::const_iterator NodeCollection<Ramp>::find_by_id(ElementID id) const;

template NodeCollection<Worker>::const_iterator NodeCollection<Worker>::find_by_id(ElementID id) const;

template NodeCollection<Storehouse>::const_iterator NodeCollection<Storehouse>::find_by_id(ElementID id) const;

template<class Node>
void NodeCollection<Node>::remove_by_id(ElementID id) {
    auto it = find_by_id(id);
    if (it != nodes_.end()) {
        nodes_.erase(it);
    }
}

template void NodeCollection<Ramp>::remove_by_id(ElementID id);

template void NodeCollection<Worker>::remove_by_id(ElementID id);

template void NodeCollection<Storehouse>::remove_by_id(ElementID id);

template<class Node>
void Factory::remove_receiver(NodeCollection<Node> &collection, ElementID id) {
    auto removed = collection.find_by_id(id);
    IPackageReceiver *receiver = dynamic_cast<IPackageReceiver *>(&(*removed));
    for (auto &worker: workers_) {
        worker.receiver_preferences_.remove_receiver(receiver);
    }
    for (auto &ramp: ramps_) {
        ramp.receiver_preferences_.remove_receiver(receiver);
    }
    collection.remove_by_id(id);
}

template void Factory::remove_receiver(NodeCollection<Worker> &collection, ElementID id);

template void Factory::remove_receiver(NodeCollection<Storehouse> &collection, ElementID id);


void Factory::do_deliveries(Time t) {
    for (auto &ramp: ramps_) {
        ramp.deliver_goods(t);
    }
}

void Factory::do_work(Time t) {
    for (auto &worker: workers_) {
        worker.do_work(t);
    }
}

void Factory::do_package_passing() {
    for (auto &ramp: ramps_) {
        ramp.send_package();
    }
    for (auto &worker: workers_) {
        worker.send_package();
    }
}

enum class NodeState {
    kNotVisited,
    kVisited,
    kFinished
};

bool is_storehouse_achievable(const PackageSender *node, std::map<const PackageSender *, NodeState> &node_states) {
    if (node_states[node] == NodeState::kFinished) {
        return true;
    }
    node_states[node] = NodeState::kVisited;

    if (node->receiver_preferences_.get_preferences().empty()) {
        throw std::logic_error("No receivers");
    }

    bool has_receiver = false;
    for (auto &receiver: node->receiver_preferences_.get_preferences()) {
        if (receiver.first->get_receiver_type() == ReceiverType::STOREHOUSE) {
            has_receiver = true;
        } else if (receiver.first->get_receiver_type() == ReceiverType::WORKER) {
            IPackageReceiver *receiver_ptr = receiver.first;
            auto worker_ptr = dynamic_cast<Worker *>(receiver_ptr);
            auto sendrecv_ptr = dynamic_cast<PackageSender *>(worker_ptr);
            if (sendrecv_ptr == node) {
                continue;
            }
            has_receiver = true;
            if (node_states[sendrecv_ptr] == NodeState::kNotVisited) {
                is_storehouse_achievable(sendrecv_ptr, node_states);
            }
        }
    }

    node_states[node] = NodeState::kFinished;

    if (!has_receiver) {
        throw std::logic_error("No receiver");
    }
    return true;
}

bool Factory::is_consistent() const {
    std::map<const PackageSender *, NodeState> node_states;

    for (const PackageSender &ramp: ramps_) {
        node_states[&ramp] = NodeState::kNotVisited;
    }

    for (const PackageSender &worker: workers_) {
        node_states[&worker] = NodeState::kNotVisited;
    }

    try {
        for (const PackageSender &ramp: ramps_) {
            is_storehouse_achievable(&ramp, node_states);
        }
    }
    catch (...) {
        return false;
    }
    return true;
}


ParsedLineData parse_line(const std::string &line) {
    std::istringstream iss(line);
    std::string tag;
    iss >> tag;
    if (ELEMENT_TYPE_NAMES.find(tag) == ELEMENT_TYPE_NAMES.end()) {
        throw std::logic_error("Unknown tag");
    }
    ParsedLineData data;
    ElementType type = ELEMENT_TYPE_NAMES.at(tag);

    data.type = type;

    std::string key_value;
    while (iss >> key_value) {
        std::istringstream iss2(key_value);
        std::string key;
        std::string value;
        std::getline(iss2, key, '=');
        std::getline(iss2, value, '=');
        if (key.empty() || value.empty()) {
            throw std::logic_error("Empty key or value");
        }
        if (std::find(REQUIRED_FIELDS.at(type).begin(), REQUIRED_FIELDS.at(type).end(), key) ==
            REQUIRED_FIELDS.at(type).end()) {
            throw std::logic_error("Unknown key");
        }
        data.data.insert({key, value});
    }

    return data;
}

Factory load_factory_structure(std::istream &is) {
    Factory factory;
    std::string line;
    while (std::getline(is, line)) {
        if (line.empty() || line[0] == ';') {
            continue;
        }
        ParsedLineData data = parse_line(line);
        switch (data.type) {
            case ElementType::RAMP:
                try {
                    ElementID id = std::stoi(data.data.at("id"));
                    TimeOffset di = std::stoi(data.data.at("delivery-interval"));
                    factory.add_ramp(Ramp(id, di));
                }
                catch (...) {
                    throw std::logic_error("Invalid values");
                }
                break;
            case ElementType::WORKER:
                try {
                    ElementID id = std::stoi(data.data.at("id"));
                    TimeOffset pd = std::stoi(data.data.at("processing-time"));
                    factory.add_worker(Worker(id, pd, std::make_unique<PackageQueue>(
                            QUEUE_TYPE_NAMES.at(data.data.at("queue-type")))));
                }
                catch (...) {
                    throw std::logic_error("Invalid values");
                }
                break;
            case ElementType::STOREHOUSE:
                try {
                    ElementID id = std::stoi(data.data.at("id"));
                    factory.add_storehouse(Storehouse(id));
                }
                catch (...) {
                    throw std::logic_error("Invalid values");
                }
                break;
            case ElementType::LINK:
                try {
                    std::istringstream src(data.data.at("src"));
                    std::string src_type;
                    std::string src_sid;
                    ElementID src_id;
                    std::getline(src, src_type, '-');
                    std::getline(src, src_sid, '-');
                    src_id = std::stoi(src_sid);
                    std::istringstream dst(data.data.at("dest"));
                    std::string dst_type;
                    std::string dst_sid;
                    ElementID dst_id;
                    std::getline(dst, dst_type, '-');
                    std::getline(dst, dst_sid, '-');
                    dst_id = std::stoi(dst_sid);

                    if (src_type == "ramp") {
                        auto ramp = factory.find_ramp_by_id(src_id);
                        if (ramp == factory.ramp_cend()) {
                            throw std::logic_error("Ramp not found");
                        }

                        if (dst_type == "worker") {
                            auto worker = factory.find_worker_by_id(dst_id);
                            if (worker == factory.worker_cend()) {
                                throw std::logic_error("Worker not found");
                            }
                            ramp->receiver_preferences_.add_receiver(&(*worker));
                        } else if (dst_type == "store") {
                            auto storehouse = factory.find_storehouse_by_id(dst_id);
                            if (storehouse == factory.storehouse_cend()) {
                                throw std::logic_error("Storehouse not found");
                            }
                            ramp->receiver_preferences_.add_receiver(&(*storehouse));
                        } else {
                            throw std::logic_error("Unknown destination type");
                        }
                    } else if (src_type == "worker") {
                        auto worker_src = factory.find_worker_by_id(src_id);
                        if (dst_type == "worker") {
                            auto worker = factory.find_worker_by_id(dst_id);
                            if (worker == factory.worker_cend()) {
                                throw std::logic_error("Worker not found");
                            }
                            worker_src->receiver_preferences_.add_receiver(&(*worker));
                        } else if (dst_type == "store") {
                            auto storehouse = factory.find_storehouse_by_id(dst_id);
                            if (storehouse == factory.storehouse_cend()) {
                                throw std::logic_error("Storehouse not found");
                            }
                            worker_src->receiver_preferences_.add_receiver(&(*storehouse));
                        } else {
                            throw std::logic_error("Unknown destination type");
                        }
                    } else {
                        throw std::logic_error("Unknown source type");
                    }
                }
                catch (...) {
                    throw std::logic_error("Invalid values");
                }
                break;
        }
    }
    return factory;
}

std::vector<std::string> get_receivers_string_list(ReceiverPreferences::const_iterator begin,
                                                   ReceiverPreferences::const_iterator end) {
    std::vector<std::string> receivers;
    std::for_each(begin, end, [&receivers](const auto &receiver) {
        receivers.push_back(RECEIVER_TYPE_NAMES.at(receiver.first->get_receiver_type()) + " #" +
                            std::to_string(receiver.first->get_id()));
    });
    std::sort(receivers.begin(), receivers.end());
    return receivers;
}

void save_factory_structure(const Factory &factory, std::ostream &os) {
    std::vector<std::string> links;

    os << "\n" << "; == LOADING RAMPS ==" << "\n\n";
    std::vector<std::string> ramp_lines;
    std::for_each(factory.ramp_cbegin(), factory.ramp_cend(), [&ramp_lines, &links](const auto &ramp) {
        std::ostringstream oss;
        oss << "LOADING_RAMP id=" << ramp.get_id() << " delivery-interval=" << ramp.get_delivery_interval() << "\n";
        ramp_lines.push_back(oss.str());
        std::for_each(ramp.receiver_preferences_.cbegin(), ramp.receiver_preferences_.cend(),
                      [&ramp, &links](const auto &receiver) {
                          std::ostringstream receiver_oss;
                          receiver_oss << "LINK src=ramp" << "-" << ramp.get_id() << " dest="
                                       << RECEIVER_TYPE_NAMES_IO.at(receiver.first->get_receiver_type()) << "-"
                                       << receiver.first->get_id() << "\n";
                          links.push_back(receiver_oss.str());
                      });

    });

    std::sort(ramp_lines.begin(), ramp_lines.end());
    for (const auto &line: ramp_lines) {
        os << line;
    }

    os << "\n" << "; == WORKERS ==" << "\n\n";
    std::vector<std::string> worker_lines;
    std::for_each(factory.worker_cbegin(), factory.worker_cend(), [&worker_lines, &links](const auto &worker) {
        std::string queue_name;
        for (const auto &it: QUEUE_TYPE_NAMES)
            if (it.second == worker.get_queue()->get_queue_type())
                queue_name = it.first;

        std::ostringstream oss;
        oss << "WORKER id=" << worker.get_id() << " processing-time=" << worker.get_processing_duration() << " queue-type=" << queue_name << "\n";
        worker_lines.push_back(oss.str());

        std::for_each(worker.receiver_preferences_.cbegin(), worker.receiver_preferences_.cend(),
                      [&worker, &links](const auto &receiver) {
                          std::ostringstream receiver_oss;
                          receiver_oss << "LINK src=worker" << "-" << worker.get_id() << " dest="
                                       << RECEIVER_TYPE_NAMES_IO.at(receiver.first->get_receiver_type()) << "-"
                                       << receiver.first->get_id() << "\n";
                          links.push_back(receiver_oss.str());
                      });
    });

    std::sort(worker_lines.begin(), worker_lines.end());
    for (const auto &line: worker_lines) {
        os << line;
    }

    os << "\n" << "; == STOREHOUSES ==" << "\n\n";
    std::vector<std::string> storehouse_lines;
    std::for_each(factory.storehouse_cbegin(), factory.storehouse_cend(), [&storehouse_lines](const auto &storehouse) {
        std::ostringstream oss;
        oss << "STOREHOUSE id=" << storehouse.get_id() << "\n";
        storehouse_lines.push_back(oss.str());
    });

    std::sort(storehouse_lines.begin(), storehouse_lines.end());
    for (const auto &line: storehouse_lines) {
        os << line;
    }

    os << "\n" << "; == LINKS ==" << "\n";
    std::sort(links.begin(), links.end());
    for (const auto &line: links) {
        os << line;
    }

    os.flush();
}

/** RAPORT **/
/*
 * void save_factory_structure(const Factory &factory, std::ostream &os) {
    os << "\n" << "== LOADING RAMPS ==" << "\n";
    std::vector<std::string> ramp_lines;
    std::for_each(factory.ramp_cbegin(), factory.ramp_cend(), [&ramp_lines](const auto &ramp) {
        std::ostringstream oss;
        oss << "LOADING RAMP #" << ramp.get_id() << "\n  Delivery interval: " << ramp.get_delivery_interval() << "\n";
        std::vector<std::string> receivers = get_receivers_string_list(ramp.receiver_preferences_.cbegin(),
                                                                       ramp.receiver_preferences_.cend());
        oss << "  Receivers:\n";
        for (const auto &receiver: receivers) {
            oss << "    " << receiver << "\n";
        }
        ramp_lines.push_back(oss.str());
    });

    std::sort(ramp_lines.begin(), ramp_lines.end());
    for (const auto &line: ramp_lines) {
        os << line;
    }

    os << "\n" << "== WORKERS ==" << "\n";
    std::vector<std::string> worker_lines;
    std::for_each(factory.worker_cbegin(), factory.worker_cend(), [&worker_lines](const auto &worker) {
        std::ostringstream oss;
        oss << "WORKER #" << worker.get_id() << "\n  Processing time: " << worker.get_processing_duration() << "\n";

        std::string queue_name;
        for (const auto &it: QUEUE_TYPE_NAMES)
            if (it.second == worker.get_queue()->get_queue_type())
                queue_name = it.first;
        oss << "  Queue type: " << queue_name << "\n";

        std::vector<std::string> receivers = get_receivers_string_list(worker.receiver_preferences_.cbegin(),
                                                                       worker.receiver_preferences_.cend());
        oss << "  Receivers:\n";
        for (const auto &receiver: receivers) {
            oss << "    " << receiver << "\n";
        }
        worker_lines.push_back(oss.str());
    });

    std::sort(worker_lines.begin(), worker_lines.end());
    for (const auto &line: worker_lines) {
        os << line;
    }

    os << "\n" << "== STOREHOUSES ==" << "\n";
    std::vector<std::string> storehouse_lines;
    std::for_each(factory.storehouse_cbegin(), factory.storehouse_cend(), [&storehouse_lines](const auto &storehouse) {
        std::ostringstream oss;
        oss << "STOREHOUSE #" << storehouse.get_id() << "\n";
        storehouse_lines.push_back(oss.str());
    });

    std::sort(storehouse_lines.begin(), storehouse_lines.end());
    for (const auto &line: storehouse_lines) {
        os << line;
    }

    os.flush();
}
 */
