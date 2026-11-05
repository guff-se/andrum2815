# andrum2815

Firmware and hardware docs for the Andrum installation in Berlin: 44 frames (one each of ANDRUM1-4, four each of ANDRUM5-14), 2,592 WS2815 pixels. Same pixels, SP803E (ESP32) controller and cabling as `andetag-nafas`.

- `docs/ELECTRONICS_GUIDE.md`: frame inventory, power and data budget, wiring rules, test procedures, and Nafas validation results for reference.
- `hardware-test/hardware-test.ino`: standalone travelling-wave test sketch (guide Test 6). Set `NUM_LEDS` to the pixel count of the data path under test.

Arduino IDE: ESP32 Dev Module, Huge APP partition, 460800 upload baud. See guide section 2.
