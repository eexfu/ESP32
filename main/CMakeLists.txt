idf_component_register(SRCS
        "src/myRC522.c"
        "src/myServo.c"
        "src/myNRF24.c"
        "src/mySpeaker.c"
        "src/mySPIFFS.c"
        "src/myIRSensor.c"
        "src/myElectromanget.c"
        "src/myLaser.c"
        "main.c"
        INCLUDE_DIRS "."
        "./include")
spiffs_create_partition_image(storage ../res FLASH_IN_PROJECT)