# Weatherbox

## Description

Weatherbox is a DIY prototype of a connected internet-of-things device. It shows either the current weather or a 1-day-forecast visually by creating a similar weather situation inside a glass box. It is motion-activated to preserve energy and prevent unnecessary noise from the pump and water atomizer. My goal with this project is an introduction into the internet of things, based on the inexpensive Particle Photon board.

A small OLED screen gives the following information:

- day of forecast (today or tomorrow)
- description of weather (e.g., cloudy, sunny, rain)
- maximum temperature of the day in °C

The following effects are possible:

- light of different colors
- fog (clouds)
- rain

The configured weather conditions are queried every 5 minutes by using the DarkSky weather service (see [DarkSky.net](https://darksky.net/dev)). The required internet connection is supplied via WiFi. Below is an image of the weatherbox in with rainy weather.

![Weatherbox](/img/box_1.jpg?raw=true)

## Weather Conditions

Here are a few gifs of different weather conditions. Please note that due to defective LED strips the colors may look weird.

### Rain

Here you can see rainy weather reflected in the weatherbox, both in dark and bright surrounding. As soon as i turn the knob in the right gif, the forecast of tomorrow changes to today's weather including the maximum temperature.

![Rainy Dark](https://media.giphy.com/media/7YDdxsqj108JE1TgUn/giphy.gif)
![Rainy](https://media.giphy.com/media/1jaLUvTY0w3D7kUjKz/giphy.gif)

### Fog/Clouds

Foggy and cloudy are basically the same thing (different lighting conditions, but the LED strips have issues). It is seldom foggy where i live, therefore cloudy weather also reflects with the fine mist created by the water atomizer.

![Foggy](https://media.giphy.com/media/3s2aAX0TISYz3XMbFs/giphy.gif)
![Cloudy](https://media.giphy.com/media/ipZJy8dGfuMzcgUcXN/giphy.gif)

### Clear Skies

Here, the LED color is supposed to be yellow/orange, but the LED strip is a bit defective. Basically the only active effect is a very bright yellowish light.

![Clear skies](https://media.giphy.com/media/8rFxqXMCDicZrf0X9r/giphy.gif)

## Hardware

### Parts List

Here is a list of important components i have used. I wanted to focus on the Particle Photon board. The Grove components easily extend the main board with the starter kit and some individual parts, no soldering required. Since the water atomizer and the water pump run at 5V, i need a logic level converter in between.

- DENNERLE NanoCube 10l Aquarium
- Neuftech DC 12V 3W Mini Water Pump (which runs slowly at 5V)
- Particle Photon with Header
- Seeed Studio Grove - PIR Motion Sensor
- Seeed Studio Grove - 96*96 OLED
- Seeed Studio Grove - Water Atomization v1.0
- Seeed Studio Grove Starter Kit for Photon
- Seeed Studio Grove Connectors
- 3.3V <-> 5V logic level converter
- RGB LED strip (WS2801 compatible)
- Mosfet, resistors, prototyping board, ...
- Sturdy box

### Assembled Electronics Box

I used a sturdy carton box to assemble the electronics inside. Holes are cut on the side for the OLED screen, the rotary encoder and the motion sensor. The components are then glued in place. On the back, a hole for the power supply and the cables for the water atomizer and water pump is drilled. The removable top of the box houses the LED strips. Rectangles with the shape of the LED strips are cut into the top. I used three short strips wired in series. Below is a look into the inside of the box. Another image also shows the individual components with labels.

![Inner side of the box](/img/box_inside.jpg?raw=true)
![Electronics in the box](/img/electronics_annotated.jpg?raw=true)

## Software

For me, the most interesting part of the project is the software. As stated in the introduction, i'm using the DarkSky.net API to get weather forecasts from the internet. DarkSky provides 1000 free requests per day, which is plenty for a hobbyist. Currently, the request interval is 5 minutes, although an hourly update would be just fine. We start by defining our integration of DarkSky into the Particle Photon.

### Particle Integration for DarkSky

Particle provides an easy way to add integrations. In the Particle console, an integration is added like this:

![API integration](/img/api_integration.png?raw=true)

The following settings can be used: I use a GET request to the DarkSky API. After registering, you should get an access token. This, as well as the coordinates of the location to forecast, make up a portion of the request URL.

![API basic settings](/img/api_edit.png?raw=true)

The next step is the definition of the response. The DarkSky request returns a JSON string, which we will convert into a much smaller C string. We filter our required information from the response and use a separator character for easy parsing. This string will be used in a function call in our code. You do not need to parse the JSON manually, the Particle integration will provide all keys as variables for use.

![API advanced settings](/img/api_advanced.png?raw=true)

After the integration is defined, the actual coding takes place. Fortunately, most Seeedstudio Grove components already come with libraries which are ready to use. Since the LED strip is WS2801 compatible, the Adafruit WS2801 library can be used to interact with the strip. In the Particle development editor, include the legacy library *WS2801*. The OLED display is controlled similarly via the legacy library *GROVE_OLED_DISPLAY_96X96*.

The code itself is split into three files:

- **ledInclude.h** only provides basic functionality for color conversion. The function *setStripColorHSB* takes a color definition as hue, saturation and brightness values and transforms them into RGB. This RGB is then used to set the RGB strip values
- **weatherInclude.h** is a short header file where the different weather conditions are defined. A weather definition is a struct with five float values: pump duty cycle, fogger duty cycle, LED hue, LED saturation, LED brightness.
- **weatherdisplay.ino** is the project source file. Typically for Arduinos, it contains a *setup*-function, which is called once at the start of the device, and a *loop*-function, which runs infinitely. The function *weatherHandler* is called with the string, which was defined in the DarkSky integration. Here, the forecast can be parsed and the weather description and maximum temperatures both for today and tomorrow are updated. Then, the function *weatherUpdate* is called. It checks the rotary encoder state to determine if the weather for today or tomorrow is displayed. Afterward, the weather description is translated into the actual weather object from *weatherInclude.h*. Finally, the OLED is updated with the new text. The *setup* initializes the LED strip, the OLED, subscribes the weatherUpdate hook/integration and deactivates the pump and fogger as their default state. The *loop*, which is run about every *CYCLE_MS* milliseconds, checks if the PIR sensor sensed motion in the last *PIR_ON_TIME* seconds. If not, all outputs are deactivated. If there was motion, the current rotary encoder value is read to determine if the weather forecast to display has changed since the last loop. If it did, the weather is updated. Lastly, if five minutes have passed, a new forecast is triggered to the DarkSky integration.

## Conclusion and Recommendation

Well, is this actually feasbile? Did i put it up in my home? Not really. Although the weather effects work in a basic way and the forecast update over internet is pretty neat, this prototype is not for permanent use. The water pump is too loud, even at 5V, the water atomizer makes a high pitched noise and only works if it floats on the water surface but is not completely submerged. I have tried making it float on a styrofoam pad, but that dissolves quickly. The LED strips and/or the strip connectors are of low quality and had issues after a few hours of operation.

But, if those issues were fixed, it had a more diffuse/soft light from below, and a black backwall behind the glass box, it might actually look very cool. Feel free to copy and improve. My intention of having a simple IOT device prototyped in a reasonable amount of time is fulfilled!