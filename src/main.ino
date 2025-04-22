#include <Arduino.h>
#include "core/Debug.h"
#include "core/MainController.h"
#include <esp_task_wdt.h>

MainController controller;

void setup() {
    Serial.begin(115200);
    delay(1000); // Let serial settle
    Debug::log(Debug::INFO, "Booting...");

    controller.begin();
    Debug::log(Debug::INFO, "Controller ready");

    esp_task_wdt_init(8, true); // 8 second timeout
    esp_task_wdt_add(NULL);
}

void loop() {
    controller.update();
    delay(16); // ~60 FPS update rate
    esp_task_wdt_reset();
    yield();
}
