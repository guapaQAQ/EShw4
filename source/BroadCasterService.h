#include <cstdint>
class BroadcasterService
{
public:
    /**
    * @param[ref] _ble
    *
    * @param[in]
    */
    
    const static uint16_t BROADCAST_SERVICE_UUID            = 0xa004;
    const static uint16_t BROADCAST_CHARACTERISTIC_UUID     = 0xa005;
    // const static uint16_t BROADCAST_WRITE_UUID     = 0xa006;
    
    BroadcasterService(BLEDevice &_ble) :
        ble(_ble),
        
        broadcasterCharacteristic(BROADCAST_CHARACTERISTIC_UUID, command, sizeof(command), sizeof(command),
                                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY) {
    
        
            static bool serviceAdded = false; /* We should only ever need to add the service once. */
            if (serviceAdded) {
                return;
            }
    
            GattCharacteristic *charTable[] = {&broadcasterCharacteristic};
            GattService         broadcasterService(BroadcasterService::BROADCAST_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
    
            ble.addService(broadcasterService);
            serviceAdded = true;
        }
    
    /**
     * @brief
     *
     * @param
     */
    GattAttribute::Handle_t getValueHandle() const
    {
        return broadcasterCharacteristic.getValueHandle();
    }
    void sendHelloCommand() {
        uint8_t _command[70] = "Hi! We're B07901034, B07901157, B07901112! Nice to meet you~!";
        printf("Sent: \"%s\"\n", _command);
        ble.updateCharacteristicValue(broadcasterCharacteristic.getValueAttribute().getHandle(), _command, sizeof(_command), false);
    }
    void sendLEDCommand() {
        uint8_t _command[70] = "You can control leds: e.g.\"LED1 on\" \"LED1 off\". There are 2 leds.";
        printf("Sent: \"%s\"\n", _command);
        ble.updateCharacteristicValue(broadcasterCharacteristic.getValueAttribute().getHandle(), _command, sizeof(_command), false);
    }
    void sendBUTTONCommand() {
        uint8_t _command[70] = "You will receive message when button is pressed.";
        printf("Sent: \"%s\"\n", _command);
        ble.updateCharacteristicValue(broadcasterCharacteristic.getValueAttribute().getHandle(), _command, sizeof(_command), false);
    }
 
private:
    BLEDevice           &ble;
    uint8_t           command[70];    
    GattCharacteristic  broadcasterCharacteristic;

    
};