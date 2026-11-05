# ANDETAG NAFAS: Electronics Reference

## Nafas Installation / Tanweer Festival + Shurooq HQ (permanent)

---

## 1. System Overview

8 frames, 5 models, 666 WS2815 pixels total. Single ESP32 controller drives the entire chain. 12V DC power fed from both ends of a 20m loop. Frames are connected in parallel to the 12V bus (each frame taps power directly; current does not flow serially through pixels). Data is daisy-chained through all frames in series.

### Frame Order (fixed, cannot be rearranged)

| Position | Frame    | Pixels | Pixel index range |
|----------|----------|--------|-------------------|
| 1        | ANDRUM9  | 48     | 0-47              |
| 2        | ANDRUM8  | 105    | 48-152            |
| 3        | ANDRUM11 | 90     | 153-242           |
| 4        | ANDRUM4  | 144    | 243-386           |
| 5        | ANDRUM2  | 36     | 387-422           |
| 6        | ANDRUM11 | 90     | 423-512           |
| 7        | ANDRUM8  | 105    | 513-617           |
| 8        | ANDRUM9  | 48     | 618-665           |

### Cable Layout

```
PSU ──3m── F1 ──2m── F2 ──2m── F3 ──2m── F4 ──2m── F5 ──2m── F6 ──2m── F7 ──2m── F8 ──3m── PSU
```

Total loop: 20m. Cable: 22AWG / 0.3mm², 3-conductor (12V, GND, DATA).

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
#define NUM_LEDS 666

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
2. Directly to the pixel loop VCC/GND at both ends

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

## 5. Power Budget

| Condition    | Per pixel | Total (666px) | At 12V |
|--------------|-----------|---------------|--------|
| Full white   | 15mA      | 9.99A         | 120W   |
| 30% avg load (estimate) | ~4.5mA | ~3.00A | ~36W |
| Breathing animation (measured) | see below | ~3.7A peak 1-min avg | ~44W peak 1-min avg |

Fed from both ends, each side carries roughly half.

### Measured Production Animation Load (Test 7)

Brightness output measured on production `andetag-nafas` breathing animation:

| Metric | Measured | Notes |
|--------|----------|-------|
| Max single-frame output | **64%** | Peak instantaneous load within one animation frame |
| Highest 1-minute average | **35%** | Peak sustained average, first 15 min of run |

The prior 30% average estimate was conservative; real sustained load peaks around **35%**. Single-frame peaks to **64%** — well below full white, and far below levels that caused voltage issues in Test 3 (100% white → 8.53V at F4).

### Voltage at Worst-Case Frame (ANDRUM4, position 4)

ANDRUM4 is the largest frame (144px) and sits 9m from Feed A. It's the heaviest load at the longest path.

| Condition    | Estimated voltage at ANDRUM4 | Status       |
|--------------|------------------------------|--------------|
| 30% avg load (estimate) | ~10.6V                  | Fine         |
| Breathing animation (measured ~35% 1-min avg) | ~11.0V at F4 (Test 7, 5 min) | Fine |
| Full white   | 8.53V measured at F4         | Below 9.5V   |

Full white does not occur in the breathing animation (measured max single frame: 64%). At measured operating load, all frames are well within spec.

---

## 6. Testing Procedures

Status: **All tests complete (1–7 PASS).** System validated for installation.

### Test 1: Baseline PSU Voltage — PASS

Measure DC voltage at PSU output terminals with no load, then with all frames connected (pixels dark). Expected: 12.0-12.5V.

| Measurement | Result | Expected | Status |
|-------------|--------|----------|--------|
| PSU output (no load) | 12.5V | 12.0-12.5V | PASS |
| PSU output (frames connected, no extension cables, pixels dark) | 12.12V | 12.0-12.5V | PASS |

### Test 2: Voltage Distribution Under Load — PASS

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

### Test 3: Voltage Under Stress — COMPLETE

Set all pixels to full white 100%. Measure voltage at ANDRUM4 (frame 4). Establishes the actual voltage floor. Expected to dip below 9.5V; acceptable since this state is momentary.

Load: full white (`CRGB::White`), full 666-pixel chain with extension cables.

| Point | Voltage | Notes |
|-------|---------|-------|
| PSU | 12.00V | Down 0.20V from Test 2 load reading |
| F4 (ANDRUM4) | 8.53V | Below 9.5V regulation threshold; above logic-failure range (~6-7V) |

Visual: white appeared uniform across all frames despite F4 voltage drop. Confirms full-white state is momentary-only in production animation; not a sustained operating condition.

### Test 4: Data Integrity (Visual) — PASS

Run a single-pixel chase at maximum speed through all 666 pixels. Observe each frame boundary for stuck pixels, color errors, flickering, or skipped pixels.

Result: clean chase through all 666 pixels. No stuck, skipped, or miscolored pixels at frame boundaries. Motion smooth and continuous.

### Test 5: Data Integrity (Quantitative) — PASS

Measure `FastLED.show()` duration. Expected: ~20ms for 666 WS2815 pixels at 800kHz (~50 FPS). If significantly longer, investigate firmware conflicts (WiFi, interrupts).

Initial attempt (discarded): single `micros()` around `show()` with static all-black buffer reported 927 µs — wrong; ESP32 RMT timing must use a moving-pixel lap test (no dirty-flag skip; `show()` always transmits full buffer).

Redo method: moving-pixel chase over 666 frames. Reports single-frame `show()` µs and full 666-frame lap time.

| Measurement | Result | Expected | Status |
|-------------|--------|----------|--------|
| Single `show()` µs | ~30,385 µs, stable | ~20,000 µs wire-only | PASS (see note) |
| 666-frame lap | 21,240 ms, stable | ~13,000–14,000 ms wire-only | PASS (see note) |
| Per-frame average | ~30,430 µs | ~20,000 µs | PASS (see note) |

Note: ~30 ms/frame is ~50% above theoretical on-wire time (~20 ms), likely ESP32 RMT driver overhead on this controller. Timing is stable with no jitter between laps. Effective max refresh ~33 FPS — adequate for breathing animation. Investigate further only if flickering correlates with frame rate (Test 6). Measured timings above were taken under the prior 698-pixel firmware count; re-check if show() budget becomes tight.

### Test 6: Flickering Isolation — PASS

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

### Test 7: Long-Duration Stability — PASS

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

- [ ] PSU output confirmed ~12V
- [ ] Barrel jack connected to SP803E
- [ ] 12V bus connected at both ends of frame loop
- [ ] GPIO16 connected to frame 1 (ANDRUM9) DI+BI
- [ ] Each inter-frame: DO only to DI+BI (not DO+BO)
- [ ] BO cut and isolated on all inter-frame cables (cannot contact other conductors)
- [ ] GND common between PSU, controller, and all frames
- [ ] USB-C→USB-A→USB-C path available for uploads (straight USB-C will not work)

---

## 8. Known Issues

| Issue | Cause | Fix |
|-------|-------|-----|
| SP803E not detected via straight USB-C | CC pin negotiation failure on board | Use USB-C→USB-A→USB-C (adapter/hub) |
| Upload fails at 921600 baud | CH340C limitation | Set upload speed to 460800 |
| Sketch too large (default partition) | 1.25MB app limit | Use Huge APP (3MB No OTA) partition |
| Signal corruption at frame boundaries | DO+BO paired together | Only connect DO to DI+BI |
| F6–F8 flicker, F1–F5 clean (canonical order) | Uncut BO conductor touching adjacent pin at cable angle | Fully cut and isolate BO on all inter-frame cables |
| Color degradation at ANDRUM4 under extreme load | Voltage drop at loop midpoint | Firmware brightness cap or thicker 3m lead cables |

---

## 9. Installation Environment

- Location: Mleiha, Sharjah, UAE (festival Nov 20-22), then permanent at Shurooq HQ (indoors)
- UAE power: Type G plug, 230V/50Hz. PSU must be rated 100-240V input.
- November ambient temp: ~30-33C. Indoor permanent placement is climate-controlled.
