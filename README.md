# ambient-weather-display
Ambient weather display code for HCDE 440 Final Project. The ambient weather display consists of three main screens on a cycle with each screen displaying for 10 seconds.

- Screen 1: Weather - Displays current temperature and weather condition (Sunny, Cloudy, Light Rain, etc.)
- Screen 2: Air Quality - Displays current air quality value and a message that corresponds to the severity level of the air quality.
- Screen 3: Historical temperature trend - Displays a graph showing the historical temperature trend for the current day from 2009 to 2019.

The code within the final_project arduino project folder contains the code ran on our ESP8266 board which served as the sensor platform. To use the code, you will need to obtain your own API key for the breezeometer AQI API and the historical weather data API. The Processing sketch is the code that ran on our Raspberry Pi to power our display screen.