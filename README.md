# M-Box

### Introduction
**Modular Smart Medicine Box**  
Prototype designed in the Da-Vinci Summer Camp.

### Hardware
- MCU: STM32F103C8T6
- Bluetooth module: ATK-HC05
- LEDs, speeker, push switch, battery...

### Software
- The server M-box connects to the clients through the ATK-HC05 bluetooth module and broadcasts the medicine plan to the clients.
- The client M-boxes will remind the elderly which kind of medicine and how much to take by the number and color of the light along with the sound.
- The client boxes will also record whether the medicine is taken and transmit the imfomation to the server box.
- The server M-box can connect to mobile phone through bluetooth and the medicine plan can be set on the phone.
- The medicine plan and the record of medicine taking collected by the server box can be send to the phone and uploaded to the cloud server we rent and built so that the information can be share with family members(available before 2020/8/1).


### Others
Slides for presentation: M-BOX.pdf