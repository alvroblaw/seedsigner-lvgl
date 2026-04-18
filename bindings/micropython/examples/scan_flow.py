"""Example: camera frame injection with uiseedsigner.

Pushes grayscale frames from Python into the scan screen.
"""

import uiseedsigner as ui

def main():
    with ui.UiRuntime(240, 320) as rt:
        rt.init()

        # Activate scan screen
        rt.activate("suite.scan", "instruction_text=Scan QR")

        # Simulate camera frames (10x10 grayscale)
        for seq in range(20):
            rt.tick(16)
            rt.refresh()

            # Create a dummy 10x10 grayscale frame
            pixels = bytes([128] * 100)
            rt.push_frame(pixels, 10, 10)

            # Check for scan completion
            while True:
                ev = rt.next_event()
                if ev is None:
                    break
                if ev.type == ui.EVENT_ACTION_INVOKED and ev.action_id == "scan_complete":
                    print(f"QR decoded: {ev.value}")
                    return

        print("No QR found.")

if __name__ == "__main__":
    main()
