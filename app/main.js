const 	electron 				= require('electron');							// electron package
const 	{app, BrowserWindow} 	= electron;										// getting elements from electron
var writeFile = require('write-file')

var data_counter;
var gps_data = {};
var pushing_enabled = 1; var pushing_counter = 0;

var geojson = {
    "type": "FeatureCollection",
    "features":[{
        "type":"Feature",
        "geometry":{
            "type":"LineString",
            "coordinates":[]
        },
        "properties":null
    }]
};

const SerialPort = require('serialport');
const Readline = SerialPort.parsers.Readline;
const port = new SerialPort('/dev/ttyUSB1');
const parser = new Readline();
port.pipe(parser);

app.once('ready', () => {
	win = new BrowserWindow({
		width: 			1000,
		height: 		500,
		resizable: 		false,
		maximizable: 	false,
		title: 			'Projector',
		//backgroundColor:'#6C7A89'

	});

	win.toggleDevTools();										// used for debuging, the dev tools show up

});

parser.on('data', function(in_data) {

	if(in_data.localeCompare("S") == 1) {
		console.log("Starting");
		data_counter = 0;
	} else if(in_data.localeCompare("E") == 1) {
		data_counter = -1;
		console.log(gps_data);
		console.log("Ending");
		if(pushing_enabled == 1) {
			if(pushing_counter == 250) {
				writeFile('test.geojson', geojson, function (err) {
					if (err) return console.log(err)
					console.log('file is written')
				pushing_enabled = 0;
				});
			}
			if(gps_data[0] != null && gps_data[1] != null) {
				geojson.features[0].geometry.coordinates.push([gps_data[1], gps_data[0]]);
			}
			pushing_counter++;
			console.log(pushing_counter);
		}

	} else {
		if(data_counter != -1) {
			gps_data[data_counter] = parseFloat(in_data);
			data_counter++;
		}
	}
});


