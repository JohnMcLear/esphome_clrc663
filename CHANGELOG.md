# Changelog

All notable changes to the ESPHome CLRC663 component will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial release of CLRC663 component for ESPHome
- I2C interface support with configurable address (default 0x28)
- SPI interface support with chip select pin
- Binary sensor platform for specific UID detection
- Health check with auto-recovery (enabled by default)
- Exponential backoff for communication errors
- Support for both hyphen and colon-separated UID formats
- Optional IRQ pin support for interrupt-driven operation
- Optional hardware reset pin support
- `on_tag` and `on_tag_removed` automation triggers
- Comprehensive documentation and examples
- CI/CD workflow for automated testing

### Supported Protocols
- ISO/IEC 14443A (MIFARE Classic, NTAG, DESFire)
- ISO/IEC 14443B
- FeliCa
- ISO/IEC 15693 (long-range RFID)
- NFC Forum Type 1/2/3/4

### Features
- Read range: Up to 9cm with proper antenna
- I2C frequency: 400kHz (fast mode) or 1MHz (fast mode+)
- Dual voltage support: 3.3V or 5V
- Health monitoring with configurable intervals
- Auto-reset on communication failures
- Compatible with ESP32 and ESP8266

## [1.0.0] - TBD

### Initial Release
- First stable release
- Tested on ESP32 and ESP8266
- Validated with multiple CLRC663 modules
- Full ESPHome integration
