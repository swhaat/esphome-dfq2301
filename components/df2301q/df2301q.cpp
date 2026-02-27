/**
 * ESPHome component for the DFRobot DF2301Q offline voice recognition module.
 *
 * Datasheet / protocol reference:
 *   https://wiki.dfrobot.com/SKU_SEN0539-EN_Offline_Voice_Recognition_Sensor_I2C_UART
 *
 * DFRobot Arduino library (MIT):
 *   https://github.com/DFRobot/DFRobot_DF2301Q
 */

#include "df2301q.h"
#include "esphome/core/log.h"
#include <cstring>

namespace esphome {
namespace df2301q {

static const char *const TAG = "df2301q";

// ── DF2301QBase ───────────────────────────────────────────────────────────────

/**
 * Map a command word ID to its human-readable English text.
 *
 * Source: DFRobot DF2301Q wiki
 *   https://wiki.dfrobot.com/SKU_SEN0539-EN_Offline_Voice_Recognition_Sensor_I2C_UART
 *
 * IDs 0–2  : silence / wake-up words
 * IDs 5–21 : user-learned custom commands
 * IDs 22+  : built-in fixed command words
 */
std::string DF2301QBase::cmd_id_to_text(uint8_t cmd_id) {
  switch (cmd_id) {
    case 0:   return "Silence";
    case 1:   return "Wake up words for learning";
    case 2:   return "Hello, Robot";
    case 5:   return "Custom Command 1";
    case 6:   return "Custom Command 2";
    case 7:   return "Custom Command 3";
    case 8:   return "Custom Command 4";
    case 9:   return "Custom Command 5";
    case 10:  return "Custom Command 6";
    case 11:  return "Custom Command 7";
    case 12:  return "Custom Command 8";
    case 13:  return "Custom Command 9";
    case 14:  return "Custom Command 10";
    case 15:  return "Custom Command 11";
    case 16:  return "Custom Command 12";
    case 17:  return "Custom Command 13";
    case 18:  return "Custom Command 14";
    case 19:  return "Custom Command 15";
    case 20:  return "Custom Command 16";
    case 21:  return "Custom Command 17";
    case 22:  return "Go forward";
    case 23:  return "Retreat";
    case 24:  return "Park a car";
    case 25:  return "Turn left ninety degrees";
    case 26:  return "Turn left forty-five degrees";
    case 27:  return "Turn left thirty degrees";
    case 29:  return "Turn right forty-five degrees";
    case 30:  return "Turn right thirty degrees";
    case 31:  return "Shift down a gear";
    case 32:  return "Line tracking mode";
    case 33:  return "Light tracking mode";
    case 34:  return "Bluetooth mode";
    case 35:  return "Obstacle avoidance mode";
    case 36:  return "Face recognition";
    case 37:  return "Object tracking";
    case 38:  return "Object recognition";
    case 39:  return "Line tracking";
    case 40:  return "Color recognition";
    case 41:  return "Tag recognition";
    case 42:  return "Object sorting";
    case 43:  return "QR code recognition";
    case 44:  return "General settings";
    case 45:  return "Clear screen";
    case 46:  return "Learn once";
    case 47:  return "Forget";
    case 48:  return "Load model";
    case 49:  return "Save model";
    case 50:  return "Take photos and save them";
    case 51:  return "Save and return";
    case 52:  return "Display number zero";
    case 53:  return "Display number one";
    case 54:  return "Display number two";
    case 55:  return "Display number three";
    case 56:  return "Display number four";
    case 57:  return "Display number five";
    case 58:  return "Display number six";
    case 59:  return "Display number seven";
    case 60:  return "Display number eight";
    case 61:  return "Display number nine";
    case 62:  return "Display smiley face";
    case 63:  return "Display crying face";
    case 64:  return "Display heart";
    case 65:  return "Turn off dot matrix";
    case 66:  return "Read current posture";
    case 67:  return "Read ambient light";
    case 68:  return "Read compass";
    case 69:  return "Read temperature";
    case 70:  return "Read acceleration";
    case 71:  return "Reading sound intensity";
    case 72:  return "Calibrate electronic gyroscope";
    case 73:  return "Turn on the camera";
    case 74:  return "Turn off the camera";
    case 75:  return "Turn on the fan";
    case 76:  return "Turn off the fan";
    case 77:  return "Turn fan speed to gear one";
    case 78:  return "Turn fan speed to gear two";
    case 79:  return "Turn fan speed to gear three";
    case 80:  return "Start oscillating";
    case 81:  return "Stop oscillating";
    case 82:  return "Reset";
    case 83:  return "Set servo to ten degrees";
    case 84:  return "Set servo to thirty degrees";
    case 85:  return "Set servo to forty-five degrees";
    case 86:  return "Set servo to sixty degrees";
    case 87:  return "Set servo to ninety degrees";
    case 88:  return "Turn on the buzzer";
    case 89:  return "Turn off the buzzer";
    case 90:  return "Turn on the speaker";
    case 91:  return "Turn off the speaker";
    case 92:  return "Play music";
    case 93:  return "Stop playing";
    case 94:  return "The last track";
    case 95:  return "The next track";
    case 96:  return "Repeat this track";
    case 97:  return "Volume up";
    case 98:  return "Volume down";
    case 99:  return "Change volume to maximum";
    case 100: return "Change volume to minimum";
    case 101: return "Change volume to medium";
    case 102: return "Play poem";
    case 103: return "Turn on the light";
    case 104: return "Turn off the light";
    case 105: return "Brighten the light";
    case 106: return "Dim the light";
    case 107: return "Adjust brightness to maximum";
    case 108: return "Adjust brightness to minimum";
    case 109: return "Increase color temperature";
    case 110: return "Decrease color temperature";
    case 111: return "Adjust color temperature to maximum";
    case 112: return "Adjust color temperature to minimum";
    case 113: return "Daylight mode";
    case 114: return "Moonlight mode";
    case 115: return "Color mode";
    case 116: return "Set to red";
    case 117: return "Set to orange";
    case 118: return "Set to yellow";
    case 119: return "Set to green";
    case 120: return "Set to cyan";
    case 121: return "Set to blue";
    case 122: return "Set to purple";
    case 123: return "Set to white";
    case 124: return "Turn on AC";
    case 125: return "Turn off AC";
    case 126: return "Increase temperature";
    case 127: return "Decrease temperature";
    case 128: return "Cool mode";
    case 129: return "Heat mode";
    case 130: return "Auto mode";
    case 131: return "Dry mode";
    case 132: return "Fan mode";
    case 133: return "Enable blowing up-down";
    case 134: return "Disable blowing up-down";
    case 135: return "Enable blowing right-left";
    case 136: return "Disable blowing right-left";
    case 137: return "Open the window";
    case 138: return "Close the window";
    case 139: return "Open curtain";
    case 140: return "Close curtain";
    case 141: return "Open the door";
    case 142: return "Close the door";
    case 200: return "Learning wake word";
    case 201: return "Learning command word";
    case 202: return "Re-learn";
    case 203: return "Exit learning";
    case 204: return "I want to delete";
    case 205: return "Delete wake word";
    case 206: return "Delete command word";
    case 207: return "Exit deleting";
    case 208: return "Delete all";
    default:  return "Unknown command " + std::to_string(cmd_id);
  }
}

void DF2301QBase::fire_trigger_(uint8_t cmd_id) {
  std::string text = cmd_id_to_text(cmd_id);
  for (auto *t : this->triggers_)
    t->process(text);
}

// ── DF2301QI2CComponent – I²C implementation ──────────────────────────────────

void DF2301QI2CComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DF2301Q (I²C)...");

  // Probe the device – write 0x00 to confirm it ACKs.
  auto err = this->write_register(0x00, nullptr, 0);
  if (err != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "DF2301Q not found on I²C bus (addr 0x%02X)", this->address_);
    this->mark_failed();
    return;
  }
  ESP_LOGCONFIG(TAG, "DF2301Q found on I²C bus.");
}

void DF2301QI2CComponent::update() {
  uint8_t cmd_id = 0;
  if (!this->read_register_(DF2301Q_REG_CMDID, cmd_id)) {
    ESP_LOGW(TAG, "I²C read failed");
    return;
  }
  if (cmd_id != 0) {
    ESP_LOGD(TAG, "Command ID: %u", cmd_id);
    this->fire_trigger_(cmd_id);
  }
}

void DF2301QI2CComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DF2301Q (I²C):");
  LOG_I2C_DEVICE(this);
  LOG_UPDATE_INTERVAL(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "  Device failed to initialise!");
  }
}

void DF2301QI2CComponent::play_command(uint8_t cmd_id) {
  this->write_register_(DF2301Q_REG_PLAY_CMDID, cmd_id);
  // Allow the module time to finish playing before the next I²C access.
  delay(1000);  // NOLINT
}

void DF2301QI2CComponent::set_volume(uint8_t vol) {
  if (vol < 1) vol = 1;
  if (vol > 7) vol = 7;
  this->write_register_(DF2301Q_REG_VOLUME, vol);
}

void DF2301QI2CComponent::set_mute(bool mute) {
  this->write_register_(DF2301Q_REG_MUTE, mute ? 1 : 0);
}

void DF2301QI2CComponent::set_wake_time(uint8_t secs) {
  this->write_register_(DF2301Q_REG_WAKE_TIME, secs);
}

// ── I²C helpers ───────────────────────────────────────────────────────────────

void DF2301QI2CComponent::write_register_(uint8_t reg, uint8_t value) {
  uint8_t data[1] = {value};
  auto err = this->write_register(reg, data, 1);
  if (err != i2c::ERROR_OK) {
    ESP_LOGW(TAG, "I²C write to reg 0x%02X failed", reg);
  }
}

bool DF2301QI2CComponent::read_register_(uint8_t reg, uint8_t &value) {
  // Send register address then read 1 byte.
  auto err = this->write_register(reg, nullptr, 0, false);
  if (err != i2c::ERROR_OK)
    return false;
  uint8_t buf[1];
  err = this->read_register(reg, buf, 1);
  if (err != i2c::ERROR_OK)
    return false;
  value = buf[0];
  return true;
}

// ── DF2301QUARTComponent – UART implementation ────────────────────────────────

void DF2301QUARTComponent::setup() {
  ESP_LOGCONFIG(TAG, "Setting up DF2301Q (UART)...");
  this->recv_state_ = RecvState::HEAD0;
  this->send_seq_   = 0;
}

void DF2301QUARTComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "DF2301Q (UART):");
  this->check_uart_settings(9600);
}

/**
 * Parse incoming bytes using a state machine that mirrors the DFRobot library.
 *
 * Frame format (all fields little-endian):
 *   [0xF4][0xF5] [len_lo][len_hi] [type][cmd][seq] [data×len]
 *   [cksum_lo][cksum_hi] [0xFB]
 *
 * where len  = number of payload bytes (does NOT count type/cmd/seq)
 * and   cksum = sum of (type + cmd + seq + data[0..len-1])
 */
void DF2301QUARTComponent::loop() {
  while (this->available()) {
    uint8_t c;
    this->read_byte(&c);

    switch (this->recv_state_) {
      case HEAD0:
        if (c == DF2301Q_UART_HEAD_LOW)
          this->recv_state_ = HEAD1;
        break;

      case HEAD1:
        if (c == DF2301Q_UART_HEAD_HIGH) {
          this->recv_msg_.header = DF2301Q_UART_HEAD;
          this->recv_state_      = LEN0;
        } else if (c != DF2301Q_UART_HEAD_LOW) {
          this->recv_state_ = HEAD0;
        }
        break;

      case LEN0:
        this->recv_data_len_ = c;
        this->recv_state_    = LEN1;
        break;

      case LEN1:
        this->recv_data_len_ |= static_cast<uint16_t>(c) << 8;
        if (this->recv_data_len_ <= DF2301Q_UART_MAX_DATA) {
          this->recv_msg_.data_length = this->recv_data_len_;
          this->recv_state_           = TYPE;
        } else {
          ESP_LOGW(TAG, "UART: frame data_length %u out of range, resync",
                   this->recv_data_len_);
          this->recv_state_ = HEAD0;
        }
        break;

      case TYPE:
        this->recv_msg_.msg_type = c;
        this->recv_cksum_        = c;  // start checksum accumulation
        this->recv_state_        = CMD;
        break;

      case CMD:
        this->recv_msg_.msg_cmd = c;
        this->recv_cksum_      += c;
        this->recv_state_       = SEQ;
        break;

      case SEQ:
        this->recv_msg_.msg_seq = c;
        this->recv_cksum_      += c;
        this->recv_data_count_  = 0;
        this->recv_state_       = (this->recv_data_len_ > 0) ? DATA : CKSUM0;
        break;

      case DATA:
        this->recv_msg_.msg_data[this->recv_data_count_++] = c;
        this->recv_cksum_ += c;
        if (this->recv_data_count_ == this->recv_data_len_)
          this->recv_state_ = CKSUM0;
        break;

      case CKSUM0:
        this->recv_cksum_lo_ = c;
        this->recv_state_    = CKSUM1;
        break;

      case CKSUM1: {
        uint16_t received = static_cast<uint16_t>(c) << 8 | this->recv_cksum_lo_;
        if (received == this->recv_cksum_) {
          this->recv_state_ = TAIL;
        } else {
          ESP_LOGW(TAG, "UART: checksum mismatch (got 0x%04X, expected 0x%04X)",
                   received, this->recv_cksum_);
          this->recv_state_ = HEAD0;
        }
        break;
      }

      case TAIL:
        this->recv_state_ = HEAD0;
        if (c == DF2301Q_UART_TAIL) {
          // Valid frame received – handle it.
          if (this->recv_msg_.msg_type == DF2301Q_TYPE_CMD_UP &&
              this->recv_msg_.msg_cmd  == DF2301Q_CMD_ASR_RESULT) {
            uint8_t cmd_id = this->recv_msg_.msg_data[0];
            ESP_LOGD(TAG, "UART: command ID %u", cmd_id);
            this->fire_trigger_(cmd_id);
          }
        } else {
          ESP_LOGW(TAG, "UART: bad tail byte 0x%02X", c);
        }
        break;

      default:
        this->recv_state_ = HEAD0;
        break;
    }
  }
}

// ── UART action helpers ───────────────────────────────────────────────────────

void DF2301QUARTComponent::play_command(uint8_t cmd_id) {
  UartMsg msg{};
  msg.msg_type   = DF2301Q_TYPE_CMD_DOWN;
  msg.msg_cmd    = DF2301Q_CMD_PLAY_VOICE;
  msg.msg_data[0] = DF2301Q_PLAY_START;
  msg.msg_data[1] = DF2301Q_PLAY_BY_CMD_ID;
  uint32_t id32 = cmd_id;
  std::memcpy(&msg.msg_data[2], &id32, 4);
  // data_length = play_start(1) + play_by_cmd_id(1) + cmd_id(4) = 6
  this->send_packet_(msg, 6);
}

void DF2301QUARTComponent::set_volume(uint8_t vol) {
  if (vol < 1) vol = 1;
  if (vol > 7) vol = 7;
  this->send_setting_(DF2301Q_CFG_VOLUME, vol);
}

void DF2301QUARTComponent::set_mute(bool mute) {
  this->send_setting_(DF2301Q_CFG_MUTE, mute ? 1 : 0);
}

void DF2301QUARTComponent::set_wake_time(uint8_t secs) {
  this->send_setting_(DF2301Q_CFG_WAKE_TIME, secs);
}

void DF2301QUARTComponent::reset_module() {
  UartMsg msg{};
  msg.msg_type    = DF2301Q_TYPE_CMD_DOWN;
  msg.msg_cmd     = DF2301Q_CMD_RESET_MODULE;
  // Payload is the ASCII string "reset"
  msg.msg_data[0] = 'r';
  msg.msg_data[1] = 'e';
  msg.msg_data[2] = 's';
  msg.msg_data[3] = 'e';
  msg.msg_data[4] = 't';
  this->send_packet_(msg, 5);
}

// ── Low-level UART send ───────────────────────────────────────────────────────

/**
 * Build and transmit one framed UART packet.
 *
 * @param msg         Message struct (type/cmd/seq/data already populated)
 * @param payload_len Number of bytes in msg.msg_data to include
 *
 * Frame emitted:
 *   HEAD_LO HEAD_HI  len_lo len_hi
 *   type cmd seq  data[0..payload_len-1]
 *   cksum_lo cksum_hi  TAIL
 */
void DF2301QUARTComponent::send_packet_(const UartMsg &msg, uint8_t payload_len) {
  // Flush any stale RX bytes before sending.
  while (this->available())
    this->read();

  // Header
  this->write_byte(DF2301Q_UART_HEAD_LOW);
  this->write_byte(DF2301Q_UART_HEAD_HIGH);

  // Data length (little-endian)
  this->write_byte(payload_len & 0xFF);
  this->write_byte((payload_len >> 8) & 0xFF);

  // type + cmd + seq – these bytes are included in the checksum
  uint16_t cksum = 0;
  auto add_byte = [&](uint8_t b) {
    cksum += b;
    this->write_byte(b);
  };

  add_byte(msg.msg_type);
  add_byte(msg.msg_cmd);
  add_byte(this->send_seq_++);

  for (uint8_t i = 0; i < payload_len; i++)
    add_byte(msg.msg_data[i]);

  // Checksum (little-endian)
  this->write_byte(cksum & 0xFF);
  this->write_byte((cksum >> 8) & 0xFF);

  // Tail
  this->write_byte(DF2301Q_UART_TAIL);
}

/**
 * Helper: send a SET_CONFIG packet with one uint32 value.
 *
 * Payload: cfg_type(1) + value(4) = 5 bytes
 */
void DF2301QUARTComponent::send_setting_(uint8_t cfg_type, uint32_t value) {
  UartMsg msg{};
  msg.msg_type    = DF2301Q_TYPE_CMD_DOWN;
  msg.msg_cmd     = DF2301Q_CMD_SET_CONFIG;
  msg.msg_data[0] = cfg_type;
  std::memcpy(&msg.msg_data[1], &value, 4);
  this->send_packet_(msg, 5);
}

}  // namespace df2301q
}  // namespace esphome
