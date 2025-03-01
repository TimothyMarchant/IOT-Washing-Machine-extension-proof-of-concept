# IOT-Washing-Machine-extension-proof-of-concept
A far fetched idea of somehow connecting a non-IOT washing machine over bluetooth  
For the person viewing this, I had this idea of connecting the washing machine in the basement to the internet (or bluetooth).  
I noticed that on the one in my house that there are 5 status LEDs.  This gave me the following idea: can we read the LEDs and transmit the current status to a phone?  
We can do this, but the only real challenge to this would be somehow attaching it to the already existing PCB (I assume there's one in there!) in the washing machine without breaking it.  
So this project is essentially if we could do such a thing how would we transmit the data, etc.  I simply made it so the microcontroller that controls the bluetooth module over UART reads the inputs and sends a short string with an acknowledgment and the current status.  
In this repo, the "washing machine" is the STM microcontroller (L432kc; STMmain.c) and the "extension" is controlled by the Renesas RA2E1 microcontroller (Renesas_hal_entry.c; the bluetooth module is the HC-05 and HM-10; both work).  
This was my first time working with both brands.  I only included the files where I actually wrote code in them.  
Here are some pictures:  
The washing machine:  
![washingmachine](https://github.com/user-attachments/assets/e96dc642-f760-4d76-97d9-9c48d312e098)  


The proof of concept setup:  
![image](https://github.com/user-attachments/assets/653bc744-be26-4dad-b1a4-6211d5c5adcf)

From what I learned in this project (and a few other things) I'm going to go back to using my SAML10 microcontroller to write to a E-paper display to make a electronic shelf price tag (proof of concept; maybe a PCB later).
