#include "ina226.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define INA226_ADDR 0x40
#define I2C_MASTER_SCL_IO 8
#define I2C_MASTER_SDA_IO 10
#define I2C_MASTER_FREQ_HZ 100000
#define I2C_MASTER_PORT_NUM 0

// Регистры INA226
#define INA226_REG_CONFIG     0x00
#define INA226_REG_SHUNT_VOLT 0x01
#define INA226_REG_BUS_VOLT   0x02
#define INA226_REG_POWER      0x03
#define INA226_REG_CURRENT    0x04
#define INA226_REG_CALIB      0x05

static const char *TAG = "INA226";

static esp_err_t ina226_write_reg(uint8_t reg, uint16_t value) {
    uint8_t data[3] = {reg, (uint8_t)(value >> 8), (uint8_t)(value & 0xFF)};
    return i2c_master_write_to_device(I2C_MASTER_PORT_NUM, INA226_ADDR, data, sizeof(data), pdMS_TO_TICKS(1000));
}

static esp_err_t ina226_read_reg(uint8_t reg, uint16_t *value) {
    uint8_t data[2];
    esp_err_t err = i2c_master_write_read_device(I2C_MASTER_PORT_NUM, INA226_ADDR, &reg, 1, data, 2, pdMS_TO_TICKS(1000));
    if (err == ESP_OK) {
        *value = (data[0] << 8) | data[1];
    }
    return err;
}

// Конфигурация INA226
esp_err_t ina226_init(const ina226_config_t *config) {
    if (config == NULL || config->shunt_resistance <= 0 || config->max_current <= 0) {
        ESP_LOGE(TAG, "Invalid INA226 configuration");
        return ESP_ERR_INVALID_ARG;
    }

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    
    esp_err_t err = i2c_param_config(I2C_MASTER_PORT_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C config failed: %s", esp_err_to_name(err));
        return err;
    }
    
    err = i2c_driver_install(I2C_MASTER_PORT_NUM, conf.mode, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %s", esp_err_to_name(err));
        return err;
    }

    // Вывод всех регистров INA226 в 16-ричном формате
    ESP_LOGI(TAG, "INA226 register dump:");
    uint16_t reg_value;
    const uint8_t regs[] = {INA226_REG_CONFIG, INA226_REG_SHUNT_VOLT,
                            INA226_REG_BUS_VOLT, INA226_REG_POWER,
                            INA226_REG_CURRENT, INA226_REG_CALIB};
    const char *reg_names[] = {"CONFIG", "SHUNT_VOLT", "BUS_VOLT",
                              "POWER", "CURRENT", "CALIB"};
    
    for (int i = 0; i < sizeof(regs)/sizeof(regs[0]); i++) {
        err = ina226_read_reg(regs[i], &reg_value);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "  %s: 0x%04X", reg_names[i], reg_value);
        } else {
            ESP_LOGE(TAG, "Failed to read register 0x%02X: %s",
                    regs[i], esp_err_to_name(err));
        }
    }

    // Конфигурация INA226
    uint16_t cfg = 0x4127; // 16 samples avg, 1.1ms conversion time
    err = ina226_write_reg(INA226_REG_CONFIG, cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure INA226: %s", esp_err_to_name(err));
        return err;
    }

    // Расчет калибровки
    float current_lsb = config->max_current / 32768.0;
    uint16_t cal = (uint16_t)(0.00512 / (current_lsb * config->shunt_resistance));
    err = ina226_write_reg(INA226_REG_CALIB, cal);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to calibrate INA226: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "INA226 initialized. Shunt: %.4f Ohm, Max current: %.2fA",
             config->shunt_resistance, config->max_current);
    return ESP_OK;
}

esp_err_t ina226_read_values(float *voltage, float *current, float *power) {
    if (voltage == NULL || current == NULL || power == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t raw;
    esp_err_t err;

    // Чтение напряжения шины с проверкой ошибок
    err = ina226_read_reg(INA226_REG_BUS_VOLT, &raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read bus voltage: %s", esp_err_to_name(err));
        return err;
    }
    *voltage = (raw >> 3) * 0.004; // 4mV per bit

    // Чтение тока с проверкой ошибок
    err = ina226_read_reg(INA226_REG_CURRENT, &raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read current: %s", esp_err_to_name(err));
        return err;
    }
    *current = (int16_t)raw * 0.0005; // 500uA per bit

    // Чтение мощности с проверкой ошибок
    err = ina226_read_reg(INA226_REG_POWER, &raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read power: %s", esp_err_to_name(err));
        return err;
    }
    *power = raw * 0.025;

    return ESP_OK;
}