#include "storage_types.hpp"

bool PackageQueue::empty() const {
    return queue_.empty();
}

Package PackageQueue::pop() {
    Package result(std::move(queue_.front()));
    queue_.pop_front();
    return result;

}

void PackageQueue::push(Package &&package) {
    switch (type_) {
        case PackageQueueType::LIFO:
            queue_.push_front(std::move(package));
            break;
        case PackageQueueType::FIFO:
            queue_.push_back(std::move(package));
            break;
    }
}

PackageQueue::~PackageQueue() {
    for (auto &package: queue_) {
        package.~Package();
    }
}
