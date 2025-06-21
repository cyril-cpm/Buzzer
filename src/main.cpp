#include <Arduino.h>
#include "Settingator.h"
#include "CustomType.hpp"
#include "ESPNowCommunicator.h"
#include "FastLED.h"
#include "Preferences.h"

Preferences prefs;

#define LED_PIN         4
#define NUM_LEDS        31

CRGB leds[NUM_LEDS];

#define BUTTON_PIN      22

Settingator STR(nullptr);

#define BROADCAST_BTN_PIN   32

#define BUTTON_NOTIF       5

STR_UInt8 r(0, "RED");
STR_UInt8 g(0, "GREEN");
STR_UInt8 b(255, "BLUE");

char label[4] = { 'N', 'U', 'L', '\0'};

void setup() {
    prefs.begin("buzzer");
    prefs.getString("LBL", label, 3);

    Serial.begin(115200);
    
    espNowCore = ESPNowCore::CreateInstance();
    if (espNowCore)
        espNowCore->BroadcastPing(); 

    STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "UPDATE_LED", [](){
        for (auto i =0; i < NUM_LEDS; i++)
        leds[i] = CRGB(r, g , b);
    });

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    attachInterrupt(BUTTON_PIN, [](){STR.SendNotif(BUTTON_NOTIF);}, RISING);

    FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);

    pinMode(BROADCAST_BTN_PIN, INPUT_PULLDOWN);
    attachInterrupt(BROADCAST_BTN_PIN, [](){espNowCore->BroadcastPing(); Serial.println("Broad");}, RISING); 

    for (auto i =0; i < NUM_LEDS; i++)
        leds[i] = CRGB(r, g , b);

}

void loop() {
    
    FastLED.show();
    STR.Update();
}