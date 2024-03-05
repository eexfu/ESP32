# ESP32

You can edit the fsm and your init code in main.c
If your want to control your sensor or actuator, you can write your code down in src file and call it in main.c file.

main.c: entry of the app and the fsm
include: where the header in
src: where the source code in
component: where the libs in

Tips:

1. You may not need to set the python path in the CMakeList under ESP_EE3 folder, if you want to use it change it to your own path
2. If you want to write your own code in include and src folder, please add your c file in the CMakeList file under the main folder.
3. If you want to add your component, your can add it on your own Espressif folder(for example: C:\Tools\Espressif\frameworks\esp-idf-v5.1.2\components for me). Please also add it to the component folder, so that we can run the program in our own computer.

The basic struct of this project is:

|component

|main

|-- main.c

|-- include

|  |-- myNRF24.h

|  |-- myRC522.h

|  |-- myServo.h

|-- src

|  |-- myNRF24.c

|  |-- myRC522.c

|  |-- myServo.c