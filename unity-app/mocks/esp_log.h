#pragma once

#include <cstdarg>
#include <cstdio>

#define ESP_LOG_INFO 1

// Заменяем esp_log_writev на vprintf
inline void esp_log_writev(int level, const char* tag, const char* format, va_list args) {
  printf("[%s] ", tag);
  vprintf(format, args);
  printf("\n");
}

// Заменяем ESP_LOGI на printf
#define ESP_LOGI(tag, format, ...)                   \
  do {                                               \
    printf("[%s] " format "\n", tag, ##__VA_ARGS__); \
  } while (0)
