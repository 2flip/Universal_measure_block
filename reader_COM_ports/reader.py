#made with AI
# -*- coding: utf-8 -*-
import os
import sys
import threading
import time
import serial

PORTS_TO_RECORD = ["/dev/ttyACM0", "/dev/ttyACM1"]
BAUD_RATE = 576000


def generate_unique_filename(port_name):
    clean_port_name = port_name.replace("/", "_").replace("\\", "_")
    base_name = f"log_{clean_port_name}"
    extension = ".bin"  # Поменял расширение на .bin, так как данные бинарные

    filename = f"{base_name}{extension}"

    counter = 1
    while os.path.exists(filename):
        filename = f"{base_name}_{counter}{extension}"
        counter += 1

    return filename


def record_port(port_name, baud_rate):
    filename = generate_unique_filename(port_name)
    print(f"[{port_name}] Started binary logging to: {filename}")

    try:
        with serial.Serial(port_name, baud_rate, timeout=1.0) as ser:
            # Режим 'wb' (запись бинарных данных). Буферизация отключена (buffering=0),
            # чтобы данные сразу уходили в файл при выдергивании питания
            with open(filename, "wb", buffering=0) as file:
                while True:
                    avail = ser.in_waiting
                    if avail > 0:
                        # Читаем ровно столько байт, сколько сейчас есть в буфере
                        raw_data = ser.read(avail)

                        file.write(raw_data)

                        # Принудительный сброс на флешку для защиты от потери данных
                        os.fsync(file.fileno())
                    else:
                        time.sleep(0.005)  # Чуть уменьшил спинлок для бинарников

    except serial.SerialException as e:
        print(f"[{port_name}] Error: {e}")
    except Exception as e:
        print(f"[{port_name}] Unexpected error: {e}")


def main():
    print("COM binary logger started.")
    print(f"Target ports: {PORTS_TO_RECORD}")
    print("-" * 50)

    threads = []
    for port in PORTS_TO_RECORD:
        thread = threading.Thread(
            target=record_port, args=(port, BAUD_RATE), daemon=True
        )
        threads.append(thread)
        thread.start()

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nStopped by user.")


if __name__ == "__main__":
    main()