#include "clrc663.h"
#include "esphome/core/log.h"

namespace esphome {
namespace clrc663 {

static const char *const TAG = "clrc663";

// CLRC663 Commands
static const uint8_t CLRC663_CMD_IDLE = 0x00;
static const uint8_t CLRC663_CMD_LPCD = 0x01;
static const uint8_t CLRC663_CMD_LOADKEY = 0x02;
static const uint8_t CLRC663_CMD_MFAUTHENT = 0x03;
static const uint8_t CLRC663_CMD_ACKREQ = 0x04;
static const uint8_t CLRC663_CMD_RECEIVE = 0x05;
static const uint8_t CLRC663_CMD_TRANSMIT = 0x06;
static const uint8_t CLRC663_CMD_TRANSCEIVE = 0x07;
static const uint8_t CLRC663_CMD_WRITEE2 = 0x08;
static const uint8_t CLRC663_CMD_WRITEE2PAGE = 0x09;
static const uint8_t CLRC663_CMD_READE2 = 0x0A;
static const uint8_t CLRC663_CMD_LOADREG = 0x0C;
static const uint8_t CLRC663_CMD_LOADPROTOCOL = 0x0D;
static const uint8_t CLRC663_CMD_LOADKEYE2 = 0x0E;
static const uint8_t CLRC663_CMD_STOREKEYE2 = 0x0F;
static const uint8_t CLRC663_CMD_SOFTRESET = 0x1F;

// CLRC663 Registers
static const uint8_t CLRC663_REG_COMMAND = 0x00;
static const uint8_t CLRC663_REG_HOST_CTRL = 0x01;
static const uint8_t CLRC663_REG_FIFO_CONTROL = 0x02;
static const uint8_t CLRC663_REG_WATER_LEVEL = 0x03;
static const uint8_t CLRC663_REG_FIFO_LENGTH = 0x04;
static const uint8_t CLRC663_REG_FIFO_DATA = 0x05;
static const uint8_t CLRC663_REG_IRQ0 = 0x06;
static const uint8_t CLRC663_REG_IRQ1 = 0x07;
static const uint8_t CLRC663_REG_IRQ0EN = 0x08;
static const uint8_t CLRC663_REG_IRQ1EN = 0x09;
static const uint8_t CLRC663_REG_ERROR = 0x0A;
static const uint8_t CLRC663_REG_STATUS = 0x0B;
static const uint8_t CLRC663_REG_RX_BIT_CTRL = 0x0C;
static const uint8_t CLRC663_REG_RX_COLL = 0x0D;
static const uint8_t CLRC663_REG_VERSION = 0x7F;

// ISO14443A Commands
static const uint8_t ISO14443A_CMD_REQA = 0x26;
static const uint8_t ISO14443A_CMD_WUPA = 0x52;
static const uint8_t ISO14443A_CMD_SELECT_CL1 = 0x93;
static const uint8_t ISO14443A_CMD_SELECT_CL2 = 0x95;
static const uint8_t ISO14443A_CMD_SELECT_CL3 = 0x97;

void CLRC663::setup() {
  ESP_LOGCONFIG(TAG, "Setting up CLRC663...");
  
  // Attempt to reset and initialize
  for (int attempts = 0; attempts < 3; attempts++) {
    if (this->reset_()) {
      delay(50);
      
      // Try to get firmware version
      std::vector<uint8_t> version;
      if (this->get_firmware_version_(version)) {
        if (version.size() >= 1) {
          ESP_LOGI(TAG, "Found CLRC663 version: 0x%02X", version[0]);
          this->mark_failed();
          return;
        }
      }
    }
    
    if (attempts < 2) {
      ESP_LOGW(TAG, "Initialization attempt %d failed, retrying...", attempts + 1);
      delay(100);
    }
  }
  
  ESP_LOGE(TAG, "Failed to initialize CLRC663 after 3 attempts");
  this->mark_failed();
  this->error_code_ = WAKEUP_FAILED;
}

void CLRC663::update() {
  if (!this->component_healthy_) {
    ESP_LOGW(TAG, "Component is unhealthy, skipping update");
    return;
  }
  
  // Exponential backoff for read errors
  if (millis() < this->next_read_after_) {
    return;
  }
  
  std::vector<uint8_t> uid;
  if (this->read_passive_target_id_(uid)) {
    if (uid.size() == 0) {
      // No tag present
      if (this->current_uid_.size() > 0) {
        // Tag was removed
        std::string uid_str = format_hex_pretty(this->current_uid_);
        ESP_LOGD(TAG, "Tag removed: %s", uid_str.c_str());
        this->on_tag_removed_callbacks_.call(uid_str);
        for (auto *sensor : this->binary_sensors_) {
          sensor->on_scan_end();
        }
        this->current_uid_.clear();
      }
    } else {
      // Tag present
      if (uid != this->current_uid_) {
        // New or different tag
        this->current_uid_ = uid;
        std::string uid_str = format_hex_pretty(uid);
        ESP_LOGI(TAG, "Found new tag: %s", uid_str.c_str());
        
        this->on_tag_callbacks_.call(uid_str);
        
        for (auto *sensor : this->binary_sensors_) {
          sensor->process(uid);
        }
      }
    }
    
    // Reset backoff on successful read
    this->read_backoff_ = 0;
  } else {
    // Read failed - apply exponential backoff
    if (this->read_backoff_ == 0) {
      this->read_backoff_ = 5000;  // Start with 5s
    } else if (this->read_backoff_ < 60000) {
      this->read_backoff_ = this->read_backoff_ * 2;  // Double up to 60s max
    }
    this->next_read_after_ = millis() + this->read_backoff_;
    ESP_LOGW(TAG, "Read failed, backing off for %d ms", this->read_backoff_);
  }
}

void CLRC663::loop() {
  // Perform periodic health checks
  if (this->health_check_enabled_ && 
      (millis() - this->last_health_check_ > this->health_check_interval_)) {
    this->perform_health_check_();
    this->last_health_check_ = millis();
  }
}

void CLRC663::dump_config() {
  ESP_LOGCONFIG(TAG, "CLRC663:");
  if (this->is_failed()) {
    ESP_LOGCONFIG(TAG, "  Setup Failed!");
    return;
  }
  ESP_LOGCONFIG(TAG, "  Health Check: %s", this->health_check_enabled_ ? "enabled" : "disabled");
  if (this->health_check_enabled_) {
    ESP_LOGCONFIG(TAG, "  Health Check Interval: %d ms", this->health_check_interval_);
    ESP_LOGCONFIG(TAG, "  Max Failed Checks: %d", this->max_failed_checks_);
    ESP_LOGCONFIG(TAG, "  Auto Reset: %s", this->auto_reset_on_failure_ ? "enabled" : "disabled");
  }
  LOG_UPDATE_INTERVAL(this);
  
  for (auto *sensor : this->binary_sensors_) {
    LOG_BINARY_SENSOR("  ", "Tag", sensor);
  }
}

bool CLRC663::get_firmware_version_(std::vector<uint8_t> &response) {
  // Read version register
  std::vector<uint8_t> cmd = {CLRC663_REG_VERSION};
  if (!this->write_data(cmd)) {
    return false;
  }
  
  response.resize(1);
  return this->read_data(response, 1);
}

bool CLRC663::read_passive_target_id_(std::vector<uint8_t> &uid) {
  // Load ISO14443A protocol from EEPROM
  std::vector<uint8_t> cmd = {CLRC663_REG_COMMAND, CLRC663_CMD_LOADPROTOCOL, 0x00};
  if (!this->write_data(cmd)) {
    return false;
  }
  
  delay(5);
  
  // Send REQA (request type A)
  cmd = {CLRC663_REG_FIFO_DATA, ISO14443A_CMD_REQA};
  if (!this->write_data(cmd)) {
    return false;
  }
  
  cmd = {CLRC663_REG_COMMAND, CLRC663_CMD_TRANSCEIVE};
  if (!this->write_data(cmd)) {
    return false;
  }
  
  delay(10);
  
  // Check if card responded (ATQA should be 2 bytes)
  std::vector<uint8_t> fifo_len_reg = {CLRC663_REG_FIFO_LENGTH};
  if (!this->write_data(fifo_len_reg)) {
    return false;
  }
  
  std::vector<uint8_t> fifo_len;
  fifo_len.resize(1);
  if (!this->read_data(fifo_len, 1)) {
    return false;
  }
  
  if (fifo_len[0] < 2) {
    // No card present
    uid.clear();
    return true;
  }
  
  // Anti-collision cascade level 1
  cmd = {CLRC663_REG_FIFO_DATA, ISO14443A_CMD_SELECT_CL1, 0x20};
  if (!this->write_data(cmd)) {
    return false;
  }
  
  cmd = {CLRC663_REG_COMMAND, CLRC663_CMD_TRANSCEIVE};
  if (!this->write_data(cmd)) {
    return false;
  }
  
  delay(10);
  
  // Read UID from FIFO
  cmd = {CLRC663_REG_FIFO_LENGTH};
  if (!this->write_data(cmd)) {
    return false;
  }
  
  if (!this->read_data(fifo_len, 1)) {
    return false;
  }
  
  if (fifo_len[0] < 5) {
    uid.clear();
    return true;
  }
  
  cmd = {CLRC663_REG_FIFO_DATA};
  if (!this->write_data(cmd)) {
    return false;
  }
  
  uid.resize(fifo_len[0]);
  if (!this->read_data(uid, fifo_len[0])) {
    return false;
  }
  
  // UID is first 4 bytes (5th byte is BCC checksum)
  if (uid.size() >= 5) {
    uid.resize(4);
  }
  
  return true;
}

void CLRC663::perform_health_check_() {
  if (!this->verify_communication_()) {
    this->failed_health_checks_++;
    ESP_LOGW(TAG, "Health check failed (%d/%d)", this->failed_health_checks_, this->max_failed_checks_);
    
    if (this->failed_health_checks_ >= this->max_failed_checks_) {
      ESP_LOGE(TAG, "Max health check failures reached, component unhealthy");
      this->component_healthy_ = false;
      this->handle_health_failure_();
    }
  } else {
    if (this->failed_health_checks_ > 0) {
      ESP_LOGI(TAG, "Health check passed, resetting failure count");
    }
    this->failed_health_checks_ = 0;
    this->component_healthy_ = true;
  }
}

bool CLRC663::verify_communication_() {
  std::vector<uint8_t> version;
  if (!this->get_firmware_version_(version)) {
    return false;
  }
  return version.size() > 0;
}

void CLRC663::handle_health_failure_() {
  if (this->auto_reset_on_failure_) {
    ESP_LOGI(TAG, "Attempting auto-reset...");
    if (this->reset_()) {
      delay(50);
      if (this->verify_communication_()) {
        ESP_LOGI(TAG, "Auto-reset successful, component recovered");
        this->component_healthy_ = true;
        this->failed_health_checks_ = 0;
        return;
      }
    }
    ESP_LOGE(TAG, "Auto-reset failed");
  }
}

}  // namespace clrc663
}  // namespace esphome
