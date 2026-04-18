"""Example: menu navigation with uiseedsigner.

Creates a runtime, activates a menu, navigates with keys, and reads events.
"""

import uiseedsigner as ui

def main():
    with ui.UiRuntime(240, 320) as rt:
        rt.init()

        # Activate a menu route (requires the route to be registered in C)
        args = "title=SeedSigner\nitems=scan|Scan QR|chevron\nseeds|Seeds|chevron"
        rt.activate("menu.main", args)

        # Navigation loop
        for _ in range(10):
            rt.tick(16)
            rt.refresh()

            # Read events
            while True:
                ev = rt.next_event()
                if ev is None:
                    break
                print(f"Event: type={ev.type} action={ev.action_id} value={ev.value}")

            # Simulate user pressing Down
            rt.send_input(ui.KEY_DOWN)

        print("Done.")

if __name__ == "__main__":
    main()
