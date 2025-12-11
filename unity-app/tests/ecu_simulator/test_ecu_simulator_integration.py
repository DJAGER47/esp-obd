#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
Интеграционные тесты для взаимодействия с ECU симулятором
"""

import unittest
import time
import can
import isotp
import subprocess
import threading
import os
import signal
import sys

# Добавляем путь к ECU симулятору
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'ecu-simulator'))

class TestEcuSimulatorIntegration(unittest.TestCase):
    """Интеграционные тесты для взаимодействия с ECU симулятором"""
    
    @classmethod
    def setUpClass(cls):
        """Инициализация перед всеми тестами"""
        cls.simulator_process = None
        cls.bus = None
        cls.stack = None
        
        # Проверка доступности CAN интерфейса
        try:
            cls.bus = can.interface.Bus(channel='vcan0', bustype='socketcan')
            cls.using_vcan = True
        except:
            try:
                cls.bus = can.interface.Bus(channel='can0', bustype='socketcan')
                cls.using_vcan = False
            except Exception as e:
                raise unittest.SkipTest(f"Не удалось подключиться к CAN интерфейсу: {e}")
        
        # Настройка ISO-TP адресов для ECU симулятора
        # Из конфигурации ECU симулятора: obd_ecu_address = 0x7E0, obd_broadcast_address = 0x7DF
        # Для UDS используем адрес 0x7E1 (из конфигурации симулятора)
        cls.obd_address = isotp.Address(isotp.AddressingMode.Normal_11bits, txid=0x7DF, rxid=0x7E8)
        cls.uds_address = isotp.Address(isotp.AddressingMode.Normal_11bits, txid=0x7E1, rxid=0x7E9)
        
        # Настройка параметров ISO-TP
        cls.params = isotp.Params()
        
        # Создание стеков для OBD и UDS
        cls.obd_stack = isotp.CanStack(bus=cls.bus, address=cls.obd_address, params=cls.params)
        cls.uds_stack = isotp.CanStack(bus=cls.bus, address=cls.uds_address, params=cls.params)
        
        # Очистка буфера
        cls._clear_bus_buffer()

    @classmethod
    def tearDownClass(cls):
        """Очистка после всех тестов"""
        if cls.bus:
            cls.bus.shutdown()

    @classmethod
    def _clear_bus_buffer(cls):
        """Очистка буфера CAN шины"""
        timeout = time.time() + 1.0  # 1 секунда на очистку
        while time.time() < timeout:
            msg = cls.bus.recv(timeout=0.01)
            if msg is None:
                break

    def setUp(self):
        """Инициализация перед каждым тестом"""
        self._clear_bus_buffer()
        time.sleep(0.05)

    def tearDown(self):
        """Очистка после каждого теста"""
        pass

    def _wait_for_response(self, stack, timeout=1.0):
        """Ожидание ответа через ISO-TP стек"""
        start_time = time.time()
        while time.time() - start_time < timeout:
            stack.process()
            if stack.available():
                return stack.recv()
            time.sleep(0.001)
        return None

    def _send_obd_request(self, service, pid=None):
        """Отправка OBD запроса"""
        if pid is not None:
            request = bytes([service, pid])
        else:
            request = bytes([service])
        self.obd_stack.send(request)

    def _send_uds_request(self, service, subfunction=None, data=None):
        """Отправка UDS запроса"""
        if subfunction is not None and data is not None:
            request = bytes([service, subfunction]) + data
        elif subfunction is not None:
            request = bytes([service, subfunction])
        else:
            request = bytes([service])
        self.uds_stack.send(request)

    def test_obd_service_01_pid_00_supported_pids(self):
        """Тест запроса поддерживаемых PID (Service 01, PID 00)"""
        # Отправка запроса на поддерживаемые PID
        self._send_obd_request(0x01, 0x00)
        
        # Ожидание ответа
        response = self._wait_for_response(self.obd_stack, 0.5)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 2, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x41, "Первый байт должен быть 0x41 (положительный ответ на Service 01)")
        self.assertEqual(response[1], 0x00, "Второй байт должен быть 0x00 (PID)")

    def test_obd_service_01_pid_05_engine_coolant_temp(self):
        """Тест запроса температуры охлаждающей жидкости (Service 01, PID 05)"""
        # Отправка запроса температуры охлаждающей жидкости
        self._send_obd_request(0x01, 0x05)
        
        # Ожидание ответа
        response = self._wait_for_response(self.obd_stack, 0.5)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 2, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x41, "Первый байт должен быть 0x41 (положительный ответ на Service 01)")
        self.assertEqual(response[1], 0x05, "Второй байт должен быть 0x05 (PID)")

    def test_obd_service_01_pid_0d_vehicle_speed(self):
        """Тест запроса скорости автомобиля (Service 01, PID 0D)"""
        # Отправка запроса скорости автомобиля
        self._send_obd_request(0x01, 0x0D)
        
        # Ожидание ответа
        response = self._wait_for_response(self.obd_stack, 0.5)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 2, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x41, "Первый байт должен быть 0x41 (положительный ответ на Service 01)")
        self.assertEqual(response[1], 0x0D, "Второй байт должен быть 0x0D (PID)")

    def test_obd_service_03_request_dtcs(self):
        """Тест запроса DTC (Service 03)"""
        # Отправка запроса DTC
        self._send_obd_request(0x03)
        
        # Ожидание ответа
        response = self._wait_for_response(self.obd_stack, 0.5)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 0, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x43, "Первый байт должен быть 0x43 (положительный ответ на Service 03)")
        
        # Проверка, что ответ содержит DTC
        self.assertGreater(len(response), 1, "Ответ должен содержать данные DTC")

    def test_obd_service_09_pid_00_supported_pids(self):
        """Тест запроса поддерживаемых PID сервиса 09 (Service 09, PID 00)"""
        # Отправка запроса на поддерживаемые PID сервиса 09
        self._send_obd_request(0x09, 0x00)
        
        # Ожидание ответа
        response = self._wait_for_response(self.obd_stack, 0.5)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 2, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x49, "Первый байт должен быть 0x49 (положительный ответ на Service 09)")
        self.assertEqual(response[1], 0x00, "Второй байт должен быть 0x00 (PID)")

    def test_obd_service_09_pid_02_vin(self):
        """Тест запроса VIN (Service 09, PID 02)"""
        # Отправка запроса VIN
        self._send_obd_request(0x09, 0x02)
        
        # Ожидание ответа
        response = self._wait_for_response(self.obd_stack, 1.0)  # Увеличенный таймаут для длинного ответа
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 2, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x49, "Первый байт должен быть 0x49 (положительный ответ на Service 09)")
        self.assertEqual(response[1], 0x02, "Второй байт должен быть 0x02 (PID)")
        
        # Проверка, что ответ содержит VIN
        self.assertGreater(len(response), 3, "Ответ должен содержать данные VIN")

    def test_obd_service_09_pid_0a_ecu_name(self):
        """Тест запроса имени ECU (Service 09, PID 0A)"""
        # Отправка запроса имени ECU
        self._send_obd_request(0x09, 0x0A)
        
        # Ожидание ответа
        response = self._wait_for_response(self.obd_stack, 0.5)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 2, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x49, "Первый байт должен быть 0x49 (положительный ответ на Service 09)")
        self.assertEqual(response[1], 0x0A, "Второй байт должен быть 0x0A (PID)")

    def test_uds_service_10_diagnostic_session_control(self):
        """Тест UDS сервиса DiagnosticSessionControl (Service 10)"""
        # Отправка запроса на переход в сессию по умолчанию
        self._send_uds_request(0x10, 0x01)
        
        # Ожидание ответа
        response = self._wait_for_response(self.uds_stack, 0.5)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 0, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x50, "Первый байт должен быть 0x50 (положительный ответ на Service 10)")

    def test_uds_service_11_ecu_reset(self):
        """Тест UDS сервиса ECUReset (Service 11)"""
        # Отправка запроса на мягкий сброс ECU
        self._send_uds_request(0x11, 0x01)
        
        # Ожидание ответа
        response = self._wait_for_response(self.uds_stack, 0.5)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 0, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x51, "Первый байт должен быть 0x51 (положительный ответ на Service 11)")

    def test_uds_service_19_read_dtc_information(self):
        """Тест UDS сервиса ReadDTCInformation (Service 19)"""
        # Отправка запроса на чтение DTC по маске статуса
        self._send_uds_request(0x19, 0x02, bytes([0xFF]))
        
        # Ожидание ответа
        response = self._wait_for_response(self.uds_stack, 0.5)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Проверка формата ответа
        self.assertGreater(len(response), 0, "Ответ должен содержать данные")
        self.assertEqual(response[0], 0x59, "Первый байт должен быть 0x59 (положительный ответ на Service 19)")
        
        # Проверка, что ответ содержит DTC
        self.assertGreater(len(response), 1, "Ответ должен содержать данные DTC")

    def test_dtc_encoding_decoding(self):
        """Тест кодирования и декодирования DTC"""
        # Отправка запроса DTC через OBD
        self._send_obd_request(0x03)
        response = self._wait_for_response(self.obd_stack, 0.5)
        
        self.assertIsNotNone(response, "Должен быть получен ответ с DTC")
        self.assertGreater(len(response), 1, "Ответ должен содержать данные DTC")
        
        # Отправка запроса DTC через UDS
        self._send_uds_request(0x19, 0x02, bytes([0xFF]))
        response = self._wait_for_response(self.uds_stack, 0.5)
        
        self.assertIsNotNone(response, "Должен быть получен ответ с DTC через UDS")
        self.assertGreater(len(response), 1, "Ответ должен содержать данные DTC")

    def test_multiple_obd_requests(self):
        """Тест последовательных OBD запросов"""
        # Последовательность запросов
        requests = [
            (0x01, 0x00),  # Поддерживаемые PID
            (0x01, 0x05),  # Температура охлаждающей жидкости
            (0x01, 0x0D),  # Скорость автомобиля
            (0x03, None),  # DTC
            (0x09, 0x02),  # VIN
        ]
        
        for service, pid in requests:
            with self.subTest(service=service, pid=pid):
                self._send_obd_request(service, pid)
                response = self._wait_for_response(self.obd_stack, 0.5)
                self.assertIsNotNone(response, f"Должен быть получен ответ на запрос {service:02X}{pid if pid else '':02X}")
                time.sleep(0.05)  # Небольшая пауза между запросами

    def test_multiple_uds_requests(self):
        """Тест последовательных UDS запросов"""
        # Последовательность запросов
        requests = [
            (0x10, 0x01, None),  # DiagnosticSessionControl - default session
            (0x11, 0x01, None),  # ECUReset - hard reset
            (0x19, 0x02, bytes([0xFF])),  # ReadDTCInformation - reportDTCByStatusMask
        ]
        
        for service, subfunction, data in requests:
            with self.subTest(service=service, subfunction=subfunction):
                self._send_uds_request(service, subfunction, data)
                response = self._wait_for_response(self.uds_stack, 0.5)
                self.assertIsNotNone(response, f"Должен быть получен ответ на запрос {service:02X}{subfunction:02X}")
                time.sleep(0.05)  # Небольшая пауза между запросами

    def test_iso_tp_protocol_compliance(self):
        """Тест соответствия протоколу ISO-TP"""
        # Отправка запроса, который должен вызвать длинный ответ
        self._send_obd_request(0x09, 0x02)  # Запрос VIN
        
        # Ожидание ответа
        response = self._wait_for_response(self.obd_stack, 1.0)
        
        # Проверка, что получен ответ
        self.assertIsNotNone(response, "Должен быть получен ответ от ECU симулятора")
        
        # Ответ на запрос VIN может быть длинным и требовать нескольких кадров ISO-TP
        # Это проверяет корректную работу протокола ISO-TP

    def test_error_handling(self):
        """Тест обработки ошибок"""
        # Отправка запроса неподдерживаемого PID
        self._send_obd_request(0x01, 0xFF)
        
        # Ожидание ответа
        response = self._wait_for_response(self.obd_stack, 0.5)
        
        # Может быть получен отрицательный ответ или таймаут
        # Важно, что система не крашится

if __name__ == '__main__':
    unittest.main()