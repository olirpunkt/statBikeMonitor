# Story 1 - Display RPM
As a rider of a stationary bike (roasb) I want to see my current rpm so that I can train according to plan (e.g. if the plan says "ride for x minutes with y rpm").

AC:
1. RPM is displayed in a web-application as a gauge
2. RPM gauge shows the difference in RPM to the previous value via "up/down" symbols an the difference value

To be used:
Nodemcu v3 as metering device
Hall sensor
Plotly for display
Websockets for communication
NodeJS as platform
Raspberry pi or any other linux machine as server

# Story 2 - Display Speed
As a roasb I want to see my current speed so that I can see how fast I would be on a flat track "in reality".

AC: 
1. Speed is displayed in a web-application as a gauge


# Story 3 - Log and display distance

As a roasb I want to see the distance traveled in my current session so that I have an idea about how far I would have traveled on a flat track "in reality". 

# Story 4 - Enter and save data

As a roasb I want to be able to enter and save:
1. Wheel circumference
2. Server (Host, Port)
so that I can reconfigure my setup without the need to recompile the code

For later: 
3. gear ratio
4. resistance factor of stationary bike

AC:
1. It is possible to enter the values stated above "somewhere"
2. The values are taken into account and are saved for further reference as well as the basis for e.g. speed and distance calculation