// Lightweight smoke test for the outbound event queue.
// Validates the polling-based retrieval path and basic FIFO behavior.

#include <cassert>
#include <iostream>

#include "seedsigner_lvgl/runtime/EventQueue.hpp"

int main() {
    using namespace seedsigner::lvgl;

    // Test 1: basic push/next
    {
        EventQueue queue(2);
        assert(queue.empty());
        assert(queue.size() == 0);
        assert(queue.capacity() == 2);

        UiEvent e1{.type = EventType::ActionInvoked,
                   .route_id = RouteId{"test"},
                   .action_id = "confirm"};
        assert(queue.push(e1));
        assert(!queue.empty());
        assert(queue.size() == 1);

        UiEvent e2{.type = EventType::CancelRequested,
                   .route_id = RouteId{"test"}};
        assert(queue.push(e2));
        assert(queue.size() == 2);

        // Next retrieves in FIFO order
        auto out1 = queue.next();
        assert(out1.has_value());
        assert(out1->type == EventType::ActionInvoked);
        assert(out1->action_id == "confirm");
        assert(queue.size() == 1);

        auto out2 = queue.next();
        assert(out2.has_value());
        assert(out2->type == EventType::CancelRequested);
        assert(queue.empty());

        // Empty queue returns nullopt
        assert(!queue.next().has_value());
    }

    // Test 2: overflow drops new events
    {
        EventQueue queue(1);
        UiEvent e1{.type = EventType::RouteActivated,
                   .route_id = RouteId{"main"}};
        assert(queue.push(e1));
        assert(queue.size() == 1);

        UiEvent e2{.type = EventType::ScreenReady,
                   .route_id = RouteId{"main"}};
        assert(!queue.push(e2));  // queue full
        assert(queue.size() == 1);

        auto out = queue.next();
        assert(out.has_value());
        assert(out->type == EventType::RouteActivated);
        assert(queue.empty());
    }

    // Test 3: clear works
    {
        EventQueue queue(5);
        for (int i = 0; i < 3; ++i) {
            UiEvent e{.type = EventType::NeedsData,
                      .route_id = RouteId{"screen"},
                      .meta = EventMeta{"key", std::string{"value"}}};
            assert(queue.push(e));
        }
        assert(queue.size() == 3);
        queue.clear();
        assert(queue.empty());
        assert(queue.size() == 0);
    }

    // Test 4: zero capacity defaults to capacity 1
    {
        EventQueue queue(0);
        assert(queue.capacity() == 1);
        UiEvent e{.type = EventType::Error,
                  .route_id = RouteId{"any"}};
        assert(queue.push(e));
        assert(queue.size() == 1);
        assert(queue.next().has_value());
        assert(queue.empty());
    }

    std::cout << "All event‑queue smoke tests passed.\n";
    return 0;
}