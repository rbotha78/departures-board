# Departures Board firmware instructions

## Change control

- Do not implement code, configuration, documentation, or generated-asset changes
  unless the user explicitly asks for implementation.
- Before any implementation, inspect the relevant code and present a concise
  plan covering the intended behavior, affected display targets, layout or
  message-flow changes, and validation steps.
- Wait for the user to confirm the plan before editing files, building, or
  uploading firmware.

## Display targets

This firmware supports two compile-time display targets:

- `esp32dev`: the original 256x64 SSD1322 OLED.
- `cyd`: the ESP32 Cheap Yellow Display (ILI9341) in 320x240 landscape mode.

Keep OLED behavior unchanged when making CYD-specific changes. Guard CYD-only
logic with `DISPLAY_CYD`; do not alter the shared display code unless the change
is required for both targets.

The CYD renderer is implemented by `U8G2_CYD_TFT` in `include/cydDisplay.h`.
It uses a full 320x240 U8g2 framebuffer (40x30 tiles). Legacy OLED partial
updates intentionally send the full CYD buffer because OLED tile geometry does
not map to the CYD canvas.

## CYD layout and fonts

- Keep the logical CYD canvas at 320x240. Do not reintroduce the former
  304x136 canvas.
- The station title belongs at the top, with a 10 px top margin.
- The departure clock belongs at the bottom, with a 10 px bottom margin. Use
  explicit top positioning (`setFontPosTop()`) for the CYD clock; font descent
  arithmetic clipped the clock on hardware.
- The clock width is anchored against `"88:88:88"` to prevent horizontal
  movement as its digits change.
- CYD National Rail service and feed rows use the fixed-width, bold
  `u8g2_font_7x14B_tf` font at 1x scale. The `ArimoRegular11` font and its
  license are retained for comparison and must not be removed unless explicitly
  requested.
- The primary CYD service uses two rows: scheduled time and destination first,
  then platform and expected/departure status. Keep this layout CYD-only; the
  OLED primary service remains a single row.
- Draw every non-empty `station.serviceMessage` immediately after the primary
  service at `y=108`. The second CYD service uses `y=130` for its
  time/destination and `y=152` for platform/status. Reserve `y=174–193` for
  the single-line RSS/NRCC station-message ticker and `y=204` for the clock
  clear area. Render the primary and second services together as one static
  panel so redraws clear stale service pixels. The primary service message and
  RSS/NRCC ticker are separate 20 px clipped scrolling bands; do not apply
  per-row clipping to the static service rows.
- During the first CYD board render, initialize and draw both ticker bands
  after building their queues and before `sendBuffer()`. Reset the detail font,
  text scale, and clip window before that first send.
- On CYD, route primary-service context (service message, calling points,
  origin/operator, seating, coach count) and station/NRCC notices to the
  primary message row. Reserve the bottom ticker for RSS and weather only.
- Right-align CYD live-status text to the fixed right-column boundary at
  `SCREEN_WIDTH - 12` so its final glyph is not clipped by the physical edge.
- The default CYD palette/color scheme is Amber (`CYD_COLOR_AMBER`, `0xFD80` / `255, 176, 0`).
- Hardware input defaults in `writeDefaultConfig()` are scoped by display target:
  - CYD: `brightness = 200`, `touch = true` (uses onboard BOOT button on GPIO 0).
  - OLED: `brightness = 20`, `touch = false` (optional TTP223 sensor).
- `displayedPrimaryServiceMessage` must remain sized to `MAXCALLINGSIZE + 12`
  (matching `cydPrimaryMessages` slot size) so that full calling-point strings
  can scroll without truncation.
- The 14 px detail glyphs must render with their baseline inside their row
  clipping rectangle. Use `railDetailBaseline()` and
  `railDetailScrollBaseline()` for National Rail service/feed rows rather than
  OLED-era `y - 1` baseline coordinates. Those earlier coordinates put the
  glyphs above CYD clip windows, leaving rows absent or partially cut off.

## Builds and deployment

When validating a CYD change, build **and upload** to the attached device:

```powershell
python -m platformio run -e cyd -t upload
```

The shared PlatformIO core can be incompatible with the installed Python
version. When needed, use the session-local PlatformIO core:

```powershell
$env:PLATFORMIO_CORE_DIR = '<session-state>\\files\\platformio-core'
python -m platformio run -e cyd -t upload
```

Also build the OLED environment for changes that touch shared firmware paths:

```powershell
python -m platformio run -e esp32dev
```

`TFT_eSPI` may warn that `TOUCH_CS` is undefined. This is expected because
TFT_eSPI touch support is not configured; it is not a build failure.

## Web assets

Source web assets are under `web\`. After changing one, regenerate the embedded
headers with:

```powershell
python scripts\generate_headers.py
```

The generated gzip headers can differ solely because of gzip timestamps. Do not
edit files under `include\webgui\` manually.
