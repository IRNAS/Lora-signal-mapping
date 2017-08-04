#!/usr/bin/python

# Copyright (C) 2007 by Jaroslaw Zachwieja <grok!warwick.ac.uk>
# Copyright (C) 2008 by TJ <linux!tjworld.net>
# Published under the terms of GNU General Public License v2 or later.
# License text available at http://www.gnu.org/licenses/licenses.html#GPL

import serial
import string
import sys
import getopt
import ast

parseStr = lambda x: x.isalpha() and x or x.isdigit() and int (x) or x.isalnum() and x or len(set(string.punctuation).intersection(x)) == 1 and x.count('.') == 1 and float(x) or x

def usage():
		print "Usage:"
		print " -p | --port <device>   e.g. /dev/ttyS0"
		print " -b | --baud <speed>    e.g. 4800"
		print " -f | --file <filename> e.g. /tmp/gps.kml"
		print " -h | --help     display options"

def main():
	# defaults
	serial_port = "/dev/ttyUSB0"
	serial_baud = 9600
	file = 'realtime.kml'

	try:
		opts, args = getopt.getopt(sys.argv[1:], "p:b:f:h", ["port=", "baud=", "file=", "help"])
	except getopt.GetoptError:
		usage()
		sys.exit(1)
	else:
		for opt, arg in opts:
			if opt in ("-p", "--port"):
				serial_port = arg
			elif opt in ("-b", "--baud"):
				serial_baud = string.atof(arg)
			elif opt in ("-f", "--file"):
				file = arg
			elif opt in ("-h", "--help"):
				usage()
				sys.exit(0)
			else:
				print "Unknown option"

	gps = serial.Serial(serial_port, serial_baud, timeout=1)

	print "Serving data from %s (%d baud) to %s" % (serial_port, serial_baud, file)

	latitude = 0
	longitude = 0
	speed = 0
	heading_in = 0
	altitude = 0
	range = 1000
	tilt = 30

	while 1:
		line = gps.readline()
		datablock = line.split(',') 
		if(line):

			print(datablock);
			
			latitude_in = parseStr(datablock[0])
			longitude_in = parseStr(datablock[2])
			try:
				altitude = parseStr(datablock[5])
			except ValueError:
				# use last good value
				altitude = altitude
			speed_in = parseStr(datablock[4])
			heading_in = heading_in
			if datablock[1] == 'S':
				latitude_in = -latitude_in
			if datablock[3] == 'W':
				longitude_in = -longitude_in

			latitude_degrees = int(latitude_in/100)
			latitude_minutes = latitude_in - latitude_degrees*100

			longitude_degrees = int(longitude_in/100)
			longitude_minutes = longitude_in - longitude_degrees*100

			#latitude = latitude_degrees + (latitude_minutes/60)
			#longitude = longitude_degrees + (longitude_minutes/60)
			latitude = latitude_in;
			longitude = longitude_in;

			speed = int(speed_in * 1.852)
			range = ( ( speed / 100  ) * 350 ) + 650
			tilt = ( ( speed / 120 ) * 43 ) + 30
			heading = heading_in

			if speed < 10:
				range = 200
				tilt = 30
				heading = 0

			output = """<?xml version="1.0" encoding="UTF-8"?>
	<kml xmlns="http://earth.google.com/kml/2.0">
		<Placemark>
			<name>%s km/h</name>
			<description>^</description>
			<LookAt>
				<longitude>%s</longitude>
				<latitude>%s</latitude>
				<range>%s</range>
				<tilt>%s</tilt>
				<heading>%s</heading>
			</LookAt>
			<Point>
				<coordinates>%s,%s,%s</coordinates>
			</Point>
		</Placemark>
	</kml>""" % (speed,longitude,latitude,range,tilt,heading,longitude,latitude,altitude)

			f=open(file, 'w')
			f.write(output)
			f.close()
			#print(output)
		
	ser.close()
	
if __name__ == "__main__":
	main()