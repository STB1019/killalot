# Instruction before Fighting

## Zero, Set Jumper configuration

In order to make killalot work, you have to connect the jumpers with this configuration: three line sensor, three proximity sensors and blue leds.

See figures (1) and (2) in this repos.


## First, set motor

If your robot has soldered flipping the two motors, please remove `//` before this statement in killalot.ino

    //#define FLIP_LEFT
    //#define FLIP_RIGHT


## Second, check Time

With our robot, we set `TIME_TO_WAIT` to 2.7 seconds, because our calibration phase requires 2.3 seconds; please consider to make some test before Fighting.
Therefore modify this statement as you wish

#define TIME_TO_WAIT 2.7