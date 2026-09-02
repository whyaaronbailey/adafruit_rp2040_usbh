# Installing SpeechKey — step by step

**Time: about 2 minutes. Nothing gets installed on your PC.** You are copying one
file onto the adapter board, which appears as a USB drive.

If anything goes wrong, jump to [Troubleshooting](#troubleshooting) — you cannot
permanently break the board.

---

## What you're installing onto

The **Adafruit RP2040 USB Host Feather** — the small circuit board. It has:

- a **USB-C** socket on one end → this goes to your **PC**
- a **USB-A** socket (the normal rectangular one) → your **Tartarus** plugs in here
- two small buttons near the USB-C socket, marked **BOOT** (or BOOTSEL) and **RESET**

---

## Step 1 — Put the board into "receive firmware" mode

You need the board to appear as a USB drive. There are two buttons; the order
matters.

1. Unplug the board from the PC.
2. **Press and HOLD the BOOT button.** Keep holding it.
3. While still holding BOOT, plug the USB-C cable into the PC.
4. Count to two, then let go of BOOT.

✅ **A new drive called `RPI-RP2` appears** (File Explorer, like a flash drive).
That means it's ready.

> **If no drive appears:** unplug, and try again holding BOOT a little longer
> before plugging in. Some USB-C cables are charge-only and carry no data — if
> the board never appears at all, try a different cable.

> **Already plugged in?** Hold BOOT, tap RESET once, then release BOOT.

---

## Step 2 — Copy the firmware onto it

Drag **`tartarus2_sk.uf2`** onto the `RPI-RP2` drive.

That's the whole installation.

✅ **The drive disappears by itself after a second or two.** That is exactly what
should happen — it means the firmware was accepted and the board restarted. It is
**not** an error, and Windows may show a "device removed unexpectedly" balloon.
Ignore it.

---

## Step 3 — Plug in the Tartarus

Plug the Tartarus's USB cable into the **USB-A** socket on the adapter board.

✅ Within a few seconds **the Tartarus keys light up light blue.** That means the
firmware is running and talking to the keypad.

> Lights but no reaction to keys, or no lights: unplug the Tartarus from the
> adapter, wait 5 seconds, plug it back in. The adapter can stay in the PC.

---

## Step 4 — Check Windows sees a PowerMic

Optional but worth doing once.

1. Right-click Start → **Device Manager**
2. Expand **Human Interface Devices**

✅ You should see entries for a **PowerMicII-NS** device.

You can also open Settings → Bluetooth & devices → Devices and look for
**PowerMicII-NS**.

**No drivers are needed and none will be installed.** Windows already knows how to
talk to it.

---

## Step 5 — Try it in PowerScribe

1. Open PowerScribe and sign in as normal.
2. Open any report.
3. **Press key 20** (the big key under your thumb).

✅ **Dictation starts, and all the Tartarus keys turn red.** Press 20 again to stop
— the keys go back to blue.

It works whatever window is focused; you don't have to click into PowerScribe
first.

---

## Troubleshooting

**PowerScribe doesn't react to key 20**

Most often, a **real PowerMic is also plugged into that PC.** PowerScribe binds to
whichever PowerMic it finds first, and it may have picked the real one. Unplug the
real PowerMic, then fully quit and reopen PowerScribe.

Otherwise, quit PowerScribe completely (check the system tray), then reopen it —
it only looks for a microphone at startup.

**The keys turn red but nothing is dictated**

That's the colour drifting out of step, not a key failure — see "One honest
limitation" in `README.md`. **Hold the thumb key and press 20** to resync the
colour.

**The keys do the wrong things**

Expected — the shipped macros are set up for the sender's PACS (Epic/GE/McKesson).
Open `CLAUDE_PROMPT.md` with Claude to change them.

**Nothing works at all / I want to start over**

You cannot brick this board. Repeat Steps 1–2 with any `.uf2` file. The BOOT
button always works, no matter what firmware is on it.

**I want to go back to a plain keyboard**

Install any other QMK `.uf2` the same way, or a stock Tartarus setup — Step 1 and 2
overwrite whatever is there.

---

## Reinstalling later

Same as above: hold BOOT while plugging in, drag the `.uf2` on. Your key
customisations live in the board's own memory and survive a reinstall of the *same*
firmware version.
