# !deprecation warning!
this package is no longer being updated  
it only remains for historical reasons
## Usage
### ./build/ballBounce [file]
#### file: an optional file to provide initial values of the variables
- build using either cmake or make
- once built, simply run the executable ball_sdl
- the inital velocities will be 0 on both axis
- to change variables, write {var_name}=value ex: vx=500.4
- the variables that exist are {vx, vy, g}
- if you wish to restart the instant, simply press the escape key
## testing
###### (from project root dir)
### make test
### ./tests/test_ball