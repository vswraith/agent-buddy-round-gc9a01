#pragma once
// Device-only drivers for the round 1.28" GC9A01 ESP32-S3 board (see BoardConfig.h).
#include "BoardConfig.h"
#include "../../DisplayDma.h"
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <TouchDrvCSTXXX.hpp>
#include <Wire.h>

namespace copilot {
namespace board {
using TouchController = TouchDrvCST816;
constexpr uint8_t kTouchPointCapacity = 1;
static_assert(kTouchI2cAddress == CST816_SLAVE_ADDRESS, "Touch address differs from the driver.");

// This board has one variant.
enum class Variant : uint8_t { Standard };
constexpr const char* variantName(Variant) { return "standard"; }
inline Variant variant() { return Variant::Standard; }

// Runs first in setup(): keeps the backlight off until the first frame fades in.
inline void beginBoard() {
  ledcAttach(kDisplayBacklightPin, kDisplayBacklightPwmHz, kDisplayBacklightPwmBits);
  constexpr uint32_t maximum = (1u << kDisplayBacklightPwmBits) - 1;
  ledcWrite(kDisplayBacklightPin, kDisplayBacklightInverted ? maximum : 0);
}

// The bus is shared with DisplayDma, so it takes the SPI host only while it writes.
inline Arduino_DataBus& displayBus() {
  static Arduino_ESP32SPIDMA bus(kDisplayDcPin, kDisplayCsPin, kDisplaySckPin, kDisplayMosiPin,
                                 GFX_NOT_DEFINED, SPI3_HOST, true);
  return bus;
}

// Colour inversion on (ips = true), as Tamagooshi set it with invert = true.
inline DisplayPanel& displayPanel() {
  static DisplayPanel panel(&displayBus(), kDisplayResetPin, 0, true, kPanelSize, kPanelSize);
  return panel;
}

// The GC9A01 takes plain SPI mode 0, and pixels with data/command high.
inline DisplayScanout displayScanout() {
  return {SPI3_HOST, 0, 0, 0, false, 0, 0, kDisplayCsPin, kDisplayDcPin};
}

// This module needs vendor register writes after the stock GC9A01 init (gamma and gate timing,
// mirror-x, BGR). They come from the Tamagooshi bring-up, verified on this exact panel.
// 0x36 = 0x48 is MADCTL: MX (mirror x) | BGR. setup() sets the brightness right after the panel
// has started, so the first brightness call sends them once.
inline void applyVendorInit() {
  static constexpr uint8_t d62[] = {0x18, 0x0D, 0x71, 0xED, 0x70, 0x70,
                                    0x18, 0x0F, 0x71, 0xEF, 0x70, 0x70};
  static constexpr uint8_t d63[] = {0x18, 0x11, 0x71, 0xF1, 0x70, 0x70,
                                    0x18, 0x13, 0x71, 0xF3, 0x70, 0x70};
  Arduino_DataBus& bus = displayBus();
  displayPanel().startWrite();
  bus.writeCommand(0x62);
  for (uint8_t v : d62) bus.write(v);
  bus.writeCommand(0x63);
  for (uint8_t v : d63) bus.write(v);
  bus.writeCommand(0x36);
  bus.write(0x48);
  bus.writeCommand(0xC3);
  bus.write(0x1F);
  bus.writeCommand(0xC4);
  bus.write(0x1F);
  displayPanel().endWrite();
}

inline void setDisplayBrightness(uint8_t brightness) {
  static bool vendorInitDone = false;
  if (!vendorInitDone) {
    vendorInitDone = true;
    applyVendorInit();
  }
  constexpr uint32_t maximum = (1u << kDisplayBacklightPwmBits) - 1;
  const uint32_t duty = (brightness * maximum + 127) / 255;
  ledcWrite(kDisplayBacklightPin, kDisplayBacklightInverted ? maximum - duty : duty);
}

// The CST816D sleeps when idle and then misses I2C reads; the touch sweep reads it every 250 ms.
inline void configureTouch(TouchController& touch) { touch.disableAutoSleep(); }

// No IMU: begin() gives the reason, and the firmware then runs without orientation.
class NoAccelerometer {
 public:
  const char* begin() { return "No accelerometer on this board"; }
  bool read(float&, float&, float&) { return false; }
};

inline NoAccelerometer& accelerometer() {
  static NoAccelerometer sensor;
  return sensor;
}

// No audio output: begin() gives the reason, and the firmware then runs without sound.
class NoAudioOutput {
 public:
  const char* begin(unsigned) { return "no_audio_hardware"; }
  bool enable(uint8_t) { return false; }
  bool write(const uint8_t*, size_t) { return false; }
  void disable() {}
  int mclkPin() const { return -1; }
};

inline NoAudioOutput& audioOutput() {
  static NoAudioOutput output;
  return output;
}
}  // namespace board
}  // namespace copilot
