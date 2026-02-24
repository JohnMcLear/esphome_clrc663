#include "clrc663_i2c.h"
#include "esphome/core/log.h"

namespace esphome {
namespace clrc663 {

static const char *const TAG = "clrc663.i2c";

void CLRC663I2C::dump_config() {
  CLRC663::dump_config();
  LOG_I2C_DEVICE(this);
  if (this->irq_pin_ != nullptr) {
    LOG_PIN("  IRQ Pin: ", this->irq_pin_);
  }
  if (this->reset_pin_ != nullptr) {
    LOG_PIN("  Reset Pin: ", this->reset_pin_);
  }
}

bool CLRC663I2C::read_data(std::vector<uint8_t> &data, uint8_t len) {
  if (this->read(data.data(), len) != i2c::ERROR_OK) {
    ESP_LOGV(TAG, "I2C read failed");
    return false;
  }
  return true;
}

bool CLRC663I2C::write_data(const std::vector<uint8_t> &data) {
  if (this->write(data.data(), data.size()) != i2c::ERROR_OK) {
    ESP_LOGV(TAG, "I2C write failed");
    return false;
  }
  return true;
}

bool CLRC663I2C::reset_() {
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
