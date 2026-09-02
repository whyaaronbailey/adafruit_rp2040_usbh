# SpeechKey (SK) — Razer Tartarus → PowerScribe dictation, firmware only

This turns a **Razer Tartarus V2/Pro** into a radiology keypad that can start and
stop PowerScribe dictation by itself — **no software installed on the PC, no
background app, no drivers, and it works no matter which window has focus.**

It does that by making the little USB adapter *pretend to be a Nuance PowerMic II*.
PowerScribe genuinely believes a PowerMic is plugged in, so pressing key 20 is the
same to it as squeezing the Record button on the real handset. Because nothing is
installed on the PC, this works on locked-down hospital workstations where you
can't install anything.

The Tartarus's per-key RGB lighting is driven by the firmware too — the keys turn
red while you're dictating, and the scroll key pulses while auto-scrolling.

---

## What you need

| | |
|---|---|
| **Razer Tartarus V2 or Pro** | the keypad itself |
| **Adafruit RP2040 USB Host Feather** | the small adapter board this firmware runs on |
| A USB cable | board → PC |

The Tartarus plugs into the **USB-A host port** on the adapter board. The adapter
plugs into the PC. The PC never sees the Tartarus — it only sees the adapter,
which presents itself as a keyboard, a mouse, and a PowerMic II.

**→ To install the firmware, follow [INSTALL.md](INSTALL.md).** It's four steps
and takes about two minutes.

---

## The default layout

Physical keys are numbered the way Razer numbers them: 01–05 top row, 06–10, 11–15,
16–19, then the big key 20 under your thumb.

```
┌──────┬──────┬──────┬──────┬──────┐
│  01  │  02  │  03  │  04  │  05  │   ┌──────┐
│Annot │WL Sof│WL Lun│WL Vas│ Zoom │   │ SW   │  scroll wheel up
├──────┼──────┼──────┼──────┼──────┤   ├──────┤
│  06  │  07  │  08  │  09  │  10  │   │ SC   │  scroll wheel down
│ Epic │WL Bon│SCROLL│WL Bra│Arrow │   └──────┘
├──────┼──────┼──────┼──────┼──────┤
│  11  │  12  │  13  │  14  │  15  │   ┌──────┐
│CopyAc│FAST ↑│SCROLL│FAST ↓│Measur│   │THUMB │  hold = layer 2
├──────┼──────┼──────┼──────┼──────┤   └──────┘
│  16  │  17  │  18  │  19  │      │
│ GE   │Spine │Spine │Spine │      │   ┌──────┐
│      │  C   │  T   │  L   │      │   │  20  │  DICTATE
└──────┴──────┴──────┴──────┴──────┘   └──────┘

D-pad:  ← previous field    → next field    ↑↓ mouse move
```

### The important keys

| Key | What it does |
|---|---|
| **20** | **Start/stop dictation.** This is the PowerMic Record button. |
| **D-pad ← / →** | Previous / next field in the PowerScribe report |
| **08** | Scroll up through images, continuously — press again to stop |
| **13** | Scroll down, continuously |
| **12 / 14** | Same but fast |
| **Wheel up/down** | Scroll up / down |
| **THUMB (held)** | Layer 2 — see below |

### Layer 2 (hold the thumb key)

| Key | What it does |
|---|---|
| 08 / 13 | Scroll up / down at the *alternate* slow speed |
| 12 / 14 | Scroll up / down at the *alternate* fast speed |
| 18 / 19 | Previous / next lighting effect |
| **20** | **Fix the dictation colour** if it gets out of step (see below) |
| 01–17 | The remaining PACS tool macros |

Everything on keys 01–19 is a **macro** — a little recorded sequence of clicks and
keystrokes (e.g. "click, wait, press Numpad 1" for a window/level preset). The
ones shipped here are set up for the sender's PACS (Epic / GE / McKesson,
window-level presets, spine labels). **They will almost certainly need changing for
yours** — that's what `CLAUDE_PROMPT.md` is for.

---

## The lighting

The firmware drives the Tartarus's RGB directly. Out of the box:

| Situation | Colour |
|---|---|
| Idle | light blue |
| **Dictating** | **all keys red** |
| A key you're holding | yellow |
| Auto-scrolling | the scroll key pulses yellow |

Hold **thumb + 18/19** to cycle through 11 lighting effects: static, spectrum
cycle, rainbow wave, colour wheel, three breathing modes, three starlight modes,
and a reactive mode where pressed keys flash cyan. Colours and brightness can be
changed without reflashing (see `CLAUDE_PROMPT.md`).

### One honest limitation

PowerScribe **never tells the PowerMic whether it's actually recording** — the real
handset only knows because PowerScribe opens *its* built-in microphone, and this
adapter has no microphone. So the red "dictating" colour is the firmware's own
count of your key presses. If dictation ever starts or stops *another* way — you
click the microphone button on screen, or it times out — the colour gets out of
step (red when idle, blue when dictating).

Two fixes, both built in:
- **Hold thumb + press 20** — flips the colour back without touching PowerScribe.
- It gives up on its own after **15 minutes** of showing red.

The keys themselves are never wrong; only the colour can drift.

---

## Customising it

You don't need to rebuild the firmware to change which key does what, or the
colours. Two ways:

1. **`CLAUDE_PROMPT.md`** — open it with Claude and it will walk you through
   changes and make them for you. Easiest by far.
2. **`tools/usevia.app.html`** — open in Chrome or Edge, click Connect, remap keys
   by clicking them. (Firefox and Safari can't talk to USB devices.)

---

## Files here

| File | What it is |
|---|---|
| `tartarus2_sk.uf2` | The firmware. This is the thing you install. |
| `INSTALL.md` | **Start here.** Step-by-step install. |
| `CLAUDE_PROMPT.md` | Hand this to Claude to customise keys, macros, colours. |
| `tools/usevia.app.html` | Point-and-click key remapper (Chrome/Edge) |
| `tools/pmcfg.ps1` | PowerShell helper for scripted changes |

---

*Nuance, PowerMic, and PowerScribe are trademarks of Microsoft/Nuance. This
firmware reproduces a publicly observable USB interface so that a keypad can
interoperate with software the user already licenses.*
