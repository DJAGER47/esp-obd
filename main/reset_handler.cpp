#include "reset_handler.h"

#include "esp_log.h"

static const char* const TAG = "reset_handler";

bool check_reset_reason(void) {
  // Проверяем причину перезагрузки
  esp_reset_reason_t reset_reason = esp_reset_reason();

  // ESP_RST_UNKNOWN,    //!< Reset reason can not be determined
  // ESP_RST_POWERON,    //!< Reset due to power-on event
  // ESP_RST_EXT,        //!< Reset by external pin (not applicable for ESP32)
  // ESP_RST_SW,         //!< Software reset via esp_restart
  // ESP_RST_PANIC,      //!< Software reset due to exception/panic
  // ESP_RST_INT_WDT,    //!< Reset (software or hardware) due to interrupt watchdog
  // ESP_RST_TASK_WDT,   //!< Reset due to task watchdog
  // ESP_RST_WDT,        //!< Reset due to other watchdogs
  // ESP_RST_DEEPSLEEP,  //!< Reset after exiting deep sleep mode
  // ESP_RST_BROWNOUT,   //!< Brownout reset (software or hardware)
  // ESP_RST_SDIO,       //!< Reset over SDIO
  // ESP_RST_USB,        //!< Reset by USB peripheral
  // ESP_RST_JTAG,       //!< Reset by JTAG
  // ESP_RST_EFUSE,      //!< Reset due to efuse error
  // ESP_RST_PWR_GLITCH, //!< Reset due to power glitch detected
  // ESP_RST_CPU_LOCKUP, //!< Reset due to CPU lock up (double exception)

  // Логируем причину перезагрузки
  switch (reset_reason) {
    case ESP_RST_POWERON:
      ESP_LOGI(TAG, "Reset reason: Power on reset");
      break;
    case ESP_RST_EXT:
      ESP_LOGI(TAG, "Reset reason: External pin reset");
      break;
    case ESP_RST_SW:
      ESP_LOGI(TAG, "Reset reason: Software reset via esp_restart");
      break;
    case ESP_RST_PANIC:
      ESP_LOGE(TAG, "Reset reason: Software reset due to exception/panic");
      break;
    case ESP_RST_INT_WDT:
      ESP_LOGE(TAG, "Reset reason: Interrupt watchdog reset");
      break;
    case ESP_RST_TASK_WDT:
      ESP_LOGE(TAG, "Reset reason: Task watchdog reset");
      break;
    case ESP_RST_WDT:
      ESP_LOGE(TAG, "Reset reason: Other watchdog reset");
      break;
    case ESP_RST_DEEPSLEEP:
      ESP_LOGI(TAG, "Reset reason: Deep sleep reset");
      break;
    case ESP_RST_BROWNOUT:
      ESP_LOGE(TAG, "Reset reason: Brownout reset");
      break;
    case ESP_RST_SDIO:
      ESP_LOGI(TAG, "Reset reason: SDIO reset");
      break;
    default:
      ESP_LOGW(TAG, "Reset reason: Unknown");
      break;
  }

  switch (reset_reason) {
    case ESP_RST_UNKNOWN:
    case ESP_RST_SW:
    case ESP_RST_PANIC:
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:
    case ESP_RST_BROWNOUT:
      ESP_LOGE(TAG, "System reset due to error detected.");
      return false;

    default:
      break;
  }

  return true;  // Нормальная перезагрузка, можно продолжать работу
}