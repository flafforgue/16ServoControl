# 16ServoControl V 1.03 

 16 Servos Controler using PCA9685 , 74hc4067 mux and SSD1306 Oled display

Remark : be careful with the power supply, with an insufficient power the voltage can drop and cause a blocking or a reset of the arduino. A solution is to use two separate power supplies for the servos and for the arduino.<br>

All function running !

**Live:**<br>
Turn pot to move servo in real time.<br>
by default pulse between 600 and 2400 us , can be configured in setup for each channel.<br>
display pulse in us of last channel moved.<br>
<br>

**Reccord :**<br>
Turn pots for setting line, then press right button for next line.<br>
maximum 10 lines can be reccorded. I think nano have enought memory for 12 but not tested.<br>
when changing of line Servo locked to stored value,<br>
need to move pot to reccorded value to unlock,<br>
after unlocking servo can move according pot pos.<br>
change line using left and right button.<br>
save by long press on Sel Btn.<br>
number of step given by exit line number.<br>
<br>

**Play:**<br>
By default running in loop, exit by pressing "Sel" button. may be configurated by compilation option.<br>
Speed is not a real speed , but time in us between to steps. by default fadding between lines use 10 steps.<br>
<br>

**Setup function :**<br>
Set minimum and maximum pulse with <br>
maximal range is 500 to 2500 us.<br>
By default live update of servo, it is reccomended to put pots in middle before entering in setup<br>
Extreme range can damadge servos ( 500-800) ( 2200-2500).<br> 
Real time servo updating may be removed by software compilation option.<br>
 L button to set min Value<br>
 R Button to set max Value<br>
 Long Press L reset to min to MIN_SAFE_US<br>
 Long Press R reset to max to MAX_SAFE_US<br>
 Long Press to select Save into Eeprom<br>
 Long Press to Rotary Encoder to Invert Channel<br>
<br>

**History**<br>
V 1.03 add servo inversion & disable CONFIGMOVESERVO<br>
V 1.02 add Copy previous step in reccord mode & default set to 127<br>
V 1.01 added MIN_SAFE_US & MAX_SAFE_US for Servos initialisation. by default 600 and 2400 us<br>
v 1.00 First fully running version.<br>

![IMG_20230416_230849](https://user-images.githubusercontent.com/30392727/232344117-aaa8cbbe-3665-4a22-98f2-86bddc6b18de.jpg)
