#include "twai_driver.h"

#include "esp_log.h"
#include "twai_errors.h"

static const char* TAG = "TwaiDriver";

TwaiDriver::TwaiDriver(gpio_num_t tx_pin,
                       gpio_num_t rx_pin,
                       uint32_t speed_kbps) :
    driver_installed(false) {
  g_config = TWAI_GENERAL_CONFIG_DEFAULT(tx_pin, rx_pin, TWAI_MODE_NORMAL);

  switch (speed_kbps) {
    case 125:
      t_config = TWAI_TIMING_CONFIG_125KBITS();
      break;
    case 250:
      t_config = TWAI_TIMING_CONFIG_250KBITS();
      break;
    case 500:
      t_config = TWAI_TIMING_CONFIG_500KBITS();
      break;
    case 1000:
      t_config = TWAI_TIMING_CONFIG_1MBITS();
      break;
    default:
      // По умолчанию 500 кбит/с для OBD-II
      t_config = TWAI_TIMING_CONFIG_500KBITS();
      break;
  }

  f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();
}

TwaiError TwaiDriver::install_and_start() {
  esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Driver installed");
    ret = twai_start();
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "Driver started");
      driver_installed = true;
      return TwaiError::OK;
    } else {
      ESP_LOGE(TAG, "Failed to start driver: %s", esp_err_to_name(ret));
      twai_driver_uninstall();
      return TwaiError::DRIVER_START_FAILED;
    }
  } else {
    ESP_LOGE(TAG, "Failed to install driver: %s", esp_err_to_name(ret));
    return TwaiError::DRIVER_INSTALL_FAILED;
  }
}

TwaiError TwaiDriver::transmit(const twai_message_t* message,
                               TickType_t ticks_to_wait) {
  if (!driver_installed) {
    return TwaiError::INVALID_STATE;
  }

  esp_err_t ret = twai_transmit(message, ticks_to_wait);
  if (ret == ESP_OK) {
    return TwaiError::OK;
  } else if (ret == ESP_ERR_TIMEOUT) {
    return TwaiError::TIMEOUT;
  } else {
    ESP_LOGE(TAG, "Failed to transmit message: %s", esp_err_to_name(ret));
    return TwaiError::TRANSMIT_FAILED;
  }
}

TwaiError TwaiDriver::receive(twai_message_t* message,
                              TickType_t ticks_to_wait) {
  if (!driver_installed) {
    return TwaiError::INVALID_STATE;
  }

  esp_err_t ret = twai_receive(message, ticks_to_wait);
  if (ret == ESP_OK) {
    return TwaiError::OK;
  } else if (ret == ESP_ERR_TIMEOUT) {
    return TwaiError::TIMEOUT;
  } else {
    ESP_LOGE(TAG, "Failed to receive message: %s", esp_err_to_name(ret));
    return TwaiError::RECEIVE_FAILED;
  }
}