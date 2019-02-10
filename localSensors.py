import Adafruit_DHT

humidity, temperature = Adafruit_DHT.read_retry(Adafruit_DHT.DHT22, 4)

humidity = round(humidity, 2)
temperature = round(temperature, 2)

if humidity is not None and temperature is not None:

  print 'Temperature: {0:0.1f}*C'.format(temperature)
  print 'Humidity: {0:0.1f}%'.format(humidity)

else:

  print 'No data available'
