import serial 
from pynmeagps import NMEAMessage, GET
import random

#pyld= ['4315.00000','N','08000.000000','E']

s = serial.Serial('COM5')


def generate_random_nmea_coordinate():
    # Generate random latitude
    lat_degrees = random.randint(0, 89)  # Latitude degrees (0-89)
    lat_minutes = random.uniform(0, 59.9999)  # Latitude minutes (0-59.9999)
    lat_hemisphere = random.choice(['N', 'S'])  # Hemisphere for latitude

    # Generate random longitude
    lon_degrees = random.randint(0, 179)  # Longitude degrees (0-179)
    lon_minutes = random.uniform(0, 59.9999)  # Longitude minutes (0-59.9999)
    lon_hemisphere = random.choice(['E', 'W'])  # Hemisphere for longitude

    # Format the coordinates in NMEA-style
    lat = f"{lat_degrees:02}{lat_minutes:07.5f}"
    lon = f"{lon_degrees:03}{lon_minutes:07.6f}"

    return lat,lon,lat_hemisphere,lon_hemisphere


while (1):
    request = s.read(1)

    lat, lon, h1, h2 = generate_random_nmea_coordinate()
    if len(lat.split('.')[0]) < 4:
        lat = "0"+lat
    pld = [lat, h1, lon, h2]


    msg = str(NMEAMessage('GN', 'GLL', GET, payload=pld)).encode('ascii')

    s.write(bytes([len(msg)]))
    s.write(bytes(msg))