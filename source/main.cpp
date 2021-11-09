/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <cstdint>
#include <events/mbed_events.h>

#include <mbed.h>
#include "PinNames.h"
#include "ThisThread.h"
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "LEDService.h"
#include "ButtonService.h"
#include "BroadCasterService.h"
#include "mbed_power_mgmt.h"
#include "pretty_printer.h"

const static char DEVICE_NAME[] = "ButtonBeauty";

static EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);


class BatteryDemo : ble::Gap::EventHandler {
public:
    BatteryDemo(BLE &ble, events::EventQueue &event_queue) :
        _ble(ble),
        _event_queue(event_queue),
        _led1(LED1, 1),
        _button(BLE_BUTTON_PIN_NAME, BLE_BUTTON_PIN_PULL),
        _button_service(NULL),
        _button_uuid(ButtonService::BUTTON_SERVICE_UUID),
        _actuated_led1(LED2, 0),
        _actuated_led2(LED3, 0),
        _led_uuid(LEDService::LED_SERVICE_UUID),
        _led_service(NULL),
        _broadcaster_uuid(BroadcasterService::BROADCAST_SERVICE_UUID),
        _broadcaster_service(NULL),
        _adv_data_builder(_adv_buffer) { }

    void start() {
        _ble.gap().setEventHandler(this);

        _ble.init(this, &BatteryDemo::on_init_complete);

        _event_queue.call_every(500, this, &BatteryDemo::blink);

        _event_queue.dispatch_forever();
    }

private:
    /** Callback triggered when the ble initialization process has finished */
    void on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
        if (params->error != BLE_ERROR_NONE) {
            printf("Ble initialization failed.");
            return;
        }


        /* Setup primary service. */

        _button_service = new ButtonService(_ble, false /* initial value for button pressed */);

        _button.fall(Callback<void()>(this, &BatteryDemo::button_pressed));
        _button.rise(Callback<void()>(this, &BatteryDemo::button_released));

        _led_service = new LEDService(_ble, false);

        _ble.gattServer().onDataWritten(this, &BatteryDemo::on_data_written);

        _broadcaster_service = new BroadcasterService(_ble);

        _ble.gattServer().onDataWritten(this, &BatteryDemo::on_data_written_broad);

        print_mac_address();

        start_advertising();

    }

    void start_advertising() {
        /* Create advertising parameters and payload */

        ble::AdvertisingParameters adv_parameters(
            ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
            ble::adv_interval_t(ble::millisecond_t(1000))
        );

        _adv_data_builder.setFlags();
        _adv_data_builder.setLocalServiceList(mbed::make_Span(&_button_uuid, 1));
        _adv_data_builder.setName(DEVICE_NAME);

        /* Setup advertising */

        ble_error_t error = _ble.gap().setAdvertisingParameters(
            ble::LEGACY_ADVERTISING_HANDLE,
            adv_parameters
        );

        if (error) {
            print_error(error, "_ble.gap().setAdvertisingParameters() failed");
            return;
        }

        error = _ble.gap().setAdvertisingPayload(
            ble::LEGACY_ADVERTISING_HANDLE,
            _adv_data_builder.getAdvertisingData()
        );

        if (error) {
            print_error(error, "_ble.gap().setAdvertisingPayload() failed");
            return;
        }

        /* Start advertising */

        error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

        if (error) {
            print_error(error, "_ble.gap().startAdvertising() failed");
            return;
        }

    }


    void button_pressed(void) {
        _event_queue.call(Callback<void(bool)>(_button_service, &ButtonService::updateButtonState), true);
    }

    void button_released(void) {
        _event_queue.call(Callback<void(bool)>(_button_service, &ButtonService::updateButtonState), false);
    }

    void on_data_written(const GattWriteCallbackParams *params) {
        if ((params->handle == _led_service->getValueHandle1()) && (params->len == 1)) {
            _actuated_led1 = *(params->data);
            printf("LED1 changed to %d\n", *(params->data));
        }
        if ((params->handle == _led_service->getValueHandle2()) && (params->len == 1)) {
            _actuated_led2 = *(params->data);
            printf("LED2 changed to %d\n", *(params->data));
        }
    }

    void on_data_written_broad(const GattWriteCallbackParams *params) {
        // printf("%p\n", params);
        if ((params->handle == _broadcaster_service->getValueHandle())) {
            // printf("Readed\n");
            printf("Received: \"%s\"\n", params->data);
            if ((params->data[0]) == 'H' && (params->data[1]) == 'e')
                _event_queue.call(Callback<void()>(_broadcaster_service, &BroadcasterService::sendHelloCommand));
            else if ((params->data[0]) == 'H' && (params->data[1]) == 'o')
                _event_queue.call(Callback<void()>(_broadcaster_service, &BroadcasterService::sendLEDCommand));
            else if ((params->data[0]) == 'W' && (params->data[1]) == 'h')
                _event_queue.call(Callback<void()>(_broadcaster_service, &BroadcasterService::sendBUTTONCommand));
        }

        // _event_queue.call(Callback<void()>(_broadcaster_service, &BroadcasterService::sendHelloCommand));
        // uint8_t a[50] = "Hi! we're b07901034, b08901112, b07901157.";
        // printf("%s\n", a);
        // _event_queue.call(Callback<void(uint8_t)>(_broadcaster_service, &BroadcasterService::sendCommand), a);
        // for (uint8_t c: a){
        //     printf("%c", c);
        //     ThisThread::sleep_for(600);
        //     _event_queue.call(Callback<void(uint8_t)>(_broadcaster_service, &BroadcasterService::sendCommand), c);
        //     // ThisThread::sleep_for(100);
        // }
        // ThisThread::sleep_for(5000);
        // _event_queue.call(Callback<void()>(_broadcaster_service, &BroadcasterService::sendHelloCommand));
        // ThisThread::sleep_for(5000);
        // _event_queue.call(Callback<void(uint8_t)>(_broadcaster_service, &BroadcasterService::sendCommand), a[1]);
        // printf("%s", "aaa");
        // ThisThread::sleep_for(5000);

        // for (int i=0;i<20;i++){
        //     uint8_t c = a[i];
        //     printf("%c", c);
        //     _event_queue.call(Callback<void(uint8_t)>(_broadcaster_service, &BroadcasterService::sendCommand), c);
        // }
    }


    void blink(void) {
        _led1 = !_led1;
    }

private:
    /* Event handler */

    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&) {
        _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    }

private:
    BLE &_ble;
    events::EventQueue &_event_queue;

    DigitalOut  _led1;
    InterruptIn _button;
    ButtonService *_button_service;

    UUID _button_uuid;

    DigitalOut _actuated_led1, _actuated_led2;

    UUID _led_uuid;
    LEDService *_led_service;

    UUID _broadcaster_uuid;
    BroadcasterService *_broadcaster_service;

    uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    ble::AdvertisingDataBuilder _adv_data_builder;
};

/** Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
    event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

int main()
{
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);

    BatteryDemo demo(ble, event_queue);
    demo.start();

    return 0;
}

