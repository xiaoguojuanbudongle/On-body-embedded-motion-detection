# Group 14 — RTES F25 Demo

**Rule while filming:**  
- **Read only the quoted lines.**  
- Everything in *italics* is an action / camera note.  
- This script is written for a **handheld board demo** plus a **Teleplot screen**. No phone BLE is shown.

---

## 0:00–0:08 — Intro

*Camera: close-up on the board. Keep LED1 and LED2 in frame.*

> “Hi, we’re **Group 14**. This is our RTES F25 device on the B-L475E-IOT01A.  
> It reports three outputs every window: **tremor level**, **dyskinesia level**, and **FOG**.”

---

## 0:08–0:20 — What the pipeline does (short and concrete)

*Camera: keep holding the board, then tilt to show the laptop screen in the background.*

> “On the board, we sample the IMU at **52 hertz** and process fixed **3-second windows**.  
> For each window we run an FFT and measure energy in **3–5 Hz** for tremor and **5–7 Hz** for dyskinesia, plus a step estimate for gait.”

---

## 0:20–0:32 — Teleplot proof (what each trace means)

*Camera: cut to laptop screen. Teleplot is open and already connected at 115200.*  
*Show traces: `steps`, `tremor_rms`, `dysk_rms`, `tremor_lvl`, `dysk_lvl`, `fog`.*

> “Teleplot is showing exactly what the board outputs per window:  
> `steps` is our step estimate, `tremor_rms` and `dysk_rms` are band RMS values, and the three discrete outputs are `tremor_lvl`, `dysk_lvl`, and `fog`.”

---

## 0:32–0:44 — Baseline

*Camera: back to the board. Hold it still for one full 3-second window.*  
*Optional: quick cut to Teleplot right after the update.*

> “First, baseline. With the board still, `steps` stays near zero and both band RMS values stay low.  
> That corresponds to **level 0**, so both LEDs stay off.”

---

## 0:44–1:02 — Tremor case

*Camera: board close-up. Do small, steady, rhythmic shaking for 1–2 windows (slow and regular).*  
*Make sure LED1 is visible. Cut to Teleplot after an update.*

> “Now we demonstrate **tremor**, which is rhythmic motion concentrated in the **3–5 hertz** band.  
> I’m shaking slowly and regularly. You should see `tremor_rms` rise, `tremor_lvl` increase, and **LED1 turns on**.”

---

## 1:02–1:20 — Dyskinesia case

*Camera: board close-up. Switch to faster and more irregular movement for 1–2 windows.*  
*Make sure LED2 is visible. Cut to Teleplot after an update.*

> “Next is **dyskinesia**, which we model as stronger, more irregular motion with energy in **5–7 hertz**.  
> I’m moving faster and less regularly. `dysk_rms` rises, `dysk_lvl` increases, and **LED2 turns on**.”

---

## 1:20–1:45 — FOG case

*Camera: board in hand. First do a steady ‘walking-like’ pattern for at least 2 windows.*  
*Then suddenly stop completely and hold still until the next update.*  
*Try to capture the brief blink of both LEDs. Quick cut to Teleplot right after.*

> “Finally, **freezing of gait**. We first show a walking pattern so `steps` is clearly non-zero.  
> Now we suddenly stop. On the next window, the step history drops and the device sets `fog` to 1, and **both LEDs blink briefly**.”

---

## 1:45–2:00 — Wrap-up

*Camera: board close-up, then quick cut to Teleplot showing the latest values.*

> “To summarize: we meet the challenge requirements with **52-hertz IMU sampling**, **3-second windowing**, and **on-board FFT-based band detection** for tremor and dyskinesia.  
> We also produce a **FOG flag** using step history, and we provide **real-time outputs** through Teleplot plots and the on-board LEDs.  
> The behaviour is repeatable: when we repeat the same motion patterns, the levels and indicators update consistently every window.”

---

## Quick filming tips (not spoken)

- Wait for the 3-second window boundary before expecting any level/LED change.
- If a level does not change, exaggerate the motion for one more window.
- Keep the board close to the camera when you expect LED changes, especially for the FOG blink.
