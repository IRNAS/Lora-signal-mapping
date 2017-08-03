## The app

This app is reading the serial data sent from the arduino and starts the processing.
The processings first steps are to log the coordinates and create a .geojson file.
This file will be later used with the npm package terriajs to generate a 3D google earth view.
