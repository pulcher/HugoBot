# Retrospective: RoboRama June 1, 2024

## Location
Dallas Makerspace - Interactive Activity Room

## Results

- JimBot: Came in 3rd place with 12.2 secs of runtime.  First place was just over 8 seconds.
- HugoBot: didn't run games.  couldn't get the distance correct.  See the Improvements secton.

## Pros

### JimBot

He did awesome.  But is not very repeatable.  got very lucky just like the year before.

### HugoBot

Lots of compilmentary comments.  
Found out that a power of 30 actually does work, and can spin the robot(most of the time).

It does correct itself, but still was a bit wonky.


## Cons

### JimBot

- He is going as fast as he can.  He needs to go faster.  
- Doesn't know how to correct itself.  
- The wheels don't track straight.  They wobble, but seem to mostly work ok.
- He can only do quiktrip, he needs to be able to run 4 square.
- no way to get real data from the robot.

### HugoBot

- The motors stall very easily.
- They are voltage controlled, and there is a non-linear wheel speed vs. voltage or in this case power supplied.  
- There is no way to detect if the wheels are really turning
- The programming model I envisioned is really not going to work.
  - Need to continously correct heading vs. actual orientation.
  - Y seemed to be really good for tracking the position.
    - X not as much
- No way to get all the data from the robot while it is in operation.

## Improvements

### JimBot

- Obtain larger wheels.  Assuming the motors can handle the larger wheels, this should give us a faster transit time.
- Reevaluate the motors/wheel connection.  
  - see if we can true it up.
  - might see if we can change out the motor connections
    - replace the gear boxes?
    - change motors to straight and use gears or belts?
- Possibly update the chasis, or create a new one with extrusions and 3d printed parts.
- Research how we might use the 3 DOF built-in IMU to help with the tracking.

### HugoBot

- Research on how we really should be controlling the maccanum wheels.
- Update the firmware of the megaPi to understand a heading, and speed.  Not a cardinal direction.
  - It needs to be able to control the speed of each motor based on the requested speed in a direction.
- Here is a link provided by Karim: [https://gm0.org/en/latest/docs/software/tutorials/mecanum-drive.html](Mecanum TeleOp)
- Possible replace the bottom portion with something like [https://www.aliexpress.us/item/3256805896729950.html?gatewayAdapt=glo2usa](New lower part example)
  - this one may be a bit much with a 25kg weight rating.
  - not sure how big this is
  - will this still work well for 6 can?
- Possible replace the frame to accommodate larger motors encoders
- Use a roboclaw from first robot and add another to be able to pid control all 4 wheels.  
- Inventory current motors to see if we can use them.
- Restructure code
  - need to incorporate keeping things striaght.
  - concept of a destination as opposed to coding to time/distance then action.
  - obstical avoidance
- document hugobot so others can help
- figure out a way to transfer data wirelessly

## First Actions

### JimBot

1. Design up JimBot a wooden bigger body.
2. Use the discount motor w/encoders as drive source
3. consider followers and differential steering.
4. Setup storage to collect data
5. explore I2C access
6. move to circuit python/arduino
7. get larger wheels.... that look cool.
8. add sound and neopixels.
9. use pulley on the drivetrain.
10. update power source.
    - all robots should have similar power sources.

### HugoBot

- Submit a PR for the menu system we are using to provide update for others.
- __next__  
