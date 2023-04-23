# 16ServoControl V 1.0 

 16 Servo Controler using PCA9685 , 74hc4067 mux and SSD1306 Oled display

Remark : be careful with the power supply, with an insufficient power the voltage can drop and cause a blocking or a reset of the arduino. A solution is to use two separate power supplies for the servos and for the arduino<br>

All function running !

**Reccord :**<br>
when changing of line Servo locked to stored value,<br>
need to move pot to reccorded value to unlock<br>
after unlocking servo can move according pot pos.<br><br>
change line using left and right button<br>
save by long press on Sel Btn<br>
number of step given by exit line number<br>
<br>

**Setup function :**<br>
 L button to set min Value<br>
 R Button to set max Value<br>
 Long Press L reset to min to MIN_US<br>
 Long Press R reset to max to MAX_US<br>
 Long press to select Save into Eeprom<br>
<br>
Add Visualisation in Live mode<br>
 last pot moved chanel & value displayed 


![IMG_20230416_230849](https://user-images.githubusercontent.com/30392727/232344117-aaa8cbbe-3665-4a22-98f2-86bddc6b18de.jpg)
