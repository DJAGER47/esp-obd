#include "twai_subscriber_iso_tp.h"

#include <cstring>

#include "esp_log.h"

static const char* const TAG = "TwaiSubscriberIsoTp";

TwaiSubscriberIsoTp::TwaiSubscriberIsoTp(uint32_t queue_size) {
  xQueue_ = xQueueCreate(queue_size, sizeof(TwaiFrame));
  if (xQueue_ == nullptr) {
    ESP_LOGE(TAG, "Failed to create queue");
  }
}

bool TwaiSubscriberIsoTp::isInterested(const TwaiFrame& frame) {
  return true;
}

bool TwaiSubscriberIsoTp::onTwaiMessage(const TwaiFrame& frame) {
  if (xQueue_ == nullptr) {
    ESP_LOGE(TAG, "Queue not initialized");
    return false;
  }

  if (xQueueSend(xQueue_, &frame, 0) != pdTRUE) {
    ESP_LOGW(TAG, "Queue is full, dropping frame");
    return false;
  }

  return true;
}

bool TwaiSubscriberIsoTp::Receive(TwaiFrame& frame, TickType_t timeout_ms) {
  if (xQueue_ == nullptr) {
    ESP_LOGE(TAG, "Queue not initialized");
    return false;
  }

  if (xQueueReceive(xQueue_, &frame, timeout_ms) != pdTRUE) {
    return false;
  }

  return true;
}