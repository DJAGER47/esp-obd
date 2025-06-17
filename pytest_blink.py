# SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
# SPDX-License-Identifier: CC0-1.0
import logging
import os
from unittest import mock

import pytest
from pytest_embedded_idf.dut import IdfDut

# Импорты для тестируемых модулей
from main.ina226 import ina226_read_values, ESP_OK
from main.x9c103s import x9c103s_dev_t, x9c103s_set_resistance


@pytest.mark.supported_targets
@pytest.mark.generic
def test_blink(dut: IdfDut) -> None:
    # check and log bin size
    binary_file = os.path.join(dut.app.binary_path, 'blink.bin')
    bin_size = os.path.getsize(binary_file)
    logging.info('blink_bin_size : {}KB'.format(bin_size // 1024))


# Тесты для INA226
@pytest.mark.parametrize('voltage,current,power', [
    (3.3, 0.5, 1.65),
    (5.0, 2.0, 10.0),
    (0.0, 0.0, 0.0)
])
def test_ina226_read_values(mocker, voltage, current, power):
    # Mock I2C функций
    mocker.patch('main.ina226.ina226_read_reg', side_effect=[
        int(voltage / 0.004) << 3,  # BUS_VOLT
        int(current / 0.0005),       # CURRENT
        int(power / 0.025)           # POWER
    ])
    
    v, c, p = float(), float(), float()
    err = ina226_read_values(v, c, p)
    
    assert err == ESP_OK
    assert abs(v - voltage) < 0.01
    assert abs(c - current) < 0.01
    assert abs(p - power) < 0.1


# Тесты для X9C103S
@pytest.mark.parametrize('value,expected_steps', [
    (0, 0), (50, 50), (99, 99), (100, 99)
])
def test_x9c103s_set_resistance(mocker, value, expected_steps):
    dev = x9c103s_dev_t()
    mocker.patch('main.x9c103s.gpio_config')
    mocker.patch('main.x9c103s.gpio_set_level')
    
    x9c103s_set_resistance(dev, value)
    
    # Проверяем количество шагов
    assert mock.call_count('gpio_set_level') == 3 + expected_steps


# Интеграционные тесты
def test_main_loop_logic(mocker):
    # Mock зависимостей
    mocker.patch('main.ina226.ina226_read_values', return_value=ESP_OK)
    mocker.patch('main.x9c103s.x9c103s_set_resistance', return_value=ESP_OK)
    
    # Запуск основной логики (нужно добавить импорт app_main)
    result = app_main()
    
    assert result == ESP_OK
    assert ina226_read_values.called
    assert x9c103s_set_resistance.called
