#include "event_list.h"

namespace nano_edr {
    EventList::~EventList() {
        // EventNode* node = this->head;
        ListClear(this);
    }
}