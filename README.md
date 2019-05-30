# BASIC ARDUINO RAYCASTER
### Ben Beshara, 2019
 This project contains a very simple raycasting algorithm, and outputs display to an ILI9341 display controller. It uses a STMPE610 touch contoller for input.

## Project Details
I wanted to brush up on some of my maths and also produce somthing fun. I picked up a Rapsberry Pi touch screen cheaply from JayCar and wired it up to an Arduino Nano v3.0 clone. Following some guides for maths and structure online (shout out to Lode Vandevenne, https://lodev.org/cgtutor/raycasting.html, who I also borrowed the map from because it was more interesting than the 16x16 plot I developed) I worked to write a basic renderer using line draw commands.

It runs at about 1/2 a frame per second on my nano so, have fun!

## Technical Limitations
I discovered late in the process that Arduinos don't really have a frame buffer. As far as I can tell, they're not created with any specialised graphics hardware what so ever. Which is fairly resonable given their primary usgae a sa microcontroller, not a grpahics adapter. All calculations are done on the CPU and they're mostly pretty okay speed wise, but drawing to the screen seems to be a slow process over SPI.

Because of the lack of a frame buffer, graphics are drawn progressively on the screen - it's a cool effect, seeing the ray cast being drawn, but it makes any illusion of smooth transitions (like there were any at 1/2 a frame per second anyway) impossible.

Ideally, we would draw to an off-screen buffer and 'flip' it to the screen before drawing the next frame to the second buffer.

The other issue is speed vs. memory. I initially wanted to create lookup tables for common calculations in order to speed up the frame time, but upon doing so quickly ran out of available RAM on the Arduino so was forced to perform these calculations on-the-fly, which saves memory but is slower.

Because of the memory constraint, it was also undesireable to add textures to the walls and floor/ceiling as not only would this greatly increase the frame time, I don't believe I could do so in a way that would be memory efficient enough for the Arduino. I cannot load the textures dynamically as I do not have an SD card or storage interface on my Arduino but this is something that would be interesting to look into in the future.

## What I learned
Microcontrollers are useful, fun and fairly accessible way to get into programming both from a microservices perspective and an IoT perspective.

It would have been better to choose a SoC with dedicated video hardware for this project if I wanted to squeeze performance and enhance the visual fidelity of the renderer. A modern SoC with graphics hardware would also be able to move the workload off of the CPU and onto the GPU, freeing up CPU time to process input and perhaps deal with other areas such as sound or AI processing, if we were to develop this prototype into a more fleshed-out video game.

An alternative is to use a separate chip just for hardware video processing, which is something I want to look in to if time permits.

## Prerequisites
This project relies on the following external libraries:
* AdaFruit_GFX
* Adafruit_ILI9341
* Adafruit_STMPE610

## License
If you need a license, I'm going to say that 'officially' this is MIT licensed code, but I'm going to go ahead and call this the Bad Religion license, 'Do what you want, just don't do it around me'. The code was written as a learning exercise for me and I hope it proves valuable to someone in the future. But don't break your stuff with it (somehow), it's not my fault if you do.