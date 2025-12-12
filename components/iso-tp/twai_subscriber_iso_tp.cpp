#include "twai_subscriber_iso_tp.h"

#include <cstring>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static const char* const TAG = "TwaiSubscriberIsoTp";

TwaiSubscriberIsoTp::TwaiSubscriberIsoTp(uint32_t queue_size) {
  xQueue_ = xQueueCreate(queue_size, sizeof(TwaiFrame));
  if (xQueue_ == nullptr) {
    ESP_LOGE(TAG, "Failed to create queue");
  }
}

#ifdef TEST_INSTANCES
#warning "only tests"
TwaiSubscriberIsoTp::~TwaiSubscriberIsoTp() {
  if (xQueue_ != nullptr) {
    vQueueDelete(xQueue_);
  }
}
#endif

bool TwaiSubscriberIsoTp::isInterested(const TwaiFrame& frame) {
  return true;
}

QueueHandle_t TwaiSubscriberIsoTp::onTwaiMessage() {
  return xQueue_;
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