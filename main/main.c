#include "include/myRC522.h"
#include "include/myServo.h"
#include "include/myNRF24.h"
#include "include/mySpeaker.h"
#include "driver/gptimer.h"

//******************************************FSM definition****************************************
// define state enum
typedef enum {
    STATE_INIT,
    STATE_NFC_READER,
    STATE_NFC_TO_PIANO,
    STATE_PIANO,
    STATE_PIANO_TO_LASER,
    STATE_LASER,
    STATE_LASER_TO_SIMON,
    STATE_SIMON,
    STATE_SIMON_TO_DISTANCE,
    STATE_DISTANCE,
    STATE_FINISH,
    STATE_STOP,
    NUM_STATES // 用于枚举计数，非实际状态
} State;

// define event enum
typedef enum {
    EVENT_INIT,
    EVENT_INIT_FINISHED,
    EVENT_NFC_READER_FINISHED,
    EVENT_NFC_TO_PIANO_FINISHED,
    EVENT_PIANO_FINISHED,
    EVENT_PIANO_TO_LASER_FINISHED,
    EVENT_LASER_FINISHED,
    EVENT_LASER_TO_SIMON_FINISHED,
    EVENT_SIMON_FINISHED,
    EVENT_SIMON_TO_DISTANCE_FINISHED,
    EVENT_DISTANCE_FINISHED,
    EVENT_FINISH_FINISHED,
    EVENT_RESTART,
    NUM_EVENTS // 用于枚举计数，非实际事件
} Event;

// state transfer definition
typedef State (*StateFunc)();

//state execution and return next state
State doInit(Event* event);
State doNFCReader(Event* event);
State doNFCToPiano(Event* event);
State doPiano(Event* event);
State doPianoToLaser(Event* event);
State doLaser(Event* event);
State doLaserToSimon(Event* event);
State doSimon(Event* event);
State doSimonToDistance(Event* event);
State doDistance(Event* event);
State doFinish(Event* event);
State doStop(Event* event);

//state  transfer table
StateFunc stateTable[NUM_STATES] = {
        doInit,
        doNFCReader,
        doNFCToPiano,
        doPiano,
        doPianoToLaser,
        doLaser,
        doLaserToSimon,
        doSimon,
        doSimonToDistance,
        doDistance,
        doFinish,
        doStop
};
//********************************************FSM DEFINITION END******************************************
NRF24_t dev;
gptimer_handle_t timer_handle;
rc522_handle_t scanner;

esp_err_t init();

void app_main(void) {
    State currentState = STATE_INIT;
    Event event = EVENT_INIT;

    while (currentState != STATE_STOP) {
        currentState = stateTable[currentState](&event);
    }
}

esp_err_t init(){
    esp_err_t ret;
    ret = rc522_power_down();
    if(ret != ESP_OK)   return ret;

    ret = nrf24_init(&dev);
    if(ret != ESP_OK)   return ret;

    ret = speaker_init(&timer_handle);
    if(ret != ESP_OK)   return ret;

    ret = rc522_init(&scanner);

    return ret;
}

//*******************************************FSM transfer function*****************************************
State doInit(Event* event) {
    printf("start init\n");
    if(*event != EVENT_INIT) return STATE_STOP;

    if(init() == ESP_OK){
        printf("done init\n");
        *event = EVENT_INIT_FINISHED;
        return STATE_NFC_READER;
    }
    printf("init fail\n");
    return STATE_INIT;
}

State doNFCReader(Event* event){
    printf("start NFC\n");
    if(*event != EVENT_INIT_FINISHED)  return STATE_STOP;

    if(myRC522_start(&scanner) == ESP_OK){
        *event = EVENT_NFC_READER_FINISHED;
        return STATE_NFC_TO_PIANO;
    }
    printf("done NFC\n");
    return STATE_NFC_READER;
}

State doNFCToPiano(Event* event){
    printf("start NFCToPiano\n");
    if(*event != EVENT_NFC_READER_FINISHED) return STATE_STOP;

    if(notifyPiano(&dev) == ESP_OK){
        *event = EVENT_NFC_TO_PIANO_FINISHED;
        printf("done NFCToPiano\n");
        return STATE_PIANO;
    }
    printf("fail NFCToPiano\n");
    return STATE_PIANO;
}

State doPiano(Event* event){
    if(*event != EVENT_NFC_TO_PIANO_FINISHED) return STATE_STOP;

    if(isPianoFinished(&timer_handle, &dev) == ESP_OK){
        *event = EVENT_PIANO_FINISHED;
        return STATE_PIANO_TO_LASER;
    }
    return STATE_PIANO_TO_LASER;
}

State doPianoToLaser(Event *event){
    if(*event != EVENT_PIANO_FINISHED)  return STATE_STOP;

    if(isReadyForLaser(&dev) == ESP_OK){
        *event = EVENT_PIANO_TO_LASER_FINISHED;
        return STATE_LASER;
    }
    return STATE_LASER;
}

State doLaser(Event* event){
    printf("Laser running...\n");
    return STATE_LASER_TO_SIMON;
}

State doLaserToSimon(Event* event){
    return STATE_SIMON;
}

State doSimon(Event* event){
    printf("Simon running...\n");
    return STATE_SIMON_TO_DISTANCE;
}

State doSimonToDistance(Event* event){
    return STATE_DISTANCE;
}

State doDistance(Event* event){
    printf("Distance running...\n");
    return STATE_FINISH;
}

State doFinish(Event* event){
    return STATE_STOP;
}

State doStop(Event* event){
    if(*event != EVENT_FINISH_FINISHED) return STATE_STOP;

    if(detectRestartTag(&scanner) == ESP_OK){
        *event = EVENT_RESTART;
        return STATE_NFC_READER;
    }
    return STATE_NFC_READER;
}