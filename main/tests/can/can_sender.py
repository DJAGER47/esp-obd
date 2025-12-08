#!/usr/bin/env python3

import socket
import struct
import time
import sys
import threading
import select

class CANPacketSender:
    def __init__(self, interface, interval_ms):
        self.interface = interface
        self.interval = interval_ms / 10000.0  # Преобразование в секунды
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

class CANPacketReceiver:
    def __init__(self, interface):
        self.interface = interface
        self.running = False
        self.received_packets = {}
        self.expected_counter = 0
        
        # Создание CAN сокета для приёма
        try:
            self.sock = socket.socket(socket.PF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
            self.sock.bind((interface,))
        except Exception as e:
            print(f"Failed to create CAN receiver socket: {e}")
            sys.exit(1)
    
    def receive_packet(self):
        """Принимает одно CAN сообщение и проверяет его индекс"""
        try:
            # Используем select для проверки наличия данных
            ready, _, _ = select.select([self.sock], [], [], 0.1)
            if ready:
                frame, _ = self.sock.recvfrom(16)
                
                # Распаковка CAN кадра
                can_id, dlc, data = struct.unpack('=IB3x8s', frame)
                
                # Фильтруем только сообщения с ID 0x321
                if can_id != 0x321:
                    return False
                
                # Извлечение счётчика из первых 4 байт данных
                counter = struct.unpack('<I', data[:4])[0]
                
                # Извлечение дополнительных данных
                additional_data = data[4:]
                
                print(f"Received packet #{counter} with ID 0x{can_id:03X}")
                
                # Проверка индекса
                if counter in self.received_packets:
                    print(f"WARNING: Duplicate packet #{counter}")
                else:
                    self.received_packets[counter] = {
                        'timestamp': time.time(),
                        'can_id': can_id,
                        'data': additional_data
                    }
                
                # Проверка последовательности
                if counter == self.expected_counter:
                    print(f"OK: Expected packet #{counter}")
                    self.expected_counter += 1
                elif counter > self.expected_counter:
                    missing = counter - self.expected_counter
                    print(f"WARNING: Missing {missing} packet(s) ({self.expected_counter} to {counter-1})")
                    self.expected_counter = counter + 1
                else:
                    print(f"INFO: Out-of-order packet #{counter} (expected {self.expected_counter})")
                
                return True
        except socket.timeout:
            return False
        except Exception as e:
            print(f"Error receiving packet: {e}")
            return False
    
    def start_receiving(self):
        """Запускает поток приёма сообщений"""
        self.running = True
        print(f"Starting CAN packet receiver on {self.interface}")
        
        while self.running:
            self.receive_packet()
    
    def stop_receiving(self):
        """Останавливает поток приёма сообщений"""
        self.running = False
        print(f"Received {len(self.received_packets)} unique packets")
    
    def get_statistics(self):
        """Возвращает статистику по принятым пакетам"""
        if not self.received_packets:
            return "No packets received"
        
        total = len(self.received_packets)
        min_counter = min(self.received_packets.keys())
        max_counter = max(self.received_packets.keys())
        
        # Проверка пропущенных пакетов
        missing = []
        for i in range(min_counter, max_counter + 1):
            if i not in self.received_packets:
                missing.append(i)
        
        stats = f"Total packets: {total}\n"
        stats += f"Counter range: {min_counter} to {max_counter}\n"
        stats += f"Missing packets: {len(missing)}\n"
        
        if missing and len(missing) <= 10:
            stats += f"Missing indices: {missing}\n"
        elif missing:
            stats += f"First missing: {missing[:5]}...\n"
        
        return stats

        return stats

def main():
    if len(sys.argv) < 3 or len(sys.argv) > 4:
        print("Usage: {} <can_interface> <interval_ms> [mode]".format(sys.argv[0]))
        print("Modes:")
        print("  send    - Only send packets (default)")
        print("  recv    - Only receive packets")
        print("  both    - Send and receive packets simultaneously")
        print("Example: {} can0 100 both".format(sys.argv[0]))
        sys.exit(1)

    interface = sys.argv[1]
    try:
        interval_ms = int(sys.argv[2])
    except ValueError:
        print("Interval must be an integer")
        sys.exit(1)
    
    mode = sys.argv[3] if len(sys.argv) == 4 else "send"
    
    if mode not in ["send", "recv", "both"]:
        print("Invalid mode. Use 'send', 'recv', or 'both'")
        sys.exit(1)
    
    sender = None
    receiver = None
    sender_thread = None
    receiver_thread = None
    
    try:
        if mode in ["send", "both"]:
            sender = CANPacketSender(interface, interval_ms)
            sender_thread = threading.Thread(target=sender.start_sending)
            sender_thread.daemon = True
        
        if mode in ["recv", "both"]:
            receiver = CANPacketReceiver(interface)
            receiver_thread = threading.Thread(target=receiver.start_receiving)
            receiver_thread.daemon = True
        
        # Запуск потоков
        if sender_thread:
            print(f"Starting CAN packet transmission on {interface} with {interval_ms}ms interval")
            sender_thread.start()
        
        if receiver_thread:
            receiver_thread.start()
        
        print("Press Ctrl+C to stop")
        
        # Основной цикл ожидания
        while True:
            time.sleep(1)
            
            # Периодический вывод статистики приёма
            if receiver and len(receiver.received_packets) > 0:
                if len(receiver.received_packets) % 10 == 0:  # Каждые 10 пакетов
                    print("\n--- Reception Statistics ---")
                    print(receiver.get_statistics())
                    print("---------------------------\n")
    
    except KeyboardInterrupt:
        print("\nStopping...")
        
        # Остановка потоков
        if sender:
            sender.stop_sending()
        
        if receiver:
            receiver.stop_receiving()
            
        # Ожидание завершения потоков
        if sender_thread:
            sender_thread.join(timeout=2)
        
        if receiver_thread:
            receiver_thread.join(timeout=2)
        
        # Вывод финальной статистики
        if receiver:
            print("\n--- Final Reception Statistics ---")
            print(receiver.get_statistics())
            print("----------------------------------\n")

if __name__ == "__main__":
    main()