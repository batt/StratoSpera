stsp_config = {
	//Launch base coordinates used to compute distance
	base_lat: 43.606167,
	base_lon: 11.311833,

	fields: [
		// Put balloon telemetry data fields here.
		// NOTE: the altitude, which is always the first field, is omitted!
		'Ext. Temperature',
		'Pressure',
		'Humidity',
		'Int. Temperature',
		'Battery Voltage',
		'Acceleration',
		'HADARP Geiger',
	],

	//Delay between updates
	upd_delay: 5000,

	//opencycle_url: 'http://b.tile.opencyclemap.org/cycle',
	//Use this for local cached maps (require custom webserver)
	opencycle_url: 'maps',

	//Set to true to have OpenCycle maps as the default map
	//if set to false, the default will be Google Maps classic
	opencycle_default: false,
};
