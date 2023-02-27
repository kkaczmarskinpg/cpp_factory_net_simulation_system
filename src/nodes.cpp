//
// Created by Hyperbook on 15.01.2023.
//

#include <stdexcept>
#include "nodes.hpp"

IPackageReceiver *ReceiverPreferences::choose_receiver() {
    double random = generator_();
    double sum = 0;
    for (auto &receiver : preferences_) {
        sum += receiver.second;
        if (random <= sum) {
            return receiver.first;
        }
    }
    throw std::logic_error("No receiver chosen");
}

void ReceiverPreferences::add_receiver(IPackageReceiver *receiver) {
    double prob = 1.0 / (preferences_.size() + 1.0);
    for (auto &pref : preferences_) {
        pref.second = prob;
    }
    preferences_.insert({receiver, prob});
}

void ReceiverPreferences::remove_receiver(IPackageReceiver *receiver) {
    if (preferences_.find(receiver) != preferences_.end()) {
        preferences_.erase(receiver);
        double prob = 1.0 / preferences_.size();
        for (auto &pref: preferences_) {
            pref.second = prob;
        }
    }
}

void PackageSender::send_package() {
    if (sending_buffer_.has_value()) {
        auto receiver = receiver_preferences_.choose_receiver();
        if (receiver != nullptr) {
            receiver->receive_package(std::move(sending_buffer_.value()));
            sending_buffer_.reset();
        }
    }
}

void Worker::do_work(Time t) {
    if(!current_package_.has_value() && !package_queue_->empty()){
        current_package_ = package_queue_->pop();
        package_processing_start_time_ = t;
    }
    if(package_processing_start_time_+pd_ == t+1){
        push_package(std::move(current_package_.value()));
        current_package_.reset();
        package_processing_start_time_ = 0;
    }
}

void Ramp::deliver_goods(Time t) {
    if ((t-1) % di_ == 0) {
        push_package(Package());
    }
}
