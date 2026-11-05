"""Voltage-drop estimate for a dual-fed WS2815 power loop, calibrated to Nafas Test 2.

Frames are current sinks proportional to pixel count; cable resistance is proportional
to length. Both feeds are held at the same voltage. Output is scaled so the Nafas loop
reproduces its measured 2.04V worst drop at 50% white (docs/ELECTRONICS_GUIDE.md).

Usage: python3 tools/loop_model.py [gap_m] [lead_m]
"""
import sys
import numpy as np

PX = {1: 89, 2: 36, 3: 81, 4: 138, 5: 27, 6: 75, 7: 33, 8: 109, 9: 54,
      10: 45, 11: 99, 12: 36, 13: 60, 14: 24}  # Andrum spreadsheet NUM_LEDS
NAFAS_PX = [48, 105, 90, 138, 36, 90, 105, 48]       # Nafas builds, physical order
NAFAS_50 = [0.87, 1.46, 1.84, 2.04, 1.94, 1.75, 1.73, 0.87]  # Test 2 drops (V)
FULL_PER_50 = 3.47 / 2.04  # Nafas Test 3 / Test 2 worst drop


def drops(loads, lead=3.0, gap=2.0):
    n = len(loads)
    seg = [lead] + [gap] * (n - 1) + [lead]
    G = np.zeros((n, n))
    for i in range(n):
        G[i, i] = 1 / seg[i] + 1 / seg[i + 1]
        if i > 0:
            G[i, i - 1] = -1 / seg[i]
        if i < n - 1:
            G[i, i + 1] = -1 / seg[i + 1]
    return np.linalg.solve(G, np.array(loads, float))


K = max(NAFAS_50) / drops(NAFAS_PX).max()

LOOPS = {
    "A-D": [8, 13, 9, 12, 14, 5, 7, 10, 6, 11],
    "E": [4, 2, 3, 1],
}

if __name__ == "__main__":
    gap = float(sys.argv[1]) if len(sys.argv) > 1 else 2.0
    lead = float(sys.argv[2]) if len(sys.argv) > 2 else 3.0
    print("Nafas model:", np.round(drops(NAFAS_PX) * K, 2), "measured:", NAFAS_50)
    for name, order in LOOPS.items():
        d = drops([PX[m] for m in order], lead, gap) * K
        print(f"Loop {name}: {sum(PX[m] for m in order)} px, gap {gap}m, lead {lead}m")
        print("  frames:       ", order)
        print("  drop @50% (V):", np.round(d, 2).tolist())
        print(f"  worst @100%:  ~{12.0 - d.max() * FULL_PER_50:.2f}V (PSU 12.0V)")
