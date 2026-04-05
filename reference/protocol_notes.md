# NS2 Pro Controller - Protocol Notes

> Source: ikz87/NSW2-controller-enabler (enable_hid.py) + djedditt/nsw2usb-con (nsw2usb-con.py)

## Product IDs

| Variant        | VID    | PID    | Source              |
|----------------|--------|--------|---------------------|
| NS2 Pro Con    | 0x057E | 0x2069 | djedditt (PID_SW2_PROCON) |
| NS2 GC NSO Con | 0x057E | 0x2073 | ikz87 + djedditt (PID_SW2_NGCCON) |
| Unknown        | 0x057E | 0x2066 | plan doc only       |
| Unknown        | 0x057E | 0x2067 | plan doc only       |

USB Interface: **1** (both scripts use `INTERFACE = 1`)

---

## Init Sequence

### Minimal (djedditt - Pro Con 0x2069)
Just 2 commands, 50ms apart. Confirmed working on real hardware.

```
Step 1 - INIT:
  03 91 00 0D 00 08 00 00 01 00 FF FF FF FF FF FF
Step 2 - Player 1 LED:
  09 91 00 07 00 08 00 00 01 00 00 00 00 00 00 00
Delay: 50ms between each command
```

### Full sequence (ikz87 - GC NSO 0x2073)
17 commands, 50ms apart. May be needed for GC variant.

```
 1. INIT:              03 91 00 0D 00 08 00 00 01 00 FF FF FF FF FF FF
 2. unknown 0x07:      07 91 00 01 00 00 00 00
 3. unknown 0x16:      16 91 00 01 00 00 00 00
 4. request MAC:       15 91 00 01 00 0E 00 00 00 02 FF FF FF FF FF FF FF FF FF FF FF FF
 5. LTK request:       15 91 00 02 00 11 00 00 00 FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
 6. unknown 0x15/0x03: 15 91 00 03 00 01 00 00 00
 7. unknown 0x09:      09 91 00 07 00 08 00 00 00 00 00 00 00 00 00 00
 8. IMU cmd 0x02:      0C 91 00 02 00 04 00 00 27 00 00 00
 9. unknown 0x11:      11 91 00 03 00 00 00 00
10. unknown 0x0A:      0A 91 00 08 00 14 00 00 01 FF FF FF FF FF FF FF FF 35 00 46 00 00 00 00 00 00 00 00
11. IMU cmd 0x04:      0C 91 00 04 00 04 00 00 27 00 00 00
12. ENABLE_HAPTICS:    03 91 00 0A 00 04 00 00 09 00 00 00
13. unknown 0x10:      10 91 00 01 00 00 00 00
14. unknown 0x01:      01 91 00 0C 00 00 00 00
15. unknown 0x03:      03 91 00 01 00 00 00
16. unknown 0x0A alt:  0A 91 00 02 00 04 00 00 03 00 00
17. SET PLAYER LED:    09 91 00 07 00 08 00 00 01 00 00 00 00 00 00 00
```

**Note:** Both scripts share the same INIT and LED bytes. Start with minimal sequence for 0x2069.

---

## Input Report Format

> Source: ikz87 enable_hid.py lines 289-339
> report = hid_device.read(64) → 64 bytes
> payload = report[1:]  ← skip report ID byte

| report[] | payload[] | Field             | Notes                         |
|----------|-----------|-------------------|-------------------------------|
| [0]      | -         | Report ID         | skip                          |
| [1]      | [0]       | unknown           |                               |
| [2]      | [1]       | unknown           |                               |
| [3]      | [2]       | Button byte 0     | see below                     |
| [4]      | [3]       | Button byte 1     | see below                     |
| [5]      | [4]       | Button byte 2     | see below                     |
| [6..8]   | [5..7]    | Left stick        | 12-bit packed (3 bytes → 2 values) |
| [9..11]  | [8..10]   | Right stick       | 12-bit packed                 |
| [13]     | [12]      | Left trigger raw  | range 36-240 → remap 0-255    |
| [14]     | [13]      | Right trigger raw | range 36-240 → remap 0-255    |

### Button byte 0 (report[3] / payload[2])
| bit | Button  |
|-----|---------|
| 0   | B       |
| 1   | A       |
| 2   | Y       |
| 3   | X       |
| 4   | R       |
| 5   | ZR      |
| 6   | Plus (+)|
| 7   | R Stick |

### Button byte 1 (report[4] / payload[3])
| bit | Button      |
|-----|-------------|
| 0   | DPad Down   |
| 1   | DPad Right  |
| 2   | DPad Left   |
| 3   | DPad Up     |
| 4   | L           |
| 5   | ZL          |
| 6   | Minus (-)   |
| 7   | L Stick     |

### Button byte 2 (report[5] / payload[4])
| bit | Button        |
|-----|---------------|
| 0   | Home          |
| 1   | Capture       |
| 2   | 背鍵右 (back right) |
| 3   | 背鍵左 (back left)  |
| 4-7 | unknown       |

> ✓ Confirmed from live hardware capture on NS2 Pro Controller (PID 0x2069)

### Stick decoding (12-bit packed)
```
3 bytes → 2 x 12-bit values:
  a = bytes[0] | ((bytes[1] & 0x0F) << 8)   ← X axis
  b = (bytes[1] >> 4) | (bytes[2] << 4)      ← Y axis
Center = 2048 (12-bit midpoint, range 0-4095)
```

### Trigger remapping
```
Raw range:  36–240 (0x24–0xF0)
Output:     0–255
Formula:    ((raw - 36) / 204.0) * 255
```

---

## LED Command
```
Player 1: 09 91 00 07 00 08 00 00 01 00 00 00 00 00 00 00
```
(Same in both scripts)

---

## Open Questions
- [ ] Rumble output format (Phase 4 - not in any existing script)
- [ ] Does NS2 Pro Con (0x2069) need only 2 init commands, or does it need the full 17?
      → Test with minimal first; add more if controller doesn't respond
- [ ] Keepalive packet required? Interval? (not present in either script)
- [ ] What is report[1] and report[2] in the input report?
