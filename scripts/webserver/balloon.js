

if (Ext.BLANK_IMAGE_URL.substr(0, 5) != 'data:') {
	Ext.BLANK_IMAGE_URL = 'lib/extjs/resources/images/default/s.gif';
}

function isInt(x) {
	return ! isNaN(x);
}

function readDate(rawData) {
	return Date.parseDate(rawData, "His");
}

function readLat(rawData) {
	var d = parseFloat(rawData.slice(8, 10));
	var m = parseFloat(rawData.slice(10, 15));
	d = d + m / 60.0;
	if (rawData.charAt(15) == 'S')
		d *= -1;
	return d;
}

function readLon(rawData) {
	var d = parseFloat(rawData.slice(17, 20));
	var m = parseFloat(rawData.slice(20, 25));
	d = d + m / 60.0;
	if (rawData.charAt(25) == 'W')
		d *= -1;
	return d;
}

function readTelemetry(rawData, n) {
	var r = rawData.slice(27);
	var d = r.split(';');
	return d[n];
}

function readMsg(m) {
	if (m.charAt(0) == '>' && isInt(m.slice(1, 7)) && m.charAt(7) == 'h')
		return m.slice(8);
	else
		return m;
}

function isTelemetry(m) {
	return m.charAt(0) == '/' && isInt(m.slice(1, 7));
}

function deg2rad(deg) {
	return deg * Math.PI / 180;
}

function rad2deg(rad) {
	return rad * 180 / Math.PI;
}


/**
 * Use the Haversine formula to calculate great-circle distances between the
 * two points.
 *
 * The Haversine formula remains particularly well-conditioned for numerical
 * computation even at small distances, unlike calculations based on the
 * spherical law of cosines.
 */
function distance(lat1, lon1, lat2, lon2)
{
	var PLANET_RADIUS = 6371000;
	var d_lat = deg2rad(lat2 - lat1);
	var d_lon = deg2rad(lon2 - lon1);
	var a = Math.sin(d_lat / 2) * Math.sin(d_lat / 2) + Math.cos(deg2rad(lat1)) * Math.cos(deg2rad(lat2)) * Math.sin(d_lon / 2) * Math.sin(d_lon / 2);
	c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a))
	return PLANET_RADIUS * c;
}

/**
 * Evaluate the bearing (also known as forward azimuth) using spherical law
 * coordinates.
 *
 * The bearing is a straight line along a great-circle arc from the start point
 * to the destination point.
 */
function bearing(lat1, lon1, lat2, lon2) {
	var res = rad2deg(Math.atan2(Math.sin(deg2rad(lon2 - lon1)) *
		Math.cos(deg2rad(lat2)), Math.cos(deg2rad(lat1)) *
		Math.sin(deg2rad(lat2)) - Math.sin(deg2rad(lat1)) *
		Math.cos(deg2rad(lat2)) * Math.cos(deg2rad(lon2) -
		deg2rad(lon1))));
	return (Math.round(res) + 360) % 360;
}

/**
 * Compute the heading
 */
function compass_hdng(brng) {
    var d = [
	    "N", "NNE", "NE", "ENE",
	    "E", "ESE", "SE", "SSE",
	    "S", "SSW", "SW", "WSW",
	    "W", "WNW", "NW", "NNW"
    ];
    return d[Math.round(brng / 22.5)];
}

var msg_set = {};

Ext.onReady(function(){
	var map = new OpenLayers.Map({ controls: [
		                new OpenLayers.Control.Navigation(),
		                new OpenLayers.Control.PanZoomBar(),
				new OpenLayers.Control.LayerSwitcher({'ascending':false}),
		                new OpenLayers.Control.ScaleLine(),
		                new OpenLayers.Control.MousePosition({displayProjection: new OpenLayers.Projection('EPSG:4326')}),
		                new OpenLayers.Control.KeyboardDefaults(),
				new OpenLayers.Control.Navigation({mouseWheelOptions: {interval: 200}})
		]
	});

	var opencycle = new OpenLayers.Layer.OSM("OpenCycleMap", stsp_config.opencycle_url + "/${z}/${x}/${y}.png",
		{numZoomLevels: 16, transitionEffect: "resize", sphericalMercator : true});

	var osm = new OpenLayers.Layer.OSM("OpenStreetMap", "http://b.tile.openstreetmap.org/${z}/${x}/${y}.png",
		{numZoomLevels: 16, transitionEffect: "resize", sphericalMercator : true});

	var gterr = new OpenLayers.Layer.Google(
		"Google Maps Terrain",
		{type: google.maps.MapTypeId.TERRAIN, sphericalMercator : true}
	);
	var gmap = new OpenLayers.Layer.Google(
		"Google Maps Classic",
		{numZoomLevels: 20, sphericalMercator : true}
	);
	var ghyb = new OpenLayers.Layer.Google(
		"Google Maps Hybrid",
		{type: google.maps.MapTypeId.HYBRID, numZoomLevels: 20, sphericalMercator : true}
	);
	var gsat = new OpenLayers.Layer.Google(
		"Google Maps Satellite",
		{type: google.maps.MapTypeId.SATELLITE, numZoomLevels: 22, sphericalMercator : true}
	);
	// create vector layer
	var vecLayer = new OpenLayers.Layer.Vector("Balloon trajectory");

	// Choose default base map
	if (stsp_config.opencycle_default)
		map.addLayers([opencycle, osm, gmap, gsat, gterr, ghyb, vecLayer]);
	else
		map.addLayers([gmap, gterr, gsat, ghyb, osm, opencycle, vecLayer]);

	var telem_fields = [
		{name: 'time', type: 'date'},
		{name: 'lat', type: 'float'},
		{name: 'lon', type: 'float'},
		{name: 'altitude', type: 'float'}
	];

	//Read other telemetry fields from configuration
	for (var i = 0; i < stsp_config.fields.length; i++)
		telem_fields.push({name: stsp_config.fields[i], type: 'string'});

	// create feature store, binding it to the vector layer
	var telem_store = new GeoExt.data.FeatureStore({
		layer: vecLayer,
		fields: telem_fields
	});
	telem_store.sort('time', 'ASC');

	var statusmsg_store = new Ext.data.ArrayStore({
		fields: [
		    {name: 'time', type: 'date'},
		    {name: 'msg', type: 'string'}
		]
	});
	statusmsg_store.sort('time', 'ASC');

	function ajaxFailure(response, opts) {
		//console.log('server-side failure with status code ' + response.status);
	}

	function addGrid(store, rec) {
		store.addSorted(rec);
		return store.indexOf(rec);
	}

	function highlightRow(grid_name, idx)
	{
		var grid = Ext.getCmp(grid_name);
		var row = grid.getView().getRow(idx);
		Ext.get(row).highlight();
	}

	function selectLast(grid_name) {
		if (Ext.getCmp('autofollow_btn').pressed) {
			var grid = Ext.getCmp(grid_name);
			grid.getSelectionModel().selectLastRow();
			grid.getView().focusRow(grid.getStore().getCount() - 1);
		}
	}


	function updateStatusBar() {
		if (telem_store.getCount() < 2)
			return;

		var last = telem_store.getAt(telem_store.getCount() - 1);
		var pre_last = telem_store.getAt(telem_store.getCount() - 2);
		if (pre_last.data.lat == 0 || last.data.lat == 0)
			return;

		var dh = last.data.altitude - pre_last.data.altitude;
		var dt = last.data.time - pre_last.data.time;
		var vs_ms = dh / (dt / 1000);
		var vs_kmh = vs_ms * 3.6;
		vs_ms = Math.round(vs_ms*100)/100;
		vs_kmh = Math.round(vs_kmh);

		var vspeed_lbl = Ext.getCmp('vspeed_label');
		vspeed_lbl.setText('<b>' + vs_ms + ' m/s | ' + vs_kmh + ' km/h'+'</b>');

		var brng = bearing(pre_last.data.lat, pre_last.data.lon, last.data.lat, last.data.lon);
		var brng_lbl = Ext.getCmp('brng_label');
		brng_lbl.setText('<b>' + brng + '° | ' + compass_hdng(brng)+'</b>');

		var vh_ms = distance(pre_last.data.lat, pre_last.data.lon, last.data.lat, last.data.lon) / (dt / 1000);
		var vh_kmh = vh_ms * 3.6;
		vh_ms = Math.round(vh_ms*100)/100;
		vh_kmh = Math.round(vh_kmh);
		var hspeed_lbl = Ext.getCmp('hspeed_label');
		hspeed_lbl.setText('<b>' + vh_ms + ' m/s | ' + vh_kmh + ' km/h'+'</b>');

		var d = distance(last.data.lat, last.data.lon, stsp_config.base_lat, stsp_config.base_lon);
		d = Math.round(d/100)/10;
		var dist_lbl = Ext.getCmp('dst_label');
		dist_lbl.setText('<b>' + d + ' km'+'</b>');
	}


	function parseIndex(resp, opts) {
		var msg_list = resp.responseText.trim().split('\n');

		for (var i = msg_list.length - 1; i >= 0; i--) {
			var msg = msg_list[i];
			if (! msg_set[msg]) {
				msg_set[msg] = true;
				Ext.Ajax.request({
					url: 'msg/' + msg,
					msg_id : msg,
					success: function(response, opts) {
						var m = response.responseText;
						if (isTelemetry(m)) {
							// Create new record
							var lat = readLat(m);
							var lon = readLon(m);
							var new_rec = {
							    time: readDate(opts.msg_id),
							    lat: lat,
							    lon: lon,
							    altitude: readTelemetry(m, 0)
							};
							for (var i = 0; i < stsp_config.fields.length; i++)
							    new_rec[stsp_config.fields[i]] = readTelemetry(m, i+1);

							var rec = new telem_store.recordType(new_rec, opts.msg_id);
							var p = new OpenLayers.Geometry.Point(lon, lat);
							p.transform(new OpenLayers.Projection('EPSG:4326'), map.getProjectionObject());
							var v = new OpenLayers.Feature.Vector(p);

							// Do not display points with invalid position.
							if (lat == 0 && lon == 0)
								v.style = 'none';

							rec.setFeature(v);
							var idx = addGrid(telem_store, rec);

							highlightRow('telem_grid', idx);
							selectLast('telem_grid');
						}
						else {
							var rec = new statusmsg_store.recordType(
								{time: readDate(opts.msg_id), msg: readMsg(m)},
								opts.msg_id
							);
							var idx = addGrid(statusmsg_store, rec);
							highlightRow('status_grid', idx);
							selectLast('status_grid');

						}
					},
					failure: ajaxFailure
				});

			}
		}
	}

	var toolbarCfg = {
		region: 'north',
		height: 25,
		xtype: 'toolbar',
		id: 'uptoolbar',
		items: [
			'Distance from launch:',
			{
			    id: 'dst_label',
				xtype: 'tbtext',
				text: '---'
			}, '-',
			'Heading:',
			{
			    id: 'brng_label',
				xtype: 'tbtext',
				text: '---'
			}, '-',
			'Horizontal speed:',
			{
			    id: 'hspeed_label',
				xtype: 'tbtext',
				text: '---'
			}, '-',
			'Vertical speed:',
			{
			    id: 'vspeed_label',
				xtype: 'tbtext',
				text: '---'
			}, '-', '->',
			{
			    text: 'Autocenter on updates',
			    enableToggle: true,
			    pressed: true,
			    id: 'autofollow_btn'
			}
		]
	};

	var telem_cols =  [
		{header: "Time", xtype: 'datecolumn', width: 60, format: 'H:i:s', dataIndex: 'time'},
		{header: "Latitude", xtype: 'numbercolumn', width: 70, format:'0.000000', dataIndex: 'lat'},
		{header: "Longitude", xtype: 'numbercolumn', width: 70, format:'0.000000', dataIndex: 'lon'},
		{header: "Altitude", dataIndex: 'altitude', renderer: 'htmlEncode'}
	];

	for (var i = 0; i < stsp_config.fields.length; i++)
		telem_cols.push({header: stsp_config.fields[i], renderer: 'htmlEncode', dataIndex: stsp_config.fields[i]});


	/*
	 * Handle selection of points on the map/grid.
     * Points with invalid coordinates are not selectable.
	 */
	var selmode = new GeoExt.grid.FeatureSelectionModel({ autoPanMapOnSelection: true });
	selmode.on('beforerowselect', function(o, idx, ke, rec) {
		if (rec.data.lat == 0 && rec.data.lon == 0)
			return false;
		else
			return true;
	});

	var layout = new Ext.Viewport({
		renderTo: Ext.getBody(),
		layout: { type: 'border' },
		items: [
			toolbarCfg, {
				title: 'Baloon trajectory',
				region: 'center',
				xtype: 'gx_mappanel',
				id: 'map_widget',
				zoom: 9,
				center: new OpenLayers.LonLat(stsp_config.base_lon, stsp_config.base_lat).transform(new OpenLayers.Projection('EPSG:4326'), map.getProjectionObject()),
				map: map
			},{
				title: 'Status Messages',
				region: 'east',
				xtype: 'grid',
				id: 'status_grid',
				split: true,
				width: 295,
				autoExpandColumn: 1,
				store: statusmsg_store,
				colModel: new Ext.grid.ColumnModel({
					deafultSortable: false,
					columns: [
						{header: "Time", xtype: 'datecolumn', width: 60, format: 'H:i:s', dataIndex: 'time'},
						{header: "Message", dataIndex: 'msg', renderer: 'htmlEncode'}
					]
				})
			},{
				title: 'Telemetry',
				region: 'south',
				xtype: 'grid',
				id: 'telem_grid',
				split: true,
				height: 220,
				store: telem_store,
				colModel: new Ext.grid.ColumnModel({
					deafultSortable: false,
					columns: telem_cols
				}),
				sm: selmode
			}
		]
	});

	updateIndex = function() {
		//console.log("updateIndex()");
		Ext.Ajax.request({
			url: 'msg/msg_index',
			success: parseIndex,
			failure: ajaxFailure
		});
		updateStatusBar();
		setTimeout("updateIndex()", stsp_config.upd_delay);
	}
	updateIndex();
});

