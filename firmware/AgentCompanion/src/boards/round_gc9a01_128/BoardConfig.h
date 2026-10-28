#pragma once
// Round 1.28" GC9A01 ESP32-S3 board (Spotpear-style "ESP32-S3-1.28-Box": 240x240 round SPI LCD,
// CST816D touch, WS2812 LED, 16 MB flash, 8 MB PSRAM). This is the board that Tamagooshi calls
// "round-esp32s3". Every pin below comes from that firmware (lib/drivers/hal/lgfx_round.h and
// touch_buttons.cpp), which was verified on this hardware. There is no IMU, and no audio wiring
// is known, so both are left out. Plain constants only: native tests include this file without
// the vendor libraries.
#include <cstdint>

class Arduino_GC9A01;

namespace copilot {
namespace board {
// Declared here so display code can name the panel without the vendor library.
using DisplayPanel = ::Arduino_GC9A01;
constexpr char kId[] = "round-gc9a01-128";
constexpr char kName[] = "Round 1.28\" GC9A01 ESP32-S3 (Spotpear-style)";
constexpr char kDisplayPanelName[] = "GC9A01";
constexpr char kTouchControllerName[] = "CST816D";
constexpr char kImuName[] = "none";
}  // namespace board

// The firmware draws 466x466 content on every board. This panel is 240x240, so scanout shows the
// content at about half size (see ScreenTransform.h). The panel and the content are both round.
constexpr int kDisplaySize = 466;
constexpr int kPanelSize = 240;
constexpr int kDisplayCsPin = 5;
constexpr int kDisplayDcPin = 47;
constexpr int kDisplaySckPin = 4;
constexpr int kDisplayMosiPin = 2;
constexpr int kDisplayResetPin = 38;
// Tamagooshi ran this bus at 80 MHz; start at 40 MHz, the clock the 1.54" board needs for pins
// on the GPIO matrix, and raise it only after the picture is confirmed stable.
constexpr int kSpiFrequency = 40000000;
constexpr int kDisplayBacklightPin = 42;
constexpr bool kDisplayBacklightInverted = true;  // The backlight is on when the pin is low.
constexpr uint32_t kDisplayBacklightPwmHz = 5000;
constexpr uint8_t kDisplayBacklightPwmBits = 10;
constexpr int kBrightness = 155;

// No power latch on this board.
constexpr int kBatteryPowerLatchPin = -1;

// Audio: this board has a speaker, but its codec wiring is not known, so audio is left out.
constexpr int kAudioMclkPin = -1;
constexpr int kAudioBclkPin = -1;
constexpr int kAudioWordSelectPin = -1;
constexpr int kAudioDataOutPin = -1;
constexpr int kAudioAmplifierPin = -1;
constexpr unsigned kAudioAmplifierStartupMs = 0;
constexpr uint8_t kAudioCodecI2cAddress = 0x18;
constexpr int kAudioMinimumCodecVolume = 70;
constexpr int kAudioMaximumCodecVolume = 83;

// Touch has its own I2C bus (Arduino Wire, I2C port 0).
constexpr unsigned kI2cPort = 0;
constexpr uint32_t kI2cFrequency = 400000;
constexpr int kTouchSda = 11;
constexpr int kTouchScl = 7;
constexpr int kTouchInterrupt = 12;
constexpr int kTouchReset = 6;
constexpr uint8_t kTouchI2cAddress = 0x15;
// The panel is mirrored in X (MADCTL 0x48, see BoardDrivers.h); correct this on the device if
// touch moves the wrong way.
constexpr bool kTouchMirrorX = false;
constexpr bool kTouchMirrorY = false;
constexpr uint8_t kImuI2cAddress = 0x00;  // No IMU.

// The BOOT button. Held at power-on it starts the ROM downloader; after that it is free to read.
constexpr int kBootButtonPin = 0;
}  // namespace copilot
