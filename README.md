# Pi-Display

GTK app to display cool stuff on a mini monitor connected to my RPI. I'll probably never complain about the JS ecosystem again.

## Dependencies
- gtkmm3
- libcurl

## Setup
```
mkdir build
cd build/
cmake ..
make && MyGTKApp
```

## Todo
[x] Refactor update_screen function, update system metrics independently of main label
[x] Refactor css implementation
[ ] Add calendar integration to display upcoming events
[ ] Add Linear integration to display tasks for the day
[ ] Life progress bar