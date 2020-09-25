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
	// Assume a server-side error
	var retval = 500;

	// Make sure the client is asking us for a URL we know about
	if(req.url == "/config") {		
		console.log('config request from ' + req.connection.remoteAddress);

		// See if we're asking for a client certificate and, if so, make sure
		// we have a subject
		if(options.requestCert) {
			var cert = req.socket.getPeerCertificate();
			if(cert == undefined) {
				retval = 403;
				console.log('no client certificate');
			}
			else {
				var subject = cert.subject;
		
				if(subject == undefined) {
					retval = 403;
					console.log('no subject in client certificate');
				}
				else {
					retval = 200;
					console.log('   cert: C=' + subject.C 
									+ ', O=' + subject.O
									+ ', L=' + subject.L);
				}
			}
		}
		else {
			retval = 200;
		}
	}
	else {
		retval = 404;
		console.log('unknown url ' + req.url);
	}

	if(retval == 200) {
		res.writeHead(retval, {'content-type': 'application/json'});
		res.end(configContent);
	}
	else {
		res.writeHead(retval);		
		res.end("");
	}
}).listen(port);
 
