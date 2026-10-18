#include "event_list.h"

namespace nano_edr {

EventList::~EventList() {
    Clear();
}

void EventList::PushBack(const Event& event) {
    if (head_ == nullptr) {
        return;
    }

    if (capacity_) {
        while(size_ >= capacity_ && head_ != nullptr) {
            PopFront();
        }
    }
    EventNode* node = new EventNode{event};

    if (tail_ == nullptr) {
        head_ = node;
    } else {
        tail_->next = node;
    }
    tail_ = node;
    ++size_;
}

void EventList::PopFront() {
    if (head_ == nullptr) {
        return;
    }

    EventNode* old = head_;
    head_ = old->next;
    if (head_ == nullptr) {
        tail_ = nullptr;
    }
    --size_;
    delete old;
}

void EventList::Clear() {
    if (head_ == nullptr) {
        return;
    }

    EventNode* node = head_;
    while(node != nullptr) {
        EventNode* next = node->next;
        delete node;
        node = next;
    }

    head_ = nullptr;
    tail_ = nullptr;
    size_ = 0;
}

}  // namespace nano_edr