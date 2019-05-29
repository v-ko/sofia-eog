# sofia-eog
Consider this app in Alpha. There are still quite a few features to be added and documented, but the basic functionality is in place.
Pipeline building for the porpouses of experimentation stream data processing. Built mostly for data similar to audio streams. The filters should be executables that take input data from stdin and output with stdout
Tested on linux, but should work multiplatform (where Qt is supported)
### Features:
- Display the streaming data as a graph (that is browsable)
- Record all the streaming data for a launch of the pipline in a separate timestamped directory with the filter configurations stored (for reproducibility)
- Browse the existing record data
- Catch and display stderr

![screenshot](http://i.imgur.com/ub4bZwJ.png)

### Filter definition files
The components of the pipeline are defined in a filters folder (defined by the -f command line argument). There you'll hold basically a template for each record, while through the GUI you can enable or disable components and whether their streams get recorded on the file system. The records folder you can specify through the -r CLI option.

Each filter has an identifier and can receive input from only one other filter (or zero for root nodes). Multiple filters can receive input from the same node though (so you can branch the graph out).

Here's an example filter definition for recording from the microphone with arecord (under linux):
```
#Static
Exec=/usr/bin/arecord
Arguments=-Dplug:hw:0 -f S16_LE -t raw -r OutputSampleRate
Identifier=raw_input
InputIdentifier=
OutputSampleRate=8000
SampleSize=2

#Dynamic (can be changed by the program)
RecordsToFile=0
DisplaySampleRate=500
IsEnabled=1
```
Then you can route the input to a second filter. I used the example as pretty much a low pass filter for the EOG sensor data (it's included in the code base):
```
#Static
Exec=/usr/bin/arecord
Arguments=-Dplug:hw:1 -f S16_LE -t raw -r OutputSampleRate
Identifier=raw_input
InputIdentifier=
OutputSampleRate=8000
SampleSize=2

#Dynamic (can be changed by the program)
RecordsToFile=0
DisplaySampleRate=500
IsEnabled=0
```
### Current status and future of the app
I'm transitioning into using the app only for recording and monitoring pipes, since it's hard to debug and control individual components through this GUI ATM. Its possible to do, but I don't have the time to implement proper process control in the app. So the idea currently is to start all filters pointed at linux named pipes and monitor/record the pipe data via this tool.

### About the origins of the project
I started working on the project as a hobby, while attempting to make a REM detection device. The software was supposed to facilitate the filtering of the sensor data and invoking some kind of signalling for the purpouses of lucid dreaming.
[The original thread on DreamViews](http://www.dreamviews.com/lucid-aids/158534-sofia-eog-open-source-software-hardware-rem-detection.html)
