#include "clrc663_spi.h"
#include "esphome/core/log.h"

namespace esphome {
namespace clrc663 {

static const char *const TAG = "clrc663.spi";

void CLRC663SPI::setup() {
  this->spi_setup();
  CLRC663::setup();
}

void CLRC663SPI::dump_config() {
  CLRC663::dump_config();
  LOG_PIN("  CS Pin: ", this->cs_);
  if (this->irq_pin_ != nullptr) {
    LOG_PIN("  IRQ Pin: ", this->irq_pin_);
  }
  if (this->reset_pin_ != nullptr) {
    LOG_PIN("  Reset Pin: ", this->reset_pin_);
  }
}

bool CLRC663SPI::read_data(std::vector<uint8_t> &data, uint8_t len) {
  this->enable();
  // SPI read protocol for CLRC663:
  // First byte: register address with read bit (0x80 | address)
  // Following bytes: data
  this->write_byte(0x80 | data[0]);  // Assume first byte is register address
  for (uint8_t i = 0; i < len; i++) {
    data[i] = this->read_byte();
  }
  this->disable();
  return true;
}

bool CLRC663SPI::write_data(const std::vector<uint8_t> &data) {
  this->enable();
  // SPI write protocol for CLRC663:
  // First byte: register address (0x00-0x7F for write)
  // Following bytes: data to write
  for (auto byte : data) {
    this->write_byte(byte);
  }
  this->disable();
  return true;
}

bool CLRC663SPI::reset_() {
  if (this->reset_pin_ != nullptr) {
    ESP_LOGV(TAG, "Performing hardware reset via reset pin");
    this->reset_pin_->digital_write(false);
    delay(10);
    this->reset_pin_->digital_write(true);
    delay(50);
  } else {
    ESP_LOGV(TAG, "Performing soft reset via command");
    std::vector<uint8_t> cmd = {0x00, 0x1F};  // Command register, SOFTRESET
    if (!this->write_data(cmd)) {
      return false;
    }
    delay(50);
  }
  
  return true;
}

}  // namespace clrc663
}  // namespace esphome
