# OpenReader - quantitative, open-source antigen test reader

**Our goal is to create a *quantitative* antigen test reader.**
We work on a prototype created by the team of Professor Alexandrou from LOB, Polytechnique.

### General presentation of the project 
(Note that the following contains what is done by both us **and** Professor Alexandrou's team, and does not represent per se the content of our CSE201 Project)

#### New antigen test and need for a reader
Traditional antigen tests provide binary information about the state of a patient: positive or negative. Professor Alexandrou's team developed new antigen tests based on fluorescent particles, that form control line and test lines whose brightness are a function of the virus' concentration in the body of the patient. The goal is not only to tell if the test is *negative* or *positive*, but also to quantify its result. To read the result of this new test, a machine is necessary as a human would not be able to perceive in a quantitative way the brightness of the lines and derive the concentration from them.
The researchers thus created a reader that takes a picture of the test cassette and runs an algorithm to analyze it.

#### Analysis algorithm
To derive a result from the image, we first have to identify the test line and the control line on the image. Once this is done, we can fit the control line and its surroundings into a gaussian on a slope, and subtract the background noise. This model has been foud by the researchers to be the most effective one to describe the results. Depending on the parameters of this model (i.e the mean and variance of the gaussian, and the y-intercept and slope of the linear function) a conversion table allows us to determine the quantitative result of the test.

#### Communication of the results
The results are then sent to a mobile app, coded in javascript. A desktop app also allows the researchers to use the algorithm on a computer without needing to use the reader.

#### Reader's work
Once the test is put in the Reader's drawer, a picture is taken. The reader needs to choose the right parameters of exposition time not to saturate the image. Some maintenance and security checks also need to be implemented in the reader's code. For example, if the drawer is open, the UV lights used by the reader to make the particles of the test fluorescent should be turned off for the safety of the user.

### What we have to do

#### MatLab code
A first already working version of the Image analysis code has been implemented in MatLab. 
As preliminary work, we first need to fix the MatLab code to make run on simple inputs. 
We then need to translate it to C++ to be able to run it on the reader and make the code faster (for the moment, the running time of the algorithm in MatLab is too long for practical use).
Once the algorithm is coded in C++, we can further improve it.

#### The reader
We need to make the algorithm work on the reader's chip, which is very different from running the code on a computer.
We need to implement maintenance and security checks.
We need to communicate the Image analysis' result to the mobile app.
We need to control the reader's camera.
