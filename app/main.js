const 	electron 				= require('electron');							// electron package
const 	{app, BrowserWindow} 	= electron;										// getting elements from electron


app.once('ready', () => {
	win = new BrowserWindow({
		width: 			1200,
		height: 		600,
		resizable: 		false,
		maximizable: 	false,
		title: 			'3D LoRa scanner',
	});

	win.toggleDevTools();										// used for debuging, the dev tools show up
});

app.on('browser-window-created', (e, window) => {
	window.setMenu(null);										
});