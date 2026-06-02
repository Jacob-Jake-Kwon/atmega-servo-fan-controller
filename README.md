# AVR-Based Automated Cooling Fan Controller

An embedded C application written for AVR microcontrollers (ATmega series) that controls a cooling fan and a sweeping servo motor using Hardware Timers and Fast PWM. The project features a custom, non-blocking button-debouncing driver to ensure highly responsive manual overrides.

## 🚀 Features

* **Hardware PWM Control:** Utilizes 16-bit Timer 3 to generate precise PWM signals for both DC fan speed control and servo motor positioning.
* **Automatic Oscillation (Sweep Mode):** Features a non-blocking state machine that smoothly sweeps a servo motor back and forth to simulate an oscillating fan.
* **Object-Oriented Button Driver:** Includes a lightweight, reusable `BUTTON` struct architecture featuring robust software debouncing ($15\text{ ms}$) for reliable state detection (`PUSH` and `RELEASE`).
* **Responsive State Machine:** Designed without heavy `_delay_ms()` blocking loops in the main cycle, allowing instant manual overrides even during active servo sweeping.

---

## 🛠️ Hardware & Pin Configuration

This code is optimized for AVR microcontrollers featuring **Timer 3** (e.g., ATmega128, ATmega2560).

| Component | Microcontroller Pin | Function |
| :--- | :--- | :--- |
| **LED Status Bar** | `PORTD` (Pins 0–7) | Displays system state visual feedback |
| **Fan Motor Control** | `PE5` (OC3A) | Fast PWM for speed control |
| **Servo Motor** | `PE3` (OC3C) | Fast PWM for oscillation positioning |
| **Button 1 (ON)** | `PG0` | Resets servo position, turns fan ON full speed |
| **Button 2 (OFF)** | `PG1` | Turns fan and status LEDs completely OFF |
| **Button 3 (Toggle)** | `PG2` | Toggles fan between medium speed and OFF |
| **Button 4 (Oscillate)** | `PG3` | Toggles the automatic sweeping mode ON/OFF |

---

## 🕹️ System Controls & Behavior

1. **Button 1 (Full On):** Activates all status LEDs, forces the fan to maximum speed (`OCR3A = 0`), cancels oscillation mode, and homes the servo.
2. **Button 2 (All Off):** Shuts off the fan (`OCR3A = 4998`) and turns off all status LEDs.
3. **Button 3 (Speed Toggle):** Alternates the fan between medium speed and OFF, while toggling the LED display.
4. **Button 4 (Oscillation Mode):** Engages or disengages a continuous $180^\circ$ style sweep. The servo increments smoothly with slight $200\text{ ms}$ pauses at its boundaries to prevent mechanical stress.

---

## 💻 Technical Details

* **Timer Frequency Setup:** Timer 3 is configured in **Fast PWM Mode** (Mode 14, using `ICR3` as the TOP value). 
* **ICR3 Value:** Set to `4999` to define the total PWM period.
* **Debouncing:** Employs a double-check validation system to filter out mechanical button noise without dragging down CPU performance.
