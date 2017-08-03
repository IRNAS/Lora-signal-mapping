const 	electron 				= require('electron');							// electron package
const 	{app, BrowserWindow} 	= electron;										// getting elements from electron

var data_counter;
var gps_data = {};

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
	} else {
		if(data_counter != -1) {
			gps_data[data_counter] = in_data.replace('\r', '');
			data_counter++;
		}
	}
});


