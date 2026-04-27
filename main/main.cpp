#include "Settingator.h"
#include "STR.h"
#include "Led.h"
#include "CustomType.hpp"

#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_PIN      GPIO_NUM_22

bool buttonPressed = false;

#define BUTTON_NOTIF       5

#define DEBOUNCE_TIME_MS 250

esp_timer_handle_t debounceTimerButton;

void debounceTimerButtonCallback(void*)
{
    gpio_intr_enable(BUTTON_PIN);
}

static void IRAM_ATTR buttonInterruptHandler(void* arg)
{
    gpio_intr_disable(BUTTON_PIN);
	buttonPressed = true;

    esp_timer_start_once(debounceTimerButton, DEBOUNCE_TIME_MS * 1000);
}

Settingator& STR = Settingator::GetInstance();

STR_UInt8 r(0, "RED"); 			
STR_UInt8 g(0, "GREEN");
STR_UInt8 b(0, "BLUE");

Led& LED = Led::GetInstance();

extern "C" void app_main(void)
{
	// TIMER //
    esp_timer_create_args_t timer_args = {
        .callback = debounceTimerButtonCallback,
        .name = "debounce_timer_broadcast"
    };
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &debounceTimerButton));

    // GPIO CONFIG //
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = 1ULL << BUTTON_PIN;
    ESP_ERROR_CHECK(gpio_config(&io_conf));


    ESP_ERROR_CHECK(gpio_install_isr_service(0));

    ESP_ERROR_CHECK(gpio_isr_handler_add(BUTTON_PIN, buttonInterruptHandler, (void*)BUTTON_PIN));
    ESP_ERROR_CHECK(gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE));
	
	InitCores();
	STR.begin();
	STR.ESPNowBroadcastPing();

	STR.AddSetting(Setting::Type::Trigger, nullptr, 0, "UPDATE_LED", []() {
				ESP_LOGI("MAIN", "UPDATE_LED");
				auto& data = LED.Strip2();

				for (auto& rgb : data)
				{
					rgb.r = r;
					rgb.b = b;
					rgb.g = g;
				}
				LED.Show();
			});

	while (true)
	{
		STR.Update();
		if (buttonPressed)
		{
			buttonPressed = false;
			STR.SendNotif(BUTTON_NOTIF);
		}
	}
}
