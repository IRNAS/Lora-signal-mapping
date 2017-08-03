const 	electron 				= require('electron');							// electron package
const 	{app, BrowserWindow} 	= electron;										// getting elements from electron
const 	writeFile 				= require('write-file')
const 	SerialPort 				= require('serialport');
const 	Readline 				= SerialPort.parsers.Readline;
const 	port 					= new SerialPort('/dev/ttyUSB1');
const 	parser 					= new Readline();


var data_counter;
var gps_data = {};
var pushing_enabled = 1; 
var pushing_counter = 0;
port.pipe(parser);

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

/*
*	This is executing on every data that is coming in from the serial
*/
parser.on('data', function(in_data) {

	// compare if it is the "starting" indentificator
	if(in_data.localeCompare("S") == 1) {
		console.log("Starting");																// small log
		data_counter = 0;																		// start the counter
	} 
	// compare if it is the "ending" indentificator
	else if(in_data.localeCompare("E") == 1) {
		data_counter = -1;																		// stop the counter
		console.log(gps_data);																	// log the data
		console.log("Ending");

		// check if pushing is enabled
		if(pushing_enabled == 1) {
			if(pushing_counter == 250) {														// small counter for testing
				
				// write to write-file
				writeFile('test.geojson', geojson, function (err) {
					if (err) return console.log(err)
					console.log('file is written')
					pushing_enabled = 0;														// disable push
				});
			}

			// only push when the data is not null
			if(gps_data[0] != null && gps_data[1] != null) {
				geojson.features[0].geometry.coordinates.push([gps_data[1], gps_data[0]]);		// push it to the coordinates array
			}

			pushing_counter++;																	// increment pushing counter
			console.log(pushing_counter);														// log it
		}

	} else {
		if(data_counter != -1) {																// if the counter is 'running'
			gps_data[data_counter] = parseFloat(in_data);										// put in the data with conversion to float
			data_counter++;																		// increment data counter
		}
	}
});


