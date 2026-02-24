# ESPHome CLRC663 Component (Enhanced)

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)

An enhanced external ESPHome component for the CLRC663 NFC/RFID reader. Compatible with ESPHome's configuration patterns, with enhanced reliability features including health checking and auto-recovery.

## Features Over Standard Readers

### Enhanced Reliability

| Feature | Description |
|---------|-------------|
| **Health check with auto-reset** | Periodically validates communication and reinitializes if needed |
| **Exponential backoff** on bus errors | 5s → 10s → 60s intervals to prevent log flooding |
| **UID flap prevention** | Same UID detected → stay present without triggering RF cycle |
| **Setup retry** | 3-attempt retry on initialization at boot |
| **IRQ support** | Optional interrupt-driven operation for better responsiveness |

### Protocol Support

* **ISO/IEC 14443A** - MIFARE Classic, NTAG, DESFire
* **ISO/IEC 14443B** - Type B cards
* **FeliCa** - Japanese contactless cards
* **ISO/IEC 15693** - Long-range RFID tags (up to 9cm)
* **NFC Forum Type 1/2/3/4** - All major NFC tag types

---

## Installation

Add to your ESPHome YAML:

```yaml
external_components:
  - source: github://JohnMcLear/esphome_clrc663
    components: [clrc663, clrc663_spi, clrc663_i2c]
    refresh: 1d
```

---

## Over I²C

```yaml
i2c:
  sda: GPIO21
  scl: GPIO22
  frequency: 400kHz  # CLRC663 supports up to 400kHz (fast mode) or 1MHz (fast mode+)

clrc663_i2c:
  update_interval: 1s
  on_tag:
    then:
      - logger.log:
          format: "Tag: %s"
          args: ['x.c_str()']
  on_tag_removed:
    then:
      - logger.log:
          format: "Tag removed: %s"
          args: ['x.c_str()']

binary_sensor:
  - platform: clrc663
    name: "My NFC Tag"
    uid: "74-10-37-94"
```

### I²C Configuration Variables

* **`address`** (*Optional*, default `0x28`): I2C address of the CLRC663 (configurable via EEPROM: 0x28-0x2B).
* **`update_interval`** (*Optional*, default `1s`): How often to scan for tags.
* **`on_tag`** (*Optional*): Automation triggered when a tag is detected. Variable `x` is the UID string.
* **`on_tag_removed`** (*Optional*): Automation triggered when a tag is removed.
* **`health_check_enabled`** (*Optional*, default `true`): Enable periodic health checks.
* **`health_check_interval`** (*Optional*, default `60s`): How often to run the health check.
* **`max_failed_checks`** (*Optional*, default `3`): Failures before marking component as unhealthy.
* **`auto_reset_on_failure`** (*Optional*, default `true`): Attempt re-initialization when unhealthy.
* **`irq_pin`** (*Optional*): IRQ (interrupt) pin for interrupt-driven operation.
* **`reset_pin`** (*Optional*): Hardware reset pin for hard resets.
* **`i2c_id`** (*Optional*): Manually specify the I2C bus ID.
* **`id`** (*Optional*): Component ID.

---

## Over SPI

```yaml
spi:
  clk_pin: GPIO18
  miso_pin: GPIO19
  mosi_pin: GPIO23

clrc663_spi:
  cs_pin: GPIO5
  update_interval: 1s
  on_tag:
    then:
      - homeassistant.tag_scanned: !lambda 'return x;'

binary_sensor:
  - platform: clrc663
    name: "My NFC Tag"
    uid: "74-10-37-94"
```

### SPI Configuration Variables

All the same options as I2C above, plus:

* **`cs_pin`** (**Required**): Chip select pin.
* **`spi_id`** (*Optional*): Manually specify the SPI bus ID.

---

## `clrc663` Binary Sensor

```yaml
binary_sensor:
  - platform: clrc663
    clrc663_id: clrc663_board   # required if multiple CLRC663 instances
    name: "My Tag"
    uid: "74-10-37-94"      # hyphen or colon separated hex
```

### Binary Sensor Configuration Variables

* **`uid`** (**Required**): The UID to match. Hyphen-separated hex: `74-10-37-94`. Colon-separated also accepted: `74:10:37:94`.
* **`clrc663_id`** (*Optional*): The ID of the `clrc663_spi` or `clrc663_i2c` hub. Required when you have more than one.
* All other options from [Binary Sensor](https://esphome.io/components/binary_sensor/).

---

## Setting Up Tags

To discover a tag's UID, configure without any binary sensors first:

```yaml
clrc663_i2c:
  update_interval: 1s
  on_tag:
    then:
      - logger.log:
          format: "Found tag: %s"
          args: ['x.c_str()']
```

Flash this, open the logs, and scan your tag. You'll see:

```
Found new tag '74-10-37-94'
```

Copy that UID into a `binary_sensor` entry.

---

## Health Check

The health check periodically issues commands to verify the CLRC663 is still responding. If it fails `max_failed_checks` times consecutively:

1. The component is marked as errored in ESPHome.
2. If `auto_reset_on_failure: true`, a re-initialization is attempted automatically (via reset pin if available, otherwise soft reset).
3. On successful recovery, normal operation resumes.

This resolves long-running freeze issues common in NFC readers.

---

## Comparison with Other NXP Readers

| Feature | PN532 | PN7160 | **CLRC663** |
|---------|-------|--------|-------------|
| **Protocols** | ISO14443A/B, FeliCa | ISO14443A/B, FeliCa, ISO15693 | ISO14443A/B, FeliCa, ISO15693 |
| **Max Read Range** | ~5cm | ~7cm | **~9cm** |
| **ISO15693 Support** | ❌ | ✅ | ✅ |
| **Interface** | I2C, SPI | I2C, SPI | I2C, SPI, UART |
| **I2C Frequency** | 400kHz max | 100kHz min (critical) | 400kHz / 1MHz |
| **Power** | 3.3V | 3.3V | **3.3-5V** |
| **SAM Support** | ❌ | ❌ | ✅ (dedicated I2C) |
| **LPCD (Low Power)** | ❌ | ❌ | ✅ |
| **Price Point** | $ | $$ | $$ |
| **Best For** | Budget projects | Modern NFC apps | Industrial, long-range |

**CLRC663 Advantages:**
- Extended read range (up to 9cm with proper antenna)
- Native ISO15693 support for industrial asset tracking
- Dedicated SAM interface for high-security applications
- Low Power Card Detection (LPCD) for battery-powered devices
- Dual-voltage support (3.3V/5V)

---

## Advanced Examples

### With Home Assistant Tag Integration

```yaml
clrc663_i2c:
  on_tag:
    then:
      - homeassistant.tag_scanned: !lambda 'return x;'
```

### Multiple Tags

```yaml
binary_sensor:
  - platform: clrc663
    name: "Employee Badge"
    uid: "74-10-37-94"
    on_press:
      then:
        - logger.log: "Employee checked in"
  
  - platform: clrc663
    name: "Guest Badge"
    uid: "AB-CD-EF-12"
    on_press:
      then:
        - logger.log: "Guest checked in"
```

### With Custom IRQ Pin

```yaml
clrc663_i2c:
  irq_pin: GPIO18  # Connect to CLRC663 IRQ output
  on_tag:
    then:
      - logger.log: "Tag detected via IRQ"
```

---

## Hardware Requirements

### Pinout

#### I2C Mode
| CLRC663 Pin | ESP32 | ESP8266 |
|-------------|-------|---------|
| VCC | 3.3V | 3.3V |
| GND | GND | GND |
| SDA | GPIO21 | D2 (GPIO4) |
| SCL | GPIO22 | D1 (GPIO5) |
| IRQ (optional) | Any GPIO | Any GPIO |
| RST (optional) | Any GPIO | Any GPIO |

#### SPI Mode
| CLRC663 Pin | ESP32 | ESP8266 |
|-------------|-------|---------|
| VCC | 3.3V | 3.3V |
| GND | GND | GND |
| SCK | GPIO18 | D5 (GPIO14) |
| MISO | GPIO19 | D6 (GPIO12) |
| MOSI | GPIO23 | D7 (GPIO13) |
| NSS/CS | GPIO5 | D8 (GPIO15) |
| IRQ (optional) | Any GPIO | Any GPIO |
| RST (optional) | Any GPIO | Any GPIO |

### Antenna Considerations

The CLRC663 requires a properly tuned antenna for optimal performance:
- **Standard**: 65mm x 65mm PCB antenna (included on most modules)
- **Extended Range**: 4-turn PCB antenna with 5V RF driver
- **Custom**: Consult NXP AN11022 for antenna tuning guidance

---

## Troubleshooting

### No Tags Detected
1. Verify I2C/SPI wiring
2. Check I2C frequency (use 400kHz for best results)
3. Verify antenna is properly connected
4. Check power supply (5V recommended for best RF performance)
5. Enable debug logging: `logger: level: DEBUG`

### Component Unhealthy
- Check `health_check_interval` and `max_failed_checks` settings
- Verify reset pin is connected (if configured)
- Check power supply stability
- Enable `auto_reset_on_failure`

### Intermittent Reads
- Increase `update_interval` to 2s or higher
- Check for WiFi interference (less likely with CLRC663)
- Verify antenna tuning
- Check IRQ pin connection (if used)

---

## Compatibility

* ESPHome 2024.x and later
* ESP32 (Arduino & ESP-IDF frameworks)
* ESP8266 (Arduino framework)
* CLRC663 modules over SPI or I2C

---

## License

Apache-2.0 (same as ESPHome)

## Credits

Based on the ESPHome component architecture and inspired by:
- [esphome_pn532](https://github.com/JohnMcLear/esphome_pn532)
- [esphome_pn7160](https://github.com/JohnMcLear/esphome_pn7160)
- NXP CLRC663 Reader Library
