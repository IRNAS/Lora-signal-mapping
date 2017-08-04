#!/usr/bin/python

import serial
import string
import sys
import getopt
import ast
import heatmap
import random

# easier transfer from string to float
parseStr = lambda x: x.isalpha() and x or x.isdigit() and int (x) or x.isalnum() and x or len(set(string.punctuation).intersection(x)) == 1 and x.count('.') == 1 and float(x) or x

#
#	Function:     def usage()
#	Description:  Print the usage of this tool	
#
def usage():
		print "Usage:"
		print " -p  | --port <device>   e.g. /dev/ttyS0"
		print " -f  | --file <filename> e.g. /tmp/gps.kml"
		print " -h  | --help     display options"
		print " -sd | --sdcard   use sd card as source"

def main():
	serial_port = "/dev/ttyUSB0"				# default serial port
	serial_baud = 9600							# dont change the baud!			
	file = 'realtime.kml'						# the output file
	sd_card = False								# without argument the sd is not executing
	sd_card_file = "gps_data.txt"				# the sd card file that we will read

	try:
		# the arguments
		opts, args = getopt.getopt(sys.argv[1:], "p:f:h:sd", ["port=", "file=", "help", "sdcard"])
	except getopt.GetoptError:
		usage()
		sys.exit(1)
	else:	
		for opt, arg in opts:					# loop through the arguments
			if opt in ("-p", "--port"):
				serial_port = arg  				# set the port
			elif opt in ("-f", "--file"):
				file = arg                      # set the output file
			elif opt in ("-h", "--help"):         
				usage()							# print usage
				sys.exit(0)
			else:
				print "Unknown option"


	gps = serial.Serial(serial_port, serial_baud, timeout=1)						# open the serial 

	print "Serving data from %s (%d baud) to %s" % (serial_port, serial_baud, file) # small log


	# basic default data
	latitude = 0
	longitude = 0
	speed = 0
	heading_in = 0
	altitude = 0
	range_gps = 1000
	tilt = 30

	while 1:

		line = gps.readline()															# reading the serial

		datablock = line.split(',') 													# splitt data by ','
		if(line):																		# if the line is not null

			print(datablock);															# small debug
			
			latitude_in = parseStr(datablock[0])										# get latitude
			longitude_in = parseStr(datablock[2])										# get longitude
			try:
				altitude = parseStr(datablock[5])										# get altitude
			except ValueError:
				# use last good value
				altitude = altitude
			speed_in = parseStr(datablock[4])											# get speed
			'''if datablock[1] == 'S':	
				latitude_in = -latitude_in
			if datablock[3] == 'W':
				longitude_in = -longitude_in'''

			'''
			latitude_degrees = int(latitude_in/100)
			latitude_minutes = latitude_in - latitude_degrees*100

			longitude_degrees = int(longitude_in/100)
			longitude_minutes = longitude_in - longitude_degrees*100
			'''

			latitude = latitude_in;														# reference
			longitude = longitude_in;													# reference

			speed = int(speed_in * 1.852)												# calculate speed
			range_gps = ( ( speed / 100  ) * 350 ) + 650								# the range
			tilt = ( ( speed / 120 ) * 43 ) + 30										# the speed

			# recalculate if speed is small
			if speed < 10:
				range_gps = 200
				tilt = 30
				heading = 0

			# pregenerated output
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
	</kml>""" % (speed,longitude,latitude,range_gps,tilt,heading,longitude,latitude,altitude)


			f=open(file, 'w')				# open the file
			f.write(output)					# write to it 
			f.close()						# close

	ser.close()								# cant leave the port open 
	
# python basics
if __name__ == "__main__":
	main()