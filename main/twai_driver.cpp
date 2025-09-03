#include "twai_driver.h"

#include "esp_log.h"

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

TwaiDriver::~TwaiDriver() {
  if (driver_installed) {
    stop_and_uninstall();
  }
}

esp_err_t TwaiDriver::install_and_start() {
  esp_err_t ret = twai_driver_install(&g_config, &t_config, &f_config);
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Driver installed");
    ret = twai_start();
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "Driver started");
      driver_installed = true;
    } else {
      ESP_LOGE(TAG, "Failed to start driver");
      twai_driver_uninstall();
    }
  } else {
    ESP_LOGE(TAG, "Failed to install driver");
  }
  return ret;
}

esp_err_t TwaiDriver::stop_and_uninstall() {
  esp_err_t ret = twai_stop();
  if (ret == ESP_OK) {
    ESP_LOGI(TAG, "Driver stopped");
    ret = twai_driver_uninstall();
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "Driver uninstalled");
      driver_installed = false;
    } else {
      ESP_LOGE(TAG, "Failed to uninstall driver");
    }
  } else {
    ESP_LOGE(TAG, "Failed to stop driver");
  }
  return ret;
}

esp_err_t TwaiDriver::transmit(const twai_message_t* message,
                               TickType_t ticks_to_wait) {
  if (!driver_installed) {
    return ESP_ERR_INVALID_STATE;
  }
  return twai_transmit(message, ticks_to_wait);
}

esp_err_t TwaiDriver::receive(twai_message_t* message,
                              TickType_t ticks_to_wait) {
  if (!driver_installed) {
    return ESP_ERR_INVALID_STATE;
  }
  return twai_receive(message, ticks_to_wait);
}