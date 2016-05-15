"use strict";

const net = require('net');

const DEFAULT_HOST = "vaporlight.club.entropia.de";
const DEFAULT_PORT = 7534;
const DEFAULT_TOKEN = "sixteen letters.";
const ALPHA_VISIBLE = 255;

const OP_AUTH = 0x02;
const OP_RGB8 = 0x01;
const OP_RGB16 = 0x03;
const OP_STROBE = 0xFF;

const VaporLight =  function (host, port, token) {
    this.host = port || process.env.HOSTNAME || DEFAULT_HOST;
    this.port = host || process.env.PORT || DEFAULT_PORT;
    this.token = token || DEFAULT_TOKEN;

    this.socket = null;

    this.connect();
};

VaporLight.prototype.connect = function (callback) {
    this.socket = new net.Socket();
    this.socket.connect(this.port, this.host, callback);
};

VaporLight.prototype.close = function () {
    this.socket.end();
    this.socket = null;
};

/**
 * On event
 * Event hast to be one of:
 * 'close', 'connect', 'data', 'drain', 'end', 'error', 'lookup' or 'timeout'
 * @param  {[type]}   event    - name of the event
 * @param  {Function} callback - event callback
 */
VaporLight.prototype.on = function (event, callback) {
    this.socket.on(event, callback);
};

VaporLight.prototype.authenticate = function(callback) {
	console.log("Try authenticate with '%s'", this.token);

    this.socket.write(Buffer.from([OP_AUTH]), () => {
        this.socket.write(this.token, "ascii", () => {
            callback();
        });
    });
};

VaporLight.prototype.strobe = function (callback) {
    this.socket.write(Buffer.from([OP_STROBE]), callback);
};

VaporLight.prototype.setRGB8 = function (led, r, g, b, callback) {
    this.setRGBA8(led, r, g, b, ALPHA_VISIBLE, callback);
};

VaporLight.prototype.setRGBA8 = function(led, r, g, b, a, callback) {
    let buffer = Buffer.alloc(7);
    buffer.writeUInt8(OP_RGB8, 0);
    buffer.writeUInt16BE(led, 1);
    buffer.writeUInt8(r, 3);
    buffer.writeUInt8(g, 4);
    buffer.writeUInt8(b, 5);
    buffer.writeUInt8(a, 6);
    this.socket.write(buffer, callback);
};

VaporLight.prototype.setRGB16 = function (led, r, g, b, callback) {
    this.setRGBA16(led, r, g, b, ALPHA_VISIBLE, callback);
};

VaporLight.prototype.setRGBA16 = function (led, r, g, b, a, callback) {
    let buffer = Buffer.alloc(11);
    buffer.writeUInt8(OP_RGB16, 0);
    buffer.writeUInt16BE(led, 1);
    buffer.writeUInt16BE(r, 3);
    buffer.writeUInt16BE(g, 5);
    buffer.writeUInt16BE(b, 7);
    buffer.writeUInt16BE(a, 9);
    this.socket.write(buffer, callback);
};

module.exports = VaporLight;
