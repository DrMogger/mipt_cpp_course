#include "event_list.h"

namespace nano_edr {

void ListPushBack(EventList* list, const Event* event) {
    if (list == nullptr || event == nullptr) {
        return;
    }

    if (list->capacity) {
        while(list->size >= list->capacity && list->head != nullptr) {
            ListPopFront(list);
        }
    }
    EventNode* node = new EventNode{*event, nullptr};

    if (list->tail == nullptr) {
        list->head = node;
    } else {
        list->tail->next = node;
    }
    list->tail = node;
    ++list->size;
}

void ListPopFront(EventList* list) {
    if (list == nullptr || list->head == nullptr) {
        return;
    }

    EventNode* old = list->head;
    list->head = old->next;
    if (list->head == nullptr) {
        list->tail = nullptr;
    }
    --list->size;
    delete old;
}

void ListClear(EventList* list) {
    if (list == nullptr) {
        return;
    }

    EventNode* node = list->head;
    while(node != nullptr) {
        EventNode* next = node->next;
        delete node;
        node = next;
    }

    list->head = nullptr;
    list->tail = nullptr;
    list->size = 0;
}

EventList::~EventList() {
    // EventNode* node = this->head;
    ListClear(this);
}

}  // namespace nano_edr