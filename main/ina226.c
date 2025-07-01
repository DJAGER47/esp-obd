#include "io.h"
#include "ina226.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define INA226_ADDR 0x40
#define I2C_MASTER_FREQ_HZ 100000

// Регистры INA226
#define INA226_REG_CONFIG     0x00
#define INA226_REG_SHUNT_VOLT 0x01
#define INA226_REG_BUS_VOLT   0x02
#define INA226_REG_POWER      0x03
#define INA226_REG_CURRENT    0x04
#define INA226_REG_CALIB      0x05
#define INA226_REG_ManufID    0xFE

static const char *TAG = "INA226";
static i2c_master_dev_handle_t dev_handle;
static i2c_master_bus_handle_t bus_handle;
static float current_lsb = 0;

static i2c_master_bus_config_t i2c_bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = I2C_NUM_0,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true
    };
static i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = INA226_ADDR,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ
    };

static esp_err_t ina226_write_reg(uint8_t reg, uint16_t value) {
    if (dev_handle == NULL) {
        ESP_LOGE(TAG, "Device handle is NULL");
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t data[3] = {reg, (uint8_t)(value >> 8), (uint8_t)(value & 0xFF)};
    esp_err_t err = i2c_master_transmit(dev_handle, data, sizeof(data), pdMS_TO_TICKS(1000));
    if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "I2C bus in invalid state, try reinitializing");
    }
    return err;
}

static esp_err_t ina226_read_reg(uint8_t reg, uint16_t *value) {
    if (dev_handle == NULL) {
        ESP_LOGE(TAG, "Device handle is NULL");
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t write_data = reg;
    uint8_t read_data[2];
    
    esp_err_t err = i2c_master_transmit_receive(dev_handle,
                                        &write_data, 1,
                                        read_data, 2,
                                        pdMS_TO_TICKS(1000));
    if (err == ESP_OK) {
        *value = (read_data[0] << 8) | read_data[1];
    } else if (err == ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "I2C bus in invalid state during read, try reinitializing");
    }
    return err;
}

void ina226_init(const ina226_config_t *config) {
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_config, &bus_handle));
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    // Вывод всех регистров INA226 в 16-ричном формате
    ESP_LOGI(TAG, "INA226 register dump:");
    uint16_t reg_value;
    const uint8_t regs[] = {INA226_REG_CONFIG, INA226_REG_SHUNT_VOLT,
                            INA226_REG_BUS_VOLT, INA226_REG_POWER,
                            INA226_REG_CURRENT, INA226_REG_CALIB, INA226_REG_ManufID};
    const char *reg_names[] = {"CONFIG", "SHUNT_VOLT", "BUS_VOLT",
                              "POWER", "CURRENT", "CALIB", "MANUFID"};
    
    for (int i = 0; i < sizeof(regs)/sizeof(regs[0]); i++) {
        esp_err_t err = ina226_read_reg(regs[i], &reg_value);
        if (err == ESP_OK) {
            ESP_LOGI(TAG, "  %s: 0x%04X", reg_names[i], reg_value);
        } else {
            ESP_LOGE(TAG, "Failed to read register 0x%02X: %s",
                    regs[i], esp_err_to_name(err));
        }
    }

    
    // Остальная часть инициализации остается прежней
    uint16_t cfg = 0x4327; // 16 samples avg, 1.1ms conversion time
    ESP_ERROR_CHECK(ina226_write_reg(INA226_REG_CONFIG, cfg));

    current_lsb = config->max_current / 32768.0;
    uint16_t cal = (uint16_t)(0.00512 / (current_lsb * config->shunt_resistance));
    ESP_ERROR_CHECK(ina226_write_reg(INA226_REG_CALIB, cal));
    
    ESP_LOGI(TAG, "cfg 0x%04X | cal 0x%04X", cfg, cal);
    ESP_LOGI(TAG, "INA226 initialized with new I2C driver");
}

esp_err_t ina226_read_values(float *voltage, float *current, float *power) {
    uint16_t bus_voltage, power_raw;
    int16_t shunt_voltage, current_raw;
    esp_err_t err;
    
    // Чтение напряжения на шунте
    err = ina226_read_reg(INA226_REG_SHUNT_VOLT, (uint16_t*)&shunt_voltage);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: %s", "Failed to read SHUNT_VOLT", esp_err_to_name(err));
    }

    // Чтение напряжения на шине
    err = ina226_read_reg(INA226_REG_BUS_VOLT, &bus_voltage);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: %s", "Failed to read BUS_VOLT", esp_err_to_name(err));
    }
    
    // Чтение тока
    err = ina226_read_reg(INA226_REG_CURRENT, (uint16_t*)&current_raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: %s", "Failed to read CURRENT", esp_err_to_name(err));
    }

    // Чтение мощности
    err = ina226_read_reg(INA226_REG_POWER, &power_raw);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s: %s", "Failed to read POWER", esp_err_to_name(err));
    }
    

    // Конвертация в физические величины
    float shunt = shunt_voltage * 0.0000025f; // 2.5 µV на бит
    *voltage = bus_voltage * 0.00125f;        // 1.25 mV на бит
    *current = current_raw * current_lsb;
    *power = power_raw * current_lsb * 25.0f;


    printf("Current readings - Shunt: %.6fV V: %.2fV, I: %.3fA, P: %.3fW\n\r", shunt, *voltage, *current, *power);

    return ESP_OK;
}