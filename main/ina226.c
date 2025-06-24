#include "ina226.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define INA226_ADDR 0x40
#define I2C_MASTER_SCL_IO 8
#define I2C_MASTER_SDA_IO 10
#define I2C_MASTER_FREQ_HZ 100000

static const char *TAG = "INA226";
static i2c_master_dev_handle_t dev_handle;

// Регистры INA226 (остаются теми же)
#define INA226_REG_CONFIG     0x00
#define INA226_REG_SHUNT_VOLT 0x01
#define INA226_REG_BUS_VOLT   0x02
#define INA226_REG_POWER      0x03
#define INA226_REG_CURRENT    0x04
#define INA226_REG_CALIB      0x05

static esp_err_t ina226_write_reg(uint8_t reg, uint16_t value) {
    uint8_t data[3] = {reg, (uint8_t)(value >> 8), (uint8_t)(value & 0xFF)};
    return i2c_master_transmit(dev_handle, data, sizeof(data), pdMS_TO_TICKS(1000));
}

static esp_err_t ina226_read_reg(uint8_t reg, uint16_t *value) {
    uint8_t write_data = reg;
    uint8_t read_data[2];
    
    esp_err_t err = i2c_master_transmit_receive(dev_handle, 
                                        &write_data, 1,
                                        read_data, 2,
                                        pdMS_TO_TICKS(1000));
        if (err == ESP_OK) {
            *value = (read_data[0] << 8) | read_data[1];
    }
    return err;
}

esp_err_t ina226_init(const ina226_config_t *config) {
    if (config == NULL || config->shunt_resistance <= 0 || config->max_current <= 0) {
        ESP_LOGE(TAG, "Invalid INA226 configuration");
        return ESP_ERR_INVALID_ARG;
    }

    // Настройка I2C master
    i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = 0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };

    i2c_master_bus_handle_t bus_handle = NULL;
    esp_err_t err = i2c_new_master_bus(&i2c_bus_config, &bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize I2C master bus: %s", esp_err_to_name(err));
        return err;
    }

    // Настройка устройства INA226
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = INA226_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ
    };

    // Добавление устройства INA226
    err = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add INA226 device: %s", esp_err_to_name(err));
        if (bus_handle) {
            i2c_del_master_bus(bus_handle);
        }
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

    // Остальная часть инициализации остается прежней
    uint16_t cfg = 0x4127; // 16 samples avg, 1.1ms conversion time
    err = ina226_write_reg(INA226_REG_CONFIG, cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure INA226: %s", esp_err_to_name(err));
        return err;
    }

    float current_lsb = config->max_current / 32768.0;
    uint16_t cal = (uint16_t)(0.00512 / (current_lsb * config->shunt_resistance));
    err = ina226_write_reg(INA226_REG_CALIB, cal);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to calibrate INA226: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG, "INA226 initialized with new I2C driver");

    return ESP_OK;
}

esp_err_t ina226_read_values(float *voltage, float *current, float *power) {
    if (voltage == NULL || current == NULL || power == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t bus_voltage, shunt_voltage, current_raw, power_raw;
    esp_err_t err;
    
    // Чтение напряжения на шине
    err = ina226_read_reg(INA226_REG_BUS_VOLT, &bus_voltage);
    if (err != ESP_OK) return err;
    
    // Чтение напряжения на шунте
    err = ina226_read_reg(INA226_REG_SHUNT_VOLT, &shunt_voltage);
    if (err != ESP_OK) return err;
    
    // Чтение тока
    err = ina226_read_reg(INA226_REG_CURRENT, &current_raw);
    if (err != ESP_OK) return err;
    
    // Чтение мощности
    err = ina226_read_reg(INA226_REG_POWER, &power_raw);
    if (err != ESP_OK) return err;

    // Конвертация в физические величины
    *voltage = bus_voltage * 0.00125f; // 1.25mV на бит
    *current = current_raw * 0.001f;   // 1mA на бит (зависит от калибровки)
    *power = power_raw * 0.025f;       // 25mW на бит
    
    return ESP_OK;
}