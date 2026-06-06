#made with AI
import struct

binary_file_path = "log__dev_ttyACM1_1.bin"
output_text_file_path = "data_restored.txt"

# Формат пакета данных (без маркеров): 4 числа float = 16 байт
packet_format = "<f fff fff f"
packet_size = struct.calcsize(packet_format)

# Ожидаемые маркеры в байтовом виде
MARKER_1 = b'\xaa'
MARKER_2 = b'\xbb'

count = 0
skipped_bytes = 0

with open(binary_file_path, "rb") as bin_file, open(output_text_file_path, "w", encoding="utf-8") as txt_file:
    while True:
        # 1. Ищем маркер начала пакета
        byte1 = bin_file.read(1)
        if not byte1:
            break # Конец файла
            
        if byte1 == MARKER_1:
            byte2 = bin_file.read(1)
            if byte2 == MARKER_2:
                # Маркеры 0xAA 0xBB успешно найдены! 
                # 2. Читаем следующие 16 байт самих данных
                chunk = bin_file.read(packet_size)
                
                if len(chunk) < packet_size:
                    print("Предупреждение: Файл оборвался на середине пакета данных.")
                    break
                
                # 3. Расшифровываем float'ы
                time,ax, ay, az, gx, gy, gz, temp = struct.unpack(packet_format, chunk)
                
                # Записываем в текст
                line = f"{time};{ax};{ay};{az};{gx};{gy};{gz};{temp}\n"
                txt_file.write(line)
                count += 1
            else:
                # Если первый байт совпал, а второй нет
                skipped_bytes += 1
        else:
            # Считанный байт не является частью маркера, идем дальше
            skipped_bytes += 1

print("Конвертация завершена!")
print(f"Успешно восстановлено строк: {count}")
if skipped_bytes > 0:
    print(f"Пропущено внесистемных байт (мусора): {skipped_bytes}")