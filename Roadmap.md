#Roadmap OpenReader project

Along the way: ** Write documentation**
## 1. Integrate image analysis code to C++
  - [ ] Translate Matlab code to C++ or compile MatLab code.
  - [ ] Insert the code in the software in a user friendly-way, with parameters
 - [ ] Embbed the code in the reader/Put it in the chip

## 2. Handle the communication between the reader and the mobile app

 - [ ] Send Image, results and other metrics to the mobile app
 - [ ] Modify the mobile app to display the results in an appropriate and safe manner (ask for the qr code of the patient)
 - [ ] Implement a communication channel for maintenance and security cheks (see 3.)
 
## 3. Write the reader's own code
### Maintenance
 - [ ] Hardware checks: for example if the light intensity is lower than a threshold, alert the user.
### Security
 - [ ] If the drawer is open, turn off the UV light.

### Low priority, optional:
  - [ ] Write our own driver for the camera
  - [ ] Create a pipeline for the researchers to easily change the embedded image-analysis code
 




