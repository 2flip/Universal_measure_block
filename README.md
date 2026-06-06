UMB - Universal measure block (Универсальный блок измерений). На данный момент реализовано получение всех типов данных с IMU ICM45686 с частотой 800 Гц. (Возможно получить 3,2 кГц, однако надо изменить способ отправки данных на пакетный)

ESP32C3 общается с ICM45686 по I2C на 400кГц, и затем данные отправляются через COM порт, который слушает BTT Pi.

Репозиторий содержит код для работы микроконтроллера ESP32С3 с ICM45686 с дальнейшей отправки их через USB CDC в бинарном виде.

Сделано через Platformio.

This project uses the motion.arduino.ICM45686 [link](https://github.com/tdk-invn-oss/motion.arduino.ICM45686)
which is distributed under the BSD 3-Clause License:
Copyright (c) [2026] [2flip]


![](./files/UMB_boards.jpg)
![](./files/UMB.jpg)


