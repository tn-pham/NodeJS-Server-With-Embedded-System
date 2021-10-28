"use strict";
// Client-side interactions with the browser.

var showedError = false;

// Make connection to server when web page is fully loaded.
var socket = io.connect();
$(document).ready(function() {

	// Beats
	$('#modeNone').click(function(){
		sendUDPRequest("beat 0");	
	});
	$('#modeRock1').click(function(){
		sendUDPRequest("beat 1");
	});
	$('#modeRock2').click(function(){
		sendUDPRequest("beat 2");
	});

	// Drum sounds
	$('#hi_hat').click(function(){
		sendUDPRequest("drum hi-hat");	
	});
	$('#snare').click(function(){
		sendUDPRequest("drum snare");
	});
	$('#bass').click(function(){
		sendUDPRequest("drum bass");
	});

	// Tempo
	$('#tempoUp').click(function(){
		sendUDPRequest("tempo up");
	});
	$('#tempoDown').click(function(){
		sendUDPRequest("tempo down");
	});

	// Volume
	$('#volumeUp').click(function(){
		sendUDPRequest("volume up");
	});
	$('#volumeDown').click(function(){
		sendUDPRequest("volume down");
	});

	$('#error-box').hide();

	// If can't connect to Node.js server
	socket.io.on('connect_error', function(err) {
		$('#error-text').html("<em>Node.js server is dead!!!</em>");
			
		if(!showedError) {
			$('#error-box').show();
			// hide after 10s, only show once
			var timer = setTimeout(function() {
				$('#error-box').hide();
			}, 10000);
		}	
		showedError = true;
	});

	// If Beaglebone app is not running
	socket.on('bbclose', function() {
		$('#error-text').html("<em>Beaglebone is dead!!!</em>");
		$('#error-box').show();
		// hide after 10s
		var timer = setTimeout(function() {
			$('#error-box').hide();
		}, 10000);
	});

	// Normal reply
	socket.on('commandReply', function(result) {
		var array = result.split("--");
		var beat = array[0];
		var volume = array[1];
		var bpm = array[2];

		$('#modeid').html("<em>" + beat + "</em>");
		$('#tempoid').html("<em>" + bpm + "</em>");
		$('#volumeid').html("<em>" + volume + "</em>");

		//clearTimeout(nodeTimer);
	});
	

	// Uptime and ping for update (every 300ms)
	window.setInterval(function() {
		socket.emit('proc', 'uptime');
		sendUDPRequest("info");
	}, 300);
	
	socket.on('fileContents', function(result) {
		var contents = parseInt(result.contents);
		console.log(contents);
		var hours = Math.floor(contents / 3600);
		var minutes = Math.floor((contents - hours * 3600)/ 60);
		var seconds = contents % 60;
	
		$('#uptime').html("<em>" + hours + ":" + minutes + ":" + seconds + "(H:M:S)</em>");
	});	
});

function sendUDPRequest(message) {
	socket.emit('beatbox', message);
};

