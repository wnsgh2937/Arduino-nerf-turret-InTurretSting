# InTurretSting
InTurretSting - Arduino nerf turret using infrared distance sensor(VL53L1X)

## Contents
- Background
- Install
- Video
- Important Materials
- Code Explain

## Background
First, I want to use radar sensor. But radar sensor is too expansive, so I decided to make radar system by using distance sensor. 
And I thought turret is best for using radar system. So I finally decided to make turret that has auto enemy detecting radar system by using distnace sensor. I want to make the radar system that can detect 3m long object. <br><br>
There are many distance sensors using ultrasonic wave, infrared, raser. Raser sensor can detect far object but is expensive and ultrasonic wave sensor is cannot detect even 1m far object. There is VL53L1X can detect 3m far object in infrared distance sensor. 
<br><br>And I made it all. I hope it will be help for who want to make turret.

## Install
Just download the code and libraries.
And import "In_turret_Sting" code to Arudino board.

## Video
[This](https://youtu.be/yleDrqbNKQg) is my test video <br>
<u>*please turn on the subscript*</u>

## Important Materials
- VL53L1X 1ea (Infrared Distance Sensor )
- PG-047 1ea (9V 2.5A Adaptor)
- MG90S 2ea ( Servo Motor )
- LM2596 1ea ( DC-DC Convertor)
- PAN14EE12AA1 2ea ( DC Motor )
- 3 Color LED ( Anode Type )
- Arduino UNO Board 1ea

## Code Explaination
