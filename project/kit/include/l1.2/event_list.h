#ifndef NANO_EDR_KIT_EVENT_LIST_H
#define NANO_EDR_KIT_EVENT_LIST_H

#include <cstddef>

#include "event.h"

namespace nano_edr {


struct EventNode {
    Event event;
    EventNode* next = nullptr;
};


struct EventList {
    EventNode* head = nullptr;
    EventNode* tail = nullptr;

    std::size_t size = 0;

    // Сколько событий держать. Ноль означает «без ограничения»: так список
    // годится и как окно, и как обычное хранилище — тестам нужно и то, и то.
    std::size_t capacity = 0;

    ~EventList();
};


void ListPushBack(EventList* list, const Event* event);

void ListPopFront(EventList* list);

void ListClear(EventList* list);

}  // namespace nano_edr

#endif  // NANO_EDR_KIT_EVENT_LIST_H
