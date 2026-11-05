# andrum2815

Uses the same WS2815 pixels, SP803E (ESP32) controller and cabling as `andetag-nafas`.

- `docs/ELECTRONICS_GUIDE.md`: hardware reference and test results, copied from `andetag-nafas`. Frame order, pixel counts and measurements describe the Nafas build.
- `hardware-test/hardware-test.ino`: standalone travelling-wave test sketch (guide Test 6). Set `NUM_LEDS` to the chain length under test.

Arduino IDE: ESP32 Dev Module, Huge APP partition, 460800 upload baud. See guide section 2.
