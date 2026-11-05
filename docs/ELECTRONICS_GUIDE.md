# ANDRUM BERLIN: Electronics Reference

## Andrum installation, Berlin

Hardware (WS2815 pixels, SP803E controller, 22AWG 3-conductor cabling) is the same as the Andetag Nafas installation. Sections 2, 3, 4, 8 and the Nafas reference results in section 6 are carried over from that project. Anything Berlin-specific that is not yet decided is marked **TBD**.

---

## 1. System Overview

44 frames, 14 models, **2,592 WS2815 pixels** total. 12V DC.

### Frame Inventory

Pixel data source: "Andrum" tab of the frame spreadsheet (`NUM_LEDS`, `NUM_MLEDS`, `NUM_STRIPS` rows), read 2026-11-05.

| Model | Qty | NUM_LEDS (per frame) | NUM_MLEDS | NUM_STRIPS | Pixels (qty x NUM_LEDS) |
|-------|-----|----------------------|-----------|------------|-------------------------|
| ANDRUM1  | 1 | 89  | 45  | 8  | 89  |
| ANDRUM2  | 1 | 36  | 18  | 4  | 36  |
| ANDRUM3  | 1 | 81  | 41  | 7  | 81  |
| ANDRUM4  | 1 | 138 | 69  | 12 | 138 |
| ANDRUM5  | 4 | 27  | 27  | 4  | 108 |
| ANDRUM6  | 4 | 75  | 75  | 12 | 300 |
| ANDRUM7  | 4 | 33  | 33  | 5  | 132 |
| ANDRUM8  | 4 | 109 | 109 | 17 | 436 |
| ANDRUM9  | 4 | 54  | 54  | 8  | 216 |
| ANDRUM10 | 4 | 45  | 45  | 7  | 180 |
| ANDRUM11 | 4 | 99  | 99  | 15 | 396 |
| ANDRUM12 | 4 | 36  | 36  | 6  | 144 |
| ANDRUM13 | 4 | 60  | 60  | 9  | 240 |
| ANDRUM14 | 4 | 24  | 24  | 3  | 96  |
| **Total** | **44** | | | | **2,592** |

Subtotals: ANDRUM1-4 (one each) = 344 px. One set of ANDRUM5-14 = 562 px; four sets = 2,248 px.

The spreadsheet is authoritative for pixel counts. Some Nafas frames were built slightly differently (Nafas firmware: ANDRUM8 = 105, ANDRUM9 = 48, ANDRUM11 = 90); do not reuse Nafas mapping values for these models.

### Power Loops (proposed)

The 44 frames are split into **5 independent power loops**, each wired like Nafas: one 22AWG 3-conductor run, 12V fed at both ends, frames tapping the bus in parallel, data daisy-chained through the loop. Each loop gets its own data output, so data and power topology are the same.

| Loop | Frames | Order (feed A → feed B) | Pixels | Est. worst drop @50% white | Est. worst frame @100% white |
|------|--------|--------------------------|--------|----------------------------|------------------------------|
| A | ANDRUM5-14, set 1 | 8, 13, 9, 12, 14, 5, 7, 10, 6, 11 | 562 | 1.52V (frames 14/5) | ~9.4V |
| B | ANDRUM5-14, set 2 | same as A | 562 | 1.52V | ~9.4V |
| C | ANDRUM5-14, set 3 | same as A | 562 | 1.52V | ~9.4V |
| D | ANDRUM5-14, set 4 | same as A | 562 | 1.52V | ~9.4V |
| E | ANDRUM1-4 | 4, 2, 3, 1 | 344 | 0.61V | ~11.0V |
| | **44 frames** | | **2,592** | Nafas measured: 2.04V | Nafas measured: 8.53V |

Why this split:
- **Stays inside the Nafas-validated envelope.** Nafas (660 px, 8 frames, 20m loop) passed all tests with a worst-case drop of 2.04V at 50% white. Every proposed loop has fewer pixels and a smaller estimated drop, so Tests 2, 3 and 7 should carry over with margin instead of being re-derived.
- **Order matters.** Heaviest frames (ANDRUM8, ANDRUM11) sit next to the feeds and the lightest (ANDRUM14, ANDRUM5) in the middle, where Nafas showed the voltage floor is. Placing them in numeric order 5..14 raises the worst drop from 1.52V to 2.06V (no margin over Nafas); the worst possible order gives 2.26V.
- **Four identical loops.** Same frames, same order, same pixel offsets: one mapping table and one test procedure for A-D.
- **Data rate.** Largest data chain is 562 px, ~24.5 ms per `show()` at the Nafas-measured rate, ~40 FPS (Nafas ran ~33 FPS).

Assumptions (all estimates depend on them):
- Same cable as Nafas, 3m lead from each feed to the end frames, **2m between frames**. Inter-frame spacing up to ~3m keeps Loops A-D within the Nafas 2.04V drop (3m: 1.88V; 4m: 2.24V). Longer runs need heavier cable or shorter loops.
- Feeds A and B of a loop are at equal voltage (same PSU, or matched PSUs with common ground).
- Estimates come from a resistive model of the loop (frames as current sinks proportional to pixel count, cable resistance proportional to length) scaled to the Nafas Test 2 measurements. The model reproduces Nafas Test 2 within 0.26V per frame (r = 0.97). Verify with Berlin Test 2 on the built loops.

Fallback with 4 loops: one ANDRUM5-14 set plus one of ANDRUM1-4 per loop (11 frames, 598-700 px). Estimated worst drop 1.73-1.97V at 2m spacing: inside the Nafas envelope but with almost no margin, and it fails at 3m spacing. Use only if a fifth PSU or data output is not available.

Physical placement of loops in the room and exact cable lengths: **TBD**.

---

## 2. Controller

**BTF-LIGHTING SP803E** (WLED-ESP32)

- MCU: ESP32-WROOM-32E (ESP32-D0WD-V3 rev 3.1), 240MHz dual core
- USB-to-serial: CH340C
- Data pin: **GPIO16**
- Barrel jack: 5-24V DC input (powers ESP32 only; LED power goes directly from PSU to strip)
- Operating temp: -10 to 40C

### Arduino IDE Settings

- Board: **ESP32 Dev Module**
- Partition Scheme: **Huge APP (3MB No OTA/1MB SPIFFS)**
- Upload Speed: **460800** (921600 causes upload failure on CH340C)
- Flash Size: 4MB
- CPU Frequency: 240MHz

### USB Connection (macOS Tahoe 26.0)

**A straight USB-C cable does NOT work** with this board (CC pin negotiation issue on the SP803E's USB-C port). Working path verified: **USB-C (Mac) → USB-A adapter/hub → USB-C (SP803E)**.

Driver: WCH CH340 DriverKit extension (cn.wch.CH34xVCPDriver). Install from WCH .pkg, then open CH34xVCPDriver app from Launchpad and click Install. Verify with `systemextensionsctl list`. Device appears as `/dev/cu.usbserial-*`.

### FastLED Setup

```cpp
#define DATA_PIN 16
#define NUM_LEDS 2592  // total; split across data outputs once topology is decided (section 5)

CRGB leds[NUM_LEDS];

void setup() {
    FastLED.addLeds<WS2815, DATA_PIN, GRB>(leds, NUM_LEDS);
}
```

---

## 3. Wiring

### Power

12V PSU feeds two paths:
1. Barrel jack on SP803E (milliamps, powers ESP32 only)
2. Directly to each power loop VCC/GND at both ends (5 loops, section 1)

### Data Between Frames

Connect **DO** (data out) from last pixel of frame N to **DI + BI** (data in + backup in) on first pixel of frame N+1.

**CRITICAL: Never pair DO+BO together. Connecting two outputs causes signal corruption and frame drops. Only DO connects to the next frame.**

**BO on inter-frame cables must be fully cut and isolated** — an uncut BO conductor can contact other pins at certain cable angles, intermittently shorting two outputs (same effect as DO+BO paired). Verified in Test 6: caused F6–F8 flicker while F1–F5 looked clean.

Tying DI+BI together at each frame input means backup data protection is lost only on the first pixel of each frame. From pixel 2 onward within each frame, backup wiring on the strip PCB functions normally.

### Signal Voltage

ESP32 GPIO outputs 3.3V. WS2815 spec is 3.7-5.3V. 3.3V works in practice but is below spec. If data integrity issues appear on the first pixel, add a level shifter (SN74HCT125) between GPIO16 and the first pixel's DI.

---

## 4. WS2815 Behavior

- 12V input, internally regulated to 5V for digital logic
- Constant current LED driver: brightness is PWM-controlled, not voltage-dependent
- Above 9.5V supply: full brightness, identical regardless of voltage
- Below 9.5V: constant current driver loses regulation; LEDs dim and color-shift (gradual, not a cliff)
- Below ~6-7V: internal 5V logic fails, data regeneration stops (this would cascade downstream but is unreachable in this system's voltage range)
- Data signal regenerated as clean 5V at each pixel (spec: up to 5m between points)
- Current draw: ~15mA per pixel at full white, roughly constant regardless of color (excess dissipated as heat)
- Color order: GRB
- Operating temperature: -25C to +85C

---

## 5. Power and Data Budget (estimates)

### Power

Per-pixel figures from Nafas (section 4, Test 7). Animation load fractions are Nafas-measured and only apply if Berlin runs a similar animation.

| Condition | Per pixel | Total (2,592 px) | At 12V |
|-----------|-----------|------------------|--------|
| Full white | 15mA | 38.9A | 467W |
| Nafas breathing animation, peak single frame (64%) | ~9.6mA | ~24.9A | ~299W |
| Nafas breathing animation, peak 1-min avg (35%) | ~5.3mA | ~13.6A | ~163W |

Implications:
- Nafas (660 px, 22AWG, fed from both ends) dropped to 10.16V at its midpoint frame at 50% white and 8.53V at 100% white. Berlin draws roughly 4x the current, so it is split into 5 loops (section 1).
- Per loop: Loops A-D (562 px) draw 8.4A / 101W at full white, ~5.4A at the Nafas animation peak. Loop E (344 px) draws 5.2A / 62W at full white.
- Full white still puts the middle frames of Loops A-D at ~9.4V, just under the 9.5V regulation threshold (better than Nafas at 8.53V). A firmware brightness cap removes this if full white is not needed.

### Data Rate

WS2815 at 800kHz: 30 µs per pixel on the wire. Nafas measured ~43.6 µs per pixel including ESP32 RMT overhead (Test 5: ~30.4 ms per `show()` with 698 pixels configured).

| Data outputs (equal split) | Pixels per output | `show()` time (Nafas-measured rate) | Max FPS |
|----------------------------|-------------------|--------------------------------------|---------|
| 1 | 2,592 | ~113 ms | ~9 |
| 4 | 648 | ~28 ms | ~35 |
| 5 (proposed loops, max 562) | 562 | ~24.5 ms | ~40 |
| 8 | 324 | ~14 ms | ~70 |

A single data line gives ~9 FPS, too slow for smooth animation. The table assumes outputs transmit in parallel; whether FastLED on this controller does so, and how many outputs the SP803E exposes, is **unverified**. If the SP803E has one usable data pin, Berlin needs a different controller or multiple controllers.

---

## 6. Testing Procedures

### Berlin Tests

Status: **not started.** Re-run Tests 1-7 below on each loop. Test 2 checks the model: expected lowest frames in Loops A-D are ANDRUM14 and ANDRUM5 (positions 5-6) at ~1.5V below feed voltage under 50% white. A reading near or above 2V means spacing or cabling differs from the assumptions in section 1.

Test firmware: `hardware-test/hardware-test.ino`. Set `NUM_LEDS` to the pixel count of the data path under test.

### Reference: Nafas Validation Results

Nafas configuration for context: 8 frames in order ANDRUM9, 8, 11, 4, 2, 11, 8, 9, single data line from GPIO16, one 20m 22AWG loop fed from both ends, PSU-3m-F1-2m-...-F8-3m-PSU. Production firmware `NUM_LEDS` = 660 (9: 48, 8: 105, 11: 90, 4: 138, 2: 36). The original Nafas guide listed ANDRUM4 as 144 px and a 666 px total; production firmware and the frame spreadsheet both say 138, so pixel counts quoted below as 666 were actually 660.

Status: **All tests complete (1–7 PASS).** System validated for installation.

#### Test 1: Baseline PSU Voltage — PASS

Measure DC voltage at PSU output terminals with no load, then with all frames connected (pixels dark). Expected: 12.0-12.5V.

| Measurement | Result | Expected | Status |
|-------------|--------|----------|--------|
| PSU output (no load) | 12.5V | 12.0-12.5V | PASS |
| PSU output (frames connected, no extension cables, pixels dark) | 12.12V | 12.0-12.5V | PASS |

#### Test 2: Voltage Distribution Under Load — PASS

Set all pixels to solid color at 50% brightness. Measure 12V (VCC to GND) at each frame's input connector, frames 1-8. Record all 8 readings plus PSU output. All frames should be above 9.5V. Center frames will be lowest.

Load: 50% white (`CRGB(128,128,128)`), full 666-pixel chain with extension cables.

| Point | Voltage | Drop from PSU | Status |
|-------|---------|---------------|--------|
| PSU | 12.20V | — | — |
| F1 (ANDRUM9) | 11.33V | 0.87V | PASS |
| F2 (ANDRUM8) | 10.74V | 1.46V | PASS |
| F3 (ANDRUM11) | 10.36V | 1.84V | PASS |
| F4 (ANDRUM4) | 10.16V | 2.04V | PASS (lowest) |
| F5 (ANDRUM2) | 10.26V | 1.94V | PASS |
| F6 (ANDRUM11) | 10.45V | 1.75V | PASS |
| F7 (ANDRUM8) | 10.47V | 1.73V | PASS |
| F8 (ANDRUM9) | 11.33V | 0.87V | PASS |

Notes: Symmetric F1/F8 readings (11.33V) confirm dual-end power feed is working. F4 is the voltage floor at 10.16V — 0.66V above the 9.5V regulation threshold.

#### Test 3: Voltage Under Stress — COMPLETE

Set all pixels to full white 100%. Measure voltage at ANDRUM4 (frame 4). Establishes the actual voltage floor. Expected to dip below 9.5V; acceptable since this state is momentary.

Load: full white (`CRGB::White`), full 666-pixel chain with extension cables.

| Point | Voltage | Notes |
|-------|---------|-------|
| PSU | 12.00V | Down 0.20V from Test 2 load reading |
| F4 (ANDRUM4) | 8.53V | Below 9.5V regulation threshold; above logic-failure range (~6-7V) |

Visual: white appeared uniform across all frames despite F4 voltage drop. Confirms full-white state is momentary-only in production animation; not a sustained operating condition.

#### Test 4: Data Integrity (Visual) — PASS

Run a single-pixel chase at maximum speed through all 666 pixels. Observe each frame boundary for stuck pixels, color errors, flickering, or skipped pixels.

Result: clean chase through all 666 pixels. No stuck, skipped, or miscolored pixels at frame boundaries. Motion smooth and continuous.

#### Test 5: Data Integrity (Quantitative) — PASS

Measure `FastLED.show()` duration. Expected: ~20ms for 666 WS2815 pixels at 800kHz (~50 FPS). If significantly longer, investigate firmware conflicts (WiFi, interrupts).

Initial attempt (discarded): single `micros()` around `show()` with static all-black buffer reported 927 µs — wrong; ESP32 RMT timing must use a moving-pixel lap test (no dirty-flag skip; `show()` always transmits full buffer).

Redo method: moving-pixel chase over 666 frames. Reports single-frame `show()` µs and full 666-frame lap time.

| Measurement | Result | Expected | Status |
|-------------|--------|----------|--------|
| Single `show()` µs | ~30,385 µs, stable | ~20,000 µs wire-only | PASS (see note) |
| 666-frame lap | 21,240 ms, stable | ~13,000–14,000 ms wire-only | PASS (see note) |
| Per-frame average | ~30,430 µs | ~20,000 µs | PASS (see note) |

Note: ~30 ms/frame is ~50% above theoretical on-wire time (~20 ms), likely ESP32 RMT driver overhead on this controller. Timing is stable with no jitter between laps. Effective max refresh ~33 FPS — adequate for breathing animation. Investigate further only if flickering correlates with frame rate (Test 6). Measured timings above were taken under the prior 698-pixel firmware count; re-check if show() budget becomes tight.

#### Test 6: Flickering Isolation — PASS

Progressive isolation to determine if flickering is electrical, firmware, or controller-related.

Use a **travelling sine wave** across pixels (4.5 s to cross the strip, FastLED `cos8()` + production `gamma8[]` from `helpers.h`). Frame cap: `TARGET_FPS 60` / `FRAME_TIME` (16 ms), matching production. Effective rate is `max(TARGET_FPS, show()-limited FPS)`. Reveals frame drops (brightness jumps), stutter, and flicker better than a pixel chase. Test 4's max-speed chase is for data integrity only.

| Step | Frames | NUM_LEDS | Data | Power | Result |
|------|--------|----------|------|-------|--------|
| 1 | F1 only | 48 | F2–F8 disconnected | F2–F8 connected | PASS — smooth wave |
| 2 | F1+F2 | 153 | F3–F8 disconnected | F2–F8 connected | Slight flicker at NUM_LEDS=153 |
| 2b | F1+F2 (same wiring) | 666 in firmware | F3–F8 disconnected | F2–F8 connected | Flicker gone — phantom pixels slow show() |
| 8 | F1–F8 canonical order | 666 | all connected | all connected | **FLICKER on F6, F7, F8** |
| 9 | F5 bypassed: F4→F6 direct | 630* | F5 out of data path | all on power | **No flicker** |
| 10 | F5 moved to pos 2: F1→F5→F2… | 666 | reordered | all connected | **No flicker** |
| 11 | F1–F8 canonical, BO fixed | 666 | all connected | all connected | PASS — no issues |

*630 = 666 − 36 (F5 pixel count); firmware `NUM_LEDS` must match data path.

Cumulative `NUM_LEDS` (canonical order): F1=48, F2=153, F3=243, F4=387, F5=423, F6=513, F7=618, F8=666.

Findings:
- Partial-chain flicker at high refresh → **frame-rate timing** (fixed with `FRAME_TIME` cap in test firmware).
- Full-chain flicker on F6–F8 only → **not** F6/F7/F8 hardware; **not** F5 (ANDRUM2) electronics.
- F4→F6 bypass and F5-at-position-2 both clean → fault in **F4–F5–F6 segment cabling**, not frame electronics.

**Root cause:** BO conductor on inter-frame cable not fully cut — could contact adjacent pin at certain cable angles, intermittently creating DO+BO output conflict. Fixed by properly cutting/isolating all BO conductors on inter-frame cables.

1. ~~F1 only (48px).~~ PASS
2. ~~Partial chain timing tests.~~ Frame-rate artefact; not wiring.
3. ~~Full chain canonical order.~~ Flicker F6–F8.
4. ~~F4→F6 bypass; F5 at position 2.~~ Isolated to F4–F5–F6 cabling.
5. ~~BO cable cut/isolation fix.~~ Resolved.

#### Test 7: Long-Duration Stability — PASS

Run production breathing animation for 2+ hours. Measure 12V at ANDRUM4 (frame input connector; middle of frame is acceptable probe point) every 30 min. Monitor for visual degradation or color shift.

Load: production `andetag-nafas` firmware, breathing animation. Measured output: max single frame **64%**, highest 1-min average **35%** (first 15 min).

| Time | F4 voltage | Δ from initial | Visual | Status |
|------|------------|----------------|--------|--------|
| 5 min | ~11.0V | — | — | — |
| 30 min | 11.2–11.4V | +0.2 to +0.4V | — | Stable |
| 20 h | 11.2–11.4V | +0.2 to +0.4V | No change | **PASS** |

Result: 20-hour continuous run. F4 voltage unchanged from 30-min reading. No visual degradation or color shift. Well above 9.5V regulation threshold under real animation load.

---

## 7. Wiring Checklist

- [ ] PSU output(s) confirmed ~12V
- [ ] Barrel jack connected to controller
- [ ] Each of the 5 loops fed 12V at both ends
- [ ] Each loop's data output connected to its first frame (feed A end) at DI+BI
- [ ] Frame order in Loops A-D: 8, 13, 9, 12, 14, 5, 7, 10, 6, 11; Loop E: 4, 2, 3, 1
- [ ] Each inter-frame: DO only to DI+BI (not DO+BO)
- [ ] BO cut and isolated on all inter-frame cables (cannot contact other conductors)
- [ ] GND common between all PSUs, controller, and all frames
- [ ] USB-C→USB-A→USB-C path available for uploads (straight USB-C will not work)
- [ ] Firmware `NUM_LEDS` per output matches the pixels physically on that data path

---

## 8. Known Issues

| Issue | Cause | Fix |
|-------|-------|-----|
| SP803E not detected via straight USB-C | CC pin negotiation failure on board | Use USB-C→USB-A→USB-C (adapter/hub) |
| Upload fails at 921600 baud | CH340C limitation | Set upload speed to 460800 |
| Sketch too large (default partition) | 1.25MB app limit | Use Huge APP (3MB No OTA) partition |
| Signal corruption at frame boundaries | DO+BO paired together | Only connect DO to DI+BI |
| F6–F8 flicker, F1–F5 clean (canonical order) | Uncut BO conductor touching adjacent pin at cable angle | Fully cut and isolate BO on all inter-frame cables |
| Color degradation under heavy load (Nafas: ANDRUM4 at loop midpoint) | Voltage drop along 22AWG bus | Firmware brightness cap, more injection points, heavier feeders |
| Flicker when firmware `NUM_LEDS` differs from pixels on the data path | Phantom pixels change `show()` timing (Nafas Test 6, step 2b) | Match `NUM_LEDS` to actual chain |

---

## 9. Installation Environment

- Location: Berlin, Germany. Venue, dates, indoor/outdoor: **TBD**.
- German power: Type F (Schuko) plug, 230V/50Hz. PSU must accept 230V input.
