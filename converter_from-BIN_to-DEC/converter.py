import struct
import csv

# Константы
PACKET_SIZE = 4096          # Общий размер одного пакета
TIME_SIZE = 256 * 4         # 1024 байта (256 * 4)
ACCEL_SIZE = 256 * 6        # 1536 байт (256 * 6)

def parse_packet(data):
    """
    Распарсить один пакет (4096 байт) в читаемую структуру
    
    Args:
        data: bytes объект размером 4096 байт
    
    Returns:
        dict: {
            'time': list of 256 unsigned long,
            'accel0': list of 256 tuples (ax, ay, az),
            'accel1': list of 256 tuples (ax, ay, az)
        }
    """
    
    if len(data) != PACKET_SIZE:
        raise ValueError(f"Неверный размер пакета: {len(data)} байт (должно быть {PACKET_SIZE})")
    
    # 1. Читаем time[256] (unsigned long, 4 байта, little-endian)
    time_data = data[0:TIME_SIZE]
    times = struct.unpack(f'<{256}I', time_data)  # 'I' = unsigned int (32 bit)
    
    # 2. Читаем a_massiv_0[256] (каждый по 3 int16 = 6 байт)
    accel0_data = data[TIME_SIZE:TIME_SIZE + ACCEL_SIZE]
    accel0_raw = struct.unpack(f'<{768}h', accel0_data)  # 256*3 = 768 int16
    # Преобразуем в список кортежей (ax, ay, az)
    accel0 = [(accel0_raw[i], accel0_raw[i+1], accel0_raw[i+2]) 
              for i in range(0, len(accel0_raw), 3)]
    
    # 3. Читаем a_massiv_1[256] (каждый по 3 int16 = 6 байт)
    accel1_data = data[TIME_SIZE + ACCEL_SIZE:]
    accel1_raw = struct.unpack(f'<{768}h', accel1_data)  # 256*3 = 768 int16
    # Преобразуем в список кортежей (ax, ay, az)
    accel1 = [(accel1_raw[i], accel1_raw[i+1], accel1_raw[i+2]) 
              for i in range(0, len(accel1_raw), 3)]
    
    return {
        'time': times,
        'accel0': accel0,
        'accel1': accel1
    }

def read_bin_file(filename, max_packets=None):
    """
    Читает бинарный файл с несколькими пакетами
    
    Args:
        filename: путь к файлу
        max_packets: максимальное количество пакетов для чтения (None = все)
    
    Returns:
        list of packets
    """
    packets = []
    
    with open(filename, 'rb') as f:
        packet_num = 0
        
        while True:
            data = f.read(PACKET_SIZE)
            if len(data) != PACKET_SIZE:
                break  # Конец файла
            
            print(f"\n{'='*60}")
            print(f"Пакет #{packet_num}")
            print(f"{'='*60}")
            
            # Парсим пакет
            parsed = parse_packet(data)
            packets.append(parsed)
            
            # Выводим первые 5 элементов для проверки
            print(f"Первые 5 времен: {parsed['time'][:5]}")
            print(f"Первые 5 accel0 (ax,ay,az): {parsed['accel0'][:5]}")
            print(f"Первые 5 accel1 (ax,ay,az): {parsed['accel1'][:5]}")
            
            # Статистика по пакету
            print(f"\nСтатистика:")
            print(f"  Диапазон времени: {min(parsed['time'])} - {max(parsed['time'])}")
            
            # Средние значения акселерометров
            avg_ax0 = sum(a[0] for a in parsed['accel0']) / 256
            avg_ay0 = sum(a[1] for a in parsed['accel0']) / 256
            avg_az0 = sum(a[2] for a in parsed['accel0']) / 256
            print(f"  Среднее accel0: ax={avg_ax0:.1f}, ay={avg_ay0:.1f}, az={avg_az0:.1f}")
            
            packet_num += 1
            
            if max_packets and packet_num >= max_packets:
                break
    
    print(f"\n{'='*60}")
    print(f"Всего прочитано пакетов: {packet_num}")
    print(f"{'='*60}")
    
    return packets

def export_to_csv(packets, output_file):
    """
    Экспортирует данные в CSV файл для анализа в Excel
    
    Формат: packet_num, index, time, ax0, ay0, az0, ax1, ay1, az1
    """
    
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        
        # Заголовки
        writer.writerow(['packet_num', 'index', 'time', 
                        'ax0', 'ay0', 'az0', 
                        'ax1', 'ay1', 'az1'])
        
        # Записываем данные
        for p_num, packet in enumerate(packets):
            for i in range(256):
                writer.writerow([
                    p_num,
                    i,
                    packet['time'][i],
                    packet['accel0'][i][0],
                    packet['accel0'][i][1],
                    packet['accel0'][i][2],
                    packet['accel1'][i][0],
                    packet['accel1'][i][1],
                    packet['accel1'][i][2]
                ])
    
    print(f"Экспортировано в {output_file}")

def export_statistics(packets, output_file):
    """
    Экспортирует статистику по каждому пакету
    """
    
    with open(output_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(['packet_num', 'min_time', 'max_time', 'time_range',
                        'avg_ax0', 'avg_ay0', 'avg_az0',
                        'avg_ax1', 'avg_ay1', 'avg_az1'])
        
        for p_num, packet in enumerate(packets):
            times = packet['time']
            accel0 = packet['accel0']
            accel1 = packet['accel1']
            
            writer.writerow([
                p_num,
                min(times),
                max(times),
                max(times) - min(times),
                sum(a[0] for a in accel0) / 256,
                sum(a[1] for a in accel0) / 256,
                sum(a[2] for a in accel0) / 256,
                sum(a[0] for a in accel1) / 256,
                sum(a[1] for a in accel1) / 256,
                sum(a[2] for a in accel1) / 256,
            ])
    
    print(f"Статистика экспортирована в {output_file}")

def quick_view(packets, num_elements=10):
    """
    Быстрый просмотр данных в консоли
    """
    
    print("\n" + "="*80)
    print("ПЕРВЫЕ НЕСКОЛЬКО ЗНАЧЕНИЙ:")
    print("="*80)
    
    for p_num, packet in enumerate(packets[:3]):  # Первые 3 пакета
        print(f"\nПакет {p_num}:")
        print(f"{'Index':>5} {'Time':>10} {'ax0':>6} {'ay0':>6} {'az0':>6} {'ax1':>6} {'ay1':>6} {'az1':>6}")
        print("-" * 65)
        
        for i in range(min(num_elements, 256)):
            print(f"{i:5d} {packet['time'][i]:10d} "
                  f"{packet['accel0'][i][0]:6d} {packet['accel0'][i][1]:6d} {packet['accel0'][i][2]:6d} "
                  f"{packet['accel1'][i][0]:6d} {packet['accel1'][i][1]:6d} {packet['accel1'][i][2]:6d}")

# ========== ИСПОЛЬЗОВАНИЕ ==========

if __name__ == "__main__":
    # Читаем бинарный файл
    packets = read_bin_file('data.bin', max_packets=5)  # Читаем первые 5 пакетов
    
    # Быстрый просмотр в консоли
    quick_view(packets, num_elements=10)
    
    # Экспорт в CSV для Excel
    export_to_csv(packets, 'sensor_data.csv')
    
    # Экспорт статистики
    export_statistics(packets, 'statistics.csv')
    
    # Проверка целостности
    print(f"\nПроверка:")
    print(f"  Размер одного пакета: {PACKET_SIZE} байт")
    print(f"  Ожидаемый размер: 4096 байт")
    print(f"  Прочитано пакетов: {len(packets)}")