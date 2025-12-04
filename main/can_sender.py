#!/usr/bin/env python3

import socket
import struct
import time
import sys
import threading

class CANPacketSender:
    def __init__(self, interface, interval_ms):
        self.interface = interface
        self.interval = interval_ms / 1000.0  # Преобразование в секунды
        self.packet_counter = 0
        self.running = False
        
        # Создание CAN сокета
        try:
            self.sock = socket.socket(socket.PF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
            self.sock.bind((interface,))
        except Exception as e:
            print(f"Failed to create CAN socket: {e}")
            sys.exit(1)

    def send_packet(self):
        # Создание CAN кадра
        # Идентификатор кадра (можно изменить по необходимости)
        can_id = 0x123
        
        # Преобразование номера пакета в little-endian формат
        counter_bytes = struct.pack('<I', self.packet_counter)  # '<I' означает unsigned int в little-endian
        
        # Дополнительные данные (можно изменить по необходимости)
        additional_data = b'\xDE\xAD\xBE\xEF'
        
        # Сборка данных кадра (всего 8 байт)
        data = counter_bytes + additional_data
        
        # Создание кадра: идентификатор + данные
        # Формат кадра для SocketCAN: struct can_frame
        frame = struct.pack('=IB3x8s', can_id, len(data), data)
        
        try:
            self.sock.send(frame)
            print(f"Sent packet #{self.packet_counter}")
            self.packet_counter += 1
            return True
        except Exception as e:
            print(f"Failed to send packet: {e}")
            return False

    def start_sending(self):
        self.running = True
        while self.running:
            if not self.send_packet():
                break
            time.sleep(self.interval)

    def stop_sending(self):
        self.running = False

def main():
    if len(sys.argv) != 3:
        print("Usage: {} <can_interface> <interval_ms>".format(sys.argv[0]))
        print("Example: {} can0 100".format(sys.argv[0]))
        sys.exit(1)

    interface = sys.argv[1]
    try:
        interval_ms = int(sys.argv[2])
    except ValueError:
        print("Interval must be an integer")
        sys.exit(1)

    sender = CANPacketSender(interface, interval_ms)
    
    try:
        print(f"Starting CAN packet transmission on {interface} with {interval_ms}ms interval")
        print("Press Ctrl+C to stop")
        sender.start_sending()
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        sender.stop_sending()

if __name__ == "__main__":
    main()