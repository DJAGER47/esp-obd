# Примеры реализации ключевых тестов OBD2

## Критические тесты для немедленной реализации

### 1. Тесты метода findResponse()

#### Тест базового парсинга ответа
```cpp
// Тест: Парсинг стандартного ответа OBD2
void test_findResponse_single_response_normal_query() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: имитируем payload после получения ответа
    strcpy(obd2.payload, "410C1AF8");  // SERVICE_01 + 0x40, ENGINE_RPM, данные
    obd2.query[0] = '0'; obd2.query[1] = '1';  // SERVICE_01
    obd2.query[2] = '0'; obd2.query[3] = 'C';  // ENGINE_RPM
    obd2.longQuery = false;
    obd2.isMode0x22Query = false;
    
    // Выполнение
    uint64_t response = obd2.findResponse();
    
    // Проверка
    TEST_ASSERT_EQUAL_HEX64_MESSAGE(0x1AF8, response, 
        "findResponse должен корректно извлечь данные RPM");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(4, obd2.numPayChars, 
        "Количество символов payload должно быть 4");
}

// Тест: Обработка двойного ответа
void test_findResponse_double_response_detection() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: двойной ответ (некоторые адаптеры отправляют дубликаты)
    strcpy(obd2.payload, "410C1AF8410C1AF8");
    obd2.query[0] = '0'; obd2.query[1] = '1';
    obd2.query[2] = '0'; obd2.query[3] = 'C';
    obd2.longQuery = false;
    
    // Выполнение
    uint64_t response = obd2.findResponse();
    
    // Проверка: должен обработать только первый ответ
    TEST_ASSERT_EQUAL_HEX64_MESSAGE(0x1AF8, response, 
        "findResponse должен обработать первый ответ при двойном ответе");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(4, obd2.numPayChars, 
        "numPayChars должен соответствовать первому ответу");
}

// Тест: Обработка длинного запроса (mode 0x22)
void test_findResponse_mode_0x22_query() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: mode 0x22 ответ с zero-padding
    strcpy(obd2.payload, "6200121234");  // 0x22 + 0x40, zero-padded PID, данные
    obd2.query[0] = '2'; obd2.query[1] = '2';  // SERVICE 0x22
    obd2.query[2] = '1'; obd2.query[3] = '2';  // PID 0x12
    obd2.longQuery = false;
    obd2.isMode0x22Query = true;
    
    // Выполнение
    uint64_t response = obd2.findResponse();
    
    // Проверка
    TEST_ASSERT_EQUAL_HEX64_MESSAGE(0x1234, response, 
        "findResponse должен корректно обработать mode 0x22 запрос");
}
```

### 2. Тесты метода conditionResponse()

#### Тест базового масштабирования
```cpp
// Тест: Базовое масштабирование без смещения
void test_conditionResponse_basic_scaling() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: response = 0x80 (128 в десятичной)
    obd2.response = 0x80;
    obd2.numPayChars = 2;  // 1 байт = 2 hex символа
    
    // Выполнение: масштабирование для ENGINE_LOAD (100/255)
    double result = obd2.conditionResponse(1, 100.0/255.0, 0);
    
    // Проверка: 128 * (100/255) = 50.196%
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, 50.196f, (float)result, 
        "conditionResponse должен корректно масштабировать ENGINE_LOAD");
}

// Тест: Масштабирование со смещением
void test_conditionResponse_with_bias() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: response = 0x5A (90 в десятичной)
    obd2.response = 0x5A;
    obd2.numPayChars = 2;
    
    // Выполнение: для ENGINE_COOLANT_TEMP (scale=1, bias=-40)
    double result = obd2.conditionResponse(1, 1.0, -40.0);
    
    // Проверка: 90 - 40 = 50°C
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1f, 50.0f, (float)result, 
        "conditionResponse должен корректно применить смещение");
}

// Тест: Обработка переполнения
void test_conditionResponse_overflow_protection() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: запрос больше байт чем доступно
    obd2.response = 0x1234;
    obd2.numPayChars = 4;  // 2 байта доступно
    
    // Выполнение: запрос 4 байт (больше чем доступно)
    double result = obd2.conditionResponse(4, 1.0, 0);
    
    // Проверка: должен вернуть 0 при переполнении
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0.0, result, 
        "conditionResponse должен вернуть 0 при запросе слишком многих байт");
}

// Тест: Обнаружение trailing zeros
void test_conditionResponse_lagging_zeros_detection() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: response с trailing zeros
    obd2.response = 0x123400;  // Данные в старших байтах
    obd2.numPayChars = 6;      // 3 байта
    
    // Выполнение: ожидаем 2 байта данных
    double result = obd2.conditionResponse(2, 1.0, 0);
    
    // Проверка: должен сдвинуть и извлечь 0x1234
    TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(0x1234, result, 
        "conditionResponse должен корректно обработать trailing zeros");
}
```

### 3. Тесты метода selectCalculator()

#### Тест выбора калькулятора для ENGINE_RPM
```cpp
// Тест: Калькулятор для ENGINE_RPM
void test_selectCalculator_engine_rpm() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: данные RPM в response_A и response_B
    obd2.response_A = 0x1A;  // 26
    obd2.response_B = 0xF8;  // 248
    
    // Выполнение
    std::function<double()> calculator = obd2.selectCalculator(OBD2::ENGINE_RPM);
    
    // Проверка: калькулятор должен существовать
    TEST_ASSERT_NOT_NULL_MESSAGE(calculator.target<double(*)()>(), 
        "selectCalculator должен вернуть калькулятор для ENGINE_RPM");
    
    // Проверка: вычисление RPM = ((A*256)+B)/4
    double rpm = calculator();
    double expected_rpm = ((26 * 256) + 248) / 4.0;  // 1726 RPM
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.1f, (float)expected_rpm, (float)rpm, 
        "Калькулятор ENGINE_RPM должен корректно вычислить обороты");
}

// Тест: Калькулятор для MAF_FLOW_RATE
void test_selectCalculator_maf_flow_rate() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: данные MAF
    obd2.response_A = 0x01;  // 1
    obd2.response_B = 0x90;  // 144
    
    // Выполнение
    std::function<double()> calculator = obd2.selectCalculator(OBD2::MAF_FLOW_RATE);
    
    // Проверка
    TEST_ASSERT_NOT_NULL_MESSAGE(calculator.target<double(*)()>(), 
        "selectCalculator должен вернуть калькулятор для MAF_FLOW_RATE");
    
    // Проверка: MAF = ((A*256)+B)/100
    double maf = calculator();
    double expected_maf = ((1 * 256) + 144) / 100.0;  // 4.0 g/s
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(0.01f, (float)expected_maf, (float)maf, 
        "Калькулятор MAF_FLOW_RATE должен корректно вычислить расход воздуха");
}

// Тест: Возврат nullptr для неподдерживаемого PID
void test_selectCalculator_unsupported_pid() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Выполнение: запрос калькулятора для PID, который использует стандартную формулу
    std::function<double()> calculator = obd2.selectCalculator(OBD2::ENGINE_LOAD);
    
    // Проверка: должен вернуть nullptr (используется стандартная формула)
    TEST_ASSERT_NULL_MESSAGE(calculator.target<double(*)()>(), 
        "selectCalculator должен вернуть nullptr для PID со стандартной формулой");
}
```

### 4. Тесты SERVICE_02 и SERVICE_03

#### Тест SERVICE_02 (Freeze Frame Data)
```cpp
// Тест: Запрос freeze frame данных
void test_service_02_freeze_frame_request() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: ответ SERVICE_02 с freeze frame данными
    IIsoTp::Message response = create_obd_response_4_bytes(
        0x7E8, OBD2::SERVICE_02, OBD2::ENGINE_RPM, 0x1A, 0xF8, 0x00, 0x01);
    g_mock_iso_tp.add_receive_message(response);
    g_mock_iso_tp.set_receive_result(true);
    
    // Выполнение: запрос freeze frame для ENGINE_RPM
    obd2.queryPID(OBD2::SERVICE_02, OBD2::ENGINE_RPM);
    int8_t result = obd2.get_response();
    
    // Проверка
    TEST_ASSERT_EQUAL_INT8_MESSAGE(OBD2::OBD_SUCCESS, result, 
        "SERVICE_02 запрос должен быть успешным");
    TEST_ASSERT_TRUE_MESSAGE(g_mock_iso_tp.send_called, 
        "Запрос SERVICE_02 должен быть отправлен");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(OBD2::SERVICE_02, 
        g_mock_iso_tp.last_sent_message.data[0], 
        "Первый байт должен быть SERVICE_02");
    
    // Очистка
    delete[] response.data;
}

// Тест: SERVICE_03 запрос DTC кодов
void test_service_03_dtc_request() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: ответ с DTC кодами
    // Формат: количество кодов + коды DTC
    uint8_t dtc_data[] = {0x43, 0x02, 0x01, 0x33, 0x02, 0x44};  // 2 кода: P0133, P0244
    IIsoTp::Message response;
    response.tx_id = 0x7DF;
    response.rx_id = 0x7E8;
    response.len = 6;
    response.data = new uint8_t[6];
    memcpy(response.data, dtc_data, 6);
    
    g_mock_iso_tp.add_receive_message(response);
    g_mock_iso_tp.set_receive_result(true);
    
    // Выполнение: запрос DTC кодов
    obd2.queryPID(OBD2::SERVICE_03, 0x00);  // SERVICE_03 не использует PID
    int8_t result = obd2.get_response();
    
    // Проверка
    TEST_ASSERT_EQUAL_INT8_MESSAGE(OBD2::OBD_SUCCESS, result, 
        "SERVICE_03 запрос должен быть успешным");
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(OBD2::SERVICE_03, 
        g_mock_iso_tp.last_sent_message.data[0], 
        "Первый байт должен быть SERVICE_03");
    
    // Очистка
    delete[] response.data;
}
```

### 5. Тесты обработки ошибок

#### Тест обработки негативного ответа
```cpp
// Тест: Обработка негативного ответа (0x7F)
void test_negative_response_handling() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: негативный ответ
    IIsoTp::Message error_response = create_obd_error_response(
        0x7E8, OBD2::SERVICE_01, 0x12);  // serviceNotSupported
    g_mock_iso_tp.add_receive_message(error_response);
    g_mock_iso_tp.set_receive_result(true);
    
    // Выполнение
    obd2.queryPID(OBD2::SERVICE_01, OBD2::ENGINE_RPM);
    int8_t result = obd2.get_response();
    
    // Проверка: должен обнаружить ошибку в ответе
    TEST_ASSERT_NOT_EQUAL_MESSAGE(OBD2::OBD_SUCCESS, result, 
        "get_response должен обнаружить негативный ответ");
    
    // Проверка payload содержит код ошибки
    TEST_ASSERT_TRUE_MESSAGE(strstr(obd2.payload, "7F") != NULL, 
        "Payload должен содержать код негативного ответа");
    
    // Очистка
    delete[] error_response.data;
}

// Тест: Обработка таймаута
void test_timeout_handling() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp, 100);  // Короткий таймаут 100ms
    
    // Подготовка: не добавляем ответ, имитируем таймаут
    g_mock_iso_tp.set_receive_result(false);
    
    // Выполнение
    obd2.queryPID(OBD2::SERVICE_01, OBD2::ENGINE_RPM);
    
    // Имитируем прохождение времени
    vTaskDelay(pdMS_TO_TICKS(150));  // Ждем больше таймаута
    
    int8_t result = obd2.get_response();
    
    // Проверка
    TEST_ASSERT_EQUAL_INT8_MESSAGE(OBD2::OBD_TIMEOUT, result, 
        "get_response должен вернуть OBD_TIMEOUT при таймауте");
}

// Тест: Обработка переполнения буфера
void test_buffer_overflow_protection() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка: очень длинный ответ (больше размера буфера)
    IIsoTp::Message large_response;
    large_response.tx_id = 0x7DF;
    large_response.rx_id = 0x7E8;
    large_response.len = 200;  // Больше стандартного буфера
    large_response.data = new uint8_t[200];
    
    // Заполняем данными
    for (int i = 0; i < 200; i++) {
        large_response.data[i] = 0xAA;
    }
    
    g_mock_iso_tp.add_receive_message(large_response);
    g_mock_iso_tp.set_receive_result(true);
    
    // Выполнение
    obd2.queryPID(OBD2::SERVICE_01, OBD2::ENGINE_RPM);
    int8_t result = obd2.get_response();
    
    // Проверка: должен обработать без падения
    TEST_ASSERT_TRUE_MESSAGE(result == OBD2::OBD_SUCCESS || 
                            result == OBD2::OBD_BUFFER_OVERFLOW, 
        "Система должна корректно обработать большой ответ");
    
    // Очистка
    delete[] large_response.data;
}
```

### 6. Примеры тестов производительности

#### Тест производительности одиночного запроса
```cpp
// Тест: Измерение времени выполнения одиночного PID запроса
void test_single_pid_query_performance() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    // Подготовка
    IIsoTp::Message response = create_obd_response_2_bytes(
        0x7E8, OBD2::SERVICE_01, OBD2::ENGINE_RPM, 0x1A, 0xF8);
    g_mock_iso_tp.add_receive_message(response);
    g_mock_iso_tp.set_receive_result(true);
    
    // Измерение времени
    uint32_t start_time = xTaskGetTickCount();
    
    // Выполнение
    obd2.queryPID(OBD2::SERVICE_01, OBD2::ENGINE_RPM);
    int8_t result = obd2.get_response();
    double rpm = obd2.rpm();
    
    uint32_t end_time = xTaskGetTickCount();
    uint32_t duration_ms = (end_time - start_time) * portTICK_PERIOD_MS;
    
    // Проверка
    TEST_ASSERT_EQUAL_INT8_MESSAGE(OBD2::OBD_SUCCESS, result, 
        "Запрос должен быть успешным");
    TEST_ASSERT_LESS_THAN_MESSAGE(50, duration_ms, 
        "Одиночный PID запрос должен выполняться менее чем за 50ms");
    TEST_ASSERT_FLOAT_WITHIN_MESSAGE(1.0f, 1726.0f, (float)rpm, 
        "RPM должен быть корректно вычислен");
    
    // Очистка
    delete[] response.data;
}

// Тест: Стресс-тест множественных запросов
void test_multiple_requests_stress() {
    g_mock_iso_tp.reset();
    OBD2 obd2(g_mock_iso_tp);
    
    const int NUM_REQUESTS = 100;
    uint32_t successful_requests = 0;
    
    // Подготовка: добавляем много ответов
    for (int i = 0; i < NUM_REQUESTS; i++) {
        IIsoTp::Message response = create_obd_response_2_bytes(
            0x7E8, OBD2::SERVICE_01, OBD2::ENGINE_RPM, 0x1A, 0xF8);
        g_mock_iso_tp.add_receive_message(response);
    }
    g_mock_iso_tp.set_receive_result(true);
    
    uint32_t start_time = xTaskGetTickCount();
    
    // Выполнение: множественные запросы
    for (int i = 0; i < NUM_REQUESTS; i++) {
        obd2.queryPID(OBD2::SERVICE_01, OBD2::ENGINE_RPM);
        if (obd2.get_response() == OBD2::OBD_SUCCESS) {
            successful_requests++;
        }
    }
    
    uint32_t end_time = xTaskGetTickCount();
    uint32_t total_duration_ms = (end_time - start_time) * portTICK_PERIOD_MS;
    
    // Проверка
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(NUM_REQUESTS, successful_requests, 
        "Все запросы должны быть успешными");
    TEST_ASSERT_LESS_THAN_MESSAGE(5000, total_duration_ms, 
        "100 запросов должны выполняться менее чем за 5 секунд");
    
    // Проверка средней производительности
    uint32_t avg_time_per_request = total_duration_ms / NUM_REQUESTS;
    TEST_ASSERT_LESS_THAN_MESSAGE(50, avg_time_per_request, 
        "Среднее время на запрос должно быть менее 50ms");
}
```

## Рекомендации по реализации

### 1. Порядок реализации тестов
1. **Начать с базовых тестов:** findResponse, conditionResponse, selectCalculator
2. **Перейти к PID тестам:** группы 1-20, затем остальные
3. **Добавить тесты сервисов:** SERVICE_02, SERVICE_03
4. **Завершить тестами ошибок и производительности**

### 2. Общие принципы
- **Изоляция:** Каждый тест должен быть независимым
- **Очистка:** Всегда освобождать выделенную память
- **Документирование:** Подробные комментарии для каждого теста
- **Валидация:** Проверять все аспекты функциональности

### 3. Инструменты отладки
```cpp
// Макрос для отладочного вывода в тестах
#define TEST_DEBUG_PRINT(format, ...) \
    do { \
        if (OBD2::OBD_DEBUG) { \
            printf("[TEST DEBUG] " format "\n", ##__VA_ARGS__); \
        } \
    } while(0)

// Использование в тестах
TEST_DEBUG_PRINT("Response: 0x%llX, Expected: 0x%llX", response, expected);
```

### 4. Автоматизация тестирования
```bash
# Скрипт для запуска конкретной группы тестов
#!/bin/bash
TEST_GROUP=$1
cd unity-app
mkdir -p build
cd build
cmake ..
make
./test_runner --group=$TEST_GROUP
```

Эти примеры обеспечивают основу для реализации полного набора тестов OBD2, покрывающего все критические аспекты функциональности.