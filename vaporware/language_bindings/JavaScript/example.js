#!/usr/bin/env node

'use strict';

const VaporLight = require('./');

// index of the last LED
const LAST_LIGHT = 34;

let vaporLight = new VaporLight();

vaporLight.on('connect', () => {
    vaporLight.authenticate();
    animate();
});


const animate = function () {
    (function tail(start) {
        for (let i=0; i < 8; i++) {
            let color = {
                r: parseInt(255 - (255 / 8) * i, 10),
                g: parseInt((255 / 8) * i, 10),
                b: parseInt(255 - (255 / 8) * i, 10)
            };
            setLedColor(start-i, color);
        }
        vaporLight.strobe();

        setTimeout(() => {
            tail(putLedInRange(start+1));
        }, 100);
    })(0);
};

const setLedColor = function (led, color) {
    led = putLedInRange(led);
    if (color.a) {
        vaporLight.setRGBA8(led, color.r, color.g, color.b, color.a);
    } else {
        vaporLight.setRGB8(led, color.r, color.g, color.b);
    }
};

const putLedInRange = function (led) {
    while (led < 0) led = LAST_LIGHT + led;
    while (led > LAST_LIGHT) led = led - LAST_LIGHT;
    return led;
};

process.stdin.resume();
const exitHandler = function(options, err) {
    vaporLight.close();
    if (err) console.log(err.stack);
    if (options.exit) process.exit();
};

process.on('exit', exitHandler.bind(null,{cleanup:true}));
process.on('SIGINT', exitHandler.bind(null, {exit:true}));
process.on('uncaughtException', exitHandler.bind(null, {exit:true}));
