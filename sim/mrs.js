//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

const https = require('https');
const fs = require('fs');
const { exit } = require('process');

console.log('---------------------------------------------------------------------------');
console.log('Magellan REST Server Simulator (mrs) version 0.1');
console.log('');
console.log('Copyright (c) 2020 Rally Tactical Systems, Inc.');
console.log('---------------------------------------------------------------------------');

var port = 8081;

console.log('Listening on port ' + port + '. Press Ctrl-C to stop.');

// This is the JSON content to be returned for REST "/config" requests
var configContent = fs.readFileSync('./config.json');

// Our web server will provide this cert to authenticate itself to the client
const options = {
	cert: fs.readFileSync('../certs/server.crt'),
	key: fs.readFileSync('../certs/server.key'),
	ca: fs.readFileSync('../certs/ca.crt'),
  	requestCert: true,
	rejectUnauthorized: true,
};

// This is our little REST server.  It responds only to the "/config" REST request by
// returning the "configContent" from above
https.createServer(options, function (req, res) {
	var cert = req.socket.getPeerCertificate();
	if(cert != undefined) {
		var subject = cert.subject;

		if(subject != undefined) {
			if(req.url == "/config") {		
				console.log('config request from ' + req.connection.remoteAddress);
				res.writeHead(200, {'content-type': 'application/json'});
			  	res.end(configContent);
		  }
		  else {
			console.log('unknown url ' + req.url);
			  res.writeHead(404);
		  }	  
		}
		else {
			console.log('no subject in client certificate');
			res.writeHead(403);
		}
	}
	else {
		console.log('no client certificate');
		res.writeHead(403);
	}
}).listen(port);
 
