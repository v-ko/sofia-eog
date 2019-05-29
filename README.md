# sofia-eog
Consider this app in Alpha. There are still quite a few features to be added and documented, but the basic functionality is in place.
Pipeline building for the porpouses of experimentation stream data processing. Built mostly for data similar to audio streams. The filters should be executables that take input data from stdin and output with stdout
Tested on linux, but should work multiplatform (where Qt is supported)
Features:
..*Display the streaming data as a graph (that is browsable)
..*Record all the streaming data for a launch of the pipline in a separate timestamped directory with the filter configurations stored (for reproducibility)
..*Browse the existing record data
..*Catch and display stderr

#### About the origins of the project
I started working on the project as a hobby, while attempting to make a REM detection device. The software was supposed to facilitate the filtering of the sensor data and invoking some kind of signalling for the purpouses of lucid dreaming.
[The original thread on DreamViews](http://www.dreamviews.com/lucid-aids/158534-sofia-eog-open-source-software-hardware-rem-detection.html)
