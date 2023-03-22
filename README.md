# Accident-zone-indicator
### GPS based accident zone alert system with speed limit function
This is a working prototype of an alert system, designed to do these following tasks
- Alert the user on exceeding speed limit in any scenario
- Alert the user, if they are APPROACHING any accident/danger zones and are within 200 meters from the location

### Working principle

![image](https://user-images.githubusercontent.com/95964781/226907081-22a6db7b-12f6-4604-b967-ba79de36c930.png)

This model is a simple prototype designed for a small area of radius 10km. Random locations were chosen as accident zones, to test the authenticity of the device.
The device operations are as follows,
1. Arduino is the brain of the device
2. GPS module is used to get live location and speed of the user
3. SD card is used to store the danger zones [accident zone coordinates]
4. Search algorithm is a simple math trick, which calculates distance between the user and the zones every second and gives an alert if the user is within the range of 200 meters and approaching the location.
