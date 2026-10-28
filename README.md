# Agent Buddy for the round GC9A01 ESP32-S3 board

A board port of [Agent Buddy](https://github.com/DanWahlin/agent-buddy) (v0.16.1, by Dan Wahlin) to
a **round 1.28" GC9A01 240x240 ESP32-S3 board with a CST816D touch controller**, the kind sold as an
"ESP32-S3 1.28 box" / XiaoZhi-style round display. Upstream supports only the Waveshare
ESP32-S3-Touch-AMOLED-1.75 and the Waveshare ESP32-S3-Touch-LCD-1.54. This adds a third board
target using upstream's own board layer, so no shared firmware code changes.

This is an unofficial add-on. It is not affiliated with or endorsed by the Agent Buddy project.

## Status

Tested on one physical board. What I checked:

| Part | Result |
| --- | --- |
| Display (GC9A01, SPI, 40 MHz) | Works: upright, colours correct, 30 fps |
| Touch (CST816D) | Works: taps and gestures reach the firmware |
| Copilot character pack | Works (same 466x466 art as upstream, scaled to the 240x240 panel) |
| Audio | **Not supported.** The codec wiring of this board isn't known, so audio is stubbed out |
| Accelerometer / auto-rotation | **Not supported.** The board has no IMU |
| Companion daemon and agent hooks | **Not tested** on this board |
| Desktop app "Detect Device" / USB install | **Not updated.** Upstream's daemon doesn't list this board; build and flash from source |

The exact retail model of the board was not identified. It was recognised by its chips and pins,
which came from the author's earlier firmware for the same hardware. **If your board differs, the
screen may stay dark or touch may fail.** Verify first (see below).

## Hardware map

| Part | Pins / details |
| --- | --- |
| MCU | ESP32-S3, 16 MB flash, 8 MB PSRAM, USB-Serial/JTAG |
| Display | GC9A01 240x240 round, SPI3: SCLK 4, MOSI 2, DC 47, CS 5, RST 38, colour inversion on |
| Backlight | GPIO 42, PWM, active low (inverted) |
| Touch | CST816D on I2C: SDA 11, SCL 7, RST 6, INT 12, address 0x15 |
| Other | WS2812 LED on GPIO 48 (unused), BOOT button GPIO 0 |

The GC9A01 module needs vendor register writes after the stock init (gamma and gate timing, MADCTL
`0x48` = mirror-x + BGR). They are in `BoardDrivers.h` and are sent once, from the first
brightness call (right after the panel starts).

## Is this your board?

Do not flash this to a board you have not identified. Flashing firmware for the wrong display can
leave the screen dark (the ESP32-S3 ROM bootloader still works: hold BOOT while connecting USB).
Before you flash, **back up the current flash**:

```bash
esptool --port <port> read-flash 0 ALL backup.bin
```

and check that the existing firmware mentions a GC9A01 / CST816 (for example
`strings backup.bin | grep -i -E "GC9A01|CST816"`). A board with an AMOLED panel (CO5300) or a
CST9217 touch chip is **not** this board.

## Build and flash

1. Clone upstream and check out the release this patch is for:

   ```bash
   git clone --branch v0.16.1 https://github.com/DanWahlin/agent-buddy.git
   cd agent-buddy
   git apply /path/to/agent-buddy-round-gc9a01-128.patch
   python3 -m unittest tests.test_boards     # board layer consistency check
   ```

   (Or copy `firmware/AgentCompanion/src/boards/round_gc9a01_128/` into the clone and apply the
   changes to `boards/BoardConfig.h`, `boards/BoardDrivers.h` and `firmware/boards.json` by hand.)

2. Set up the toolchain exactly as upstream documents (docs/build-from-source.md): Arduino CLI,
   the `esp32:esp32@3.3.10` core, and Waveshare's pinned libraries from
   `waveshareteam/ESP32-S3-Touch-AMOLED-1.75` at revision `e4344e70c2fa78a13e8a06566507f1ba8af6672a`
   (GFX Library for Arduino with the GC9A01 driver, and SensorLib with CST816D support).

3. Build and flash:

   ```bash
   export ARDUINO_CLI=/path/to/arduino-cli
   export ARDUINO_CONFIG=/path/to/arduino-cli.yaml
   export WAVESHARE_DIR=/path/to/ESP32-S3-Touch-AMOLED-1.75
   BOARD=round-gc9a01-128 bash tools/arduino.sh build
   BOARD=round-gc9a01-128 bash tools/arduino.sh upload /dev/cu.usbmodemXXXX
   ```

   A successful build uses about 1.31 MB (62%) of the application partition.

4. On boot the serial log should show `READY: 466x466 ... character=copilot` and no touch errors.

## Notes and ideas

- The SPI clock is 40 MHz, the safe value used for upstream's 1.54" board. The original firmware for
  this board ran the bus at 80 MHz; raising `kSpiFrequency` may smooth frames.
- If touch moves the wrong way on your unit, flip `kTouchMirrorX` / `kTouchMirrorY` in
  `BoardConfig.h`. If the picture is mirrored, change the MADCTL byte in `applyVendorInit()`.
- Audio could be added by writing a board audio output (see `Es8311AudioOutput.h`) once the codec
  pins of the board are known.

## Licence

MIT, the same as upstream. `LICENSE` is Agent Buddy's licence (© 2026 Dan Wahlin); the files in this
repository are derived from that project and are offered under the same terms. Google Photos,
GitHub Copilot and other names belong to their owners.
