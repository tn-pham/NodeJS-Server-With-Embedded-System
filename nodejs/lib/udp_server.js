"use strict";
/*
 * Respond to commands over a websocket to relay UDP commands to a local program
 */

var socketio = require('socket.io');
var fs   = require('fs');
var io;

var dgram = require('dgram');

exports.listen = function(server) {
	io = socketio.listen(server);
	io.set('log level 1');

	io.sockets.on('connection', function(socket) {
		handleCommand(socket);
	});
};

function handleCommand(socket) {
	// Pased string of comamnd to relay
	socket.on('beatbox', function(data) {
		console.log('beatbox command: ' + data);

		// Info for connecting to the local process via UDP
		var PORT = 12345;
		var HOST = '127.0.0.1';
		var buffer = new Buffer(data);

		var client = dgram.createSocket('udp4');
		client.send(buffer, 0, buffer.length, PORT, HOST, function(err, bytes) {
			if (err) 
				throw err;
		});

		client.on('listening', function () {
			var address = client.address();
		});

		var timer = setTimeout(function() {
			socket.emit("bbclose");
		}, 1000);

		// Handle an incoming message over the UDP from the local application.
		client.on('message', function (message, remote) {
			var reply = message.toString('utf8')
			socket.emit('commandReply', reply);
			clearTimeout(timer);

			client.close();

		});
		client.on("UDP Client: close", function() {
			socket.emit("bbclose");
		});
		client.on("UDP Client: error", function(err) {
			socket.emit("bbclose");
		});
		
	});

	// Up time (from example)
	socket.on('proc', function(fileName) {
		var absPath = "/proc/" + fileName;	
		fs.exists(absPath, function(exists) {
			if (exists) {
				// Can use 2nd param: 'utf8', 
				fs.readFile(absPath, function(err, fileData) {
					if (err) {
						emitSocketData(socket, fileName, 
								"ERROR: Unable to read file " + absPath);
					} else {
						emitSocketData(socket, fileName, 
								fileData.toString('utf8'));
					}
				});
			} else {
				emitSocketData(socket, fileName, 
						"ERROR: File " + absPath + " not found.");
			}
		});
	});
};

// From example
function emitSocketData(socket, fileName, contents) {
	var result = {
			fileName: fileName,
			contents: contents
	}
	socket.emit('fileContents', result);	
}