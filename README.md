# EShw4


### How to run

##### STM32L4
* 可以直接將整個project載下，用 mbed studio 將程式燒錄在 STM32L4開發板上。
* 將 STM32L4 接電後即可以連到藍芽。

##### Raspberry Pi
* 用 Raspberry Pi 跑 ble_client.py

### Main Functions
* STM32L4 Write/Read Messages
* Client Write/Read Messages
* Client (BLE Central) control the LEDs on STM32L4
* Client monitor the Button state, notified when button pressed.

##### Control LEDs
* 功能：”LED1 on”, “LED1 off”, “LED2 on”, “LED2 off”
* 當成功啟用功能，會印出”turning on/off LEDx...”
* 如果輸入功能列表以外的功能，則印出”Wrong Input!”
* 輸入”exit”，系統印出”Bye~!”，則與STM32L4斷連，結束program。

##### Monitor button state
* 當STM32L4上的藍色按鈕被按下，則1會被寫入Button Characteristic。
* 當program讀取到Button State為1，則印出”Button pressed.”!

### Demo Result
![image](https://user-images.githubusercontent.com/71332212/140938741-c43944f4-a6f3-4ce3-a589-e078a8e859e1.png)

