# List of changes for rev. b 

> Following list of changes has been compiled by Maurice Behague and is to be applied to the inputs for LMX3 project as sent by Jean-Christophe.

## List of changes

List of changes:

1. Add connection to the camera TBD cable design -> Molex as on the camera module to keep an Off-the-shelf cable 
2. LEDs connector pinout : all 4 must have the same -> to be adapted on mother board  - OK
   (Changed LED_INTF1 and LED_INTF4 pin order)
3.  Supply for UV LEDs : 8.5 v limit the current, Boost converter to replace to reach 9.5 v - 
4.  Capacitor on reset : none as the button was only for the development, but if the line stays, should be added. 
5. Visible LEDs are too powerful (dynamic is not appropriate, it is deem at 1% currently). Check the resistor values in place, propose alternative one. 

## Kickstart mail:

> Mail sent by Maurice on 16/07, [LMX3] changes on PCBA running
>
> *Hello Jean-Christophe,
>
>  
>
> Hope you are fine, to let you know on LumediX, we are implementing, based on your Rev3.1 Altium files, the following modifications :
>
>  
>
> Main board :
>
> \1.   Add connection to the camera TBD cable design -> Molex as on the camera module to keep an Off-the-shelf cable 
>
> \2.   LEDs connector pinout : all 4 must have the same -> to be adapted on mother board 
>
> \3.   Supply for UV LEDs : 8.5 v limit the current, Boost converter to replace to reach 9.5 v 
>
> \4.   Capacitor on reset : none as the button was only for the development, but if the line stays, should be added. 
>
> \5.   Visible LEDs are too powerful (dynamic is not appropriate, it is deem at 1% currently). Check the resistor values in place, propose alternative one. 
>
>  
>
> Power board :
>
> 1. Change the push button in accordance with     mechanical design 
>
>  
>
> In CW30, Alin will submit you (via sharing the Altium project) the new files for PCBA manufacturing.
>
> In the meantime, if you see some others changes missing just let us know,
>
>  
>
> Thks & Br,
>
>  
>
> Maurice*



RLUV LEDs changed from 100Ohm to 50Ohm in order to comply with the forward voltage of the UVBLEDs

RLVIS LEDs changed from 100Ohm  to 300Ohm in order to comply with 40mA forward voltage of Visible LEDs.
