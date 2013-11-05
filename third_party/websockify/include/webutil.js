/*
 * from noVNC: HTML5 VNC client
 * Copyright (C) 2012 Joel Martin
 * Licensed under MPL 2.0 (see LICENSE.txt)
 *
 * See README.md for usage and integration instructions.
 */

"use strict";
/*jslint bitwise: false, white: false */
/*global Util, window, document */

// Globals defined here
var WebUtil = {}, $D;

/*
 * Simple DOM selector by ID
 */
if (!window.$D) {
    window.$D = function (id) {
        if (document.getElementById) {
            return document.getElementById(id);
        } else if (document.all) {
            return document.all[id];
        } else if (document.layers) {
            return document.layers[id];
        }
        return undefined;
    };
}


/* 
 * ------------------------------------------------------
 * Namespaced in WebUtil
 * ------------------------------------------------------
 */

// init log level reading the logging HTTP param
WebUtil.init_logging = function(level) {
    if (typeof level !== "undefined") {
        Util._log_level = level;
    } else {
        Util._log_level = (document.location.href.match(
            /logging=([A-Za-z0-9\._\-]*)/) ||
            ['', Util._log_level])[1];
    }
    Util.init_logging();
};


WebUtil.dirObj = function (obj, depth, parent) {
    var i, msg = "", val = "";
    if (! depth) { depth=2; }
    if (! parent) { parent= ""; }

    // Print the properties of the passed-in object 
    for (i in obj) {
        if ((depth > 1) && (typeof obj[i] === "object")) { 
            // Recurse attributes that are objects
            msg += WebUtil.dirObj(obj[i], depth-1, parent + "." + i);
        } else {
            //val = new String(obj[i]).replace("\n", " ");
            if (typeof(obj[i]) === "undefined") {
                val = "undefined";
            } else {
                val = obj[i].toString().replace("\n", " ");
            }
            if (val.length > 30) {
                val = val.substr(0,30) + "...";
            } 
            msg += parent + "." + i + ": " + val + "\n";
        }
    }
    return msg;
};

// Read a query string variable
WebUtil.getQueryVar = function(name, defVal) {
    var re = new RegExp('[?][^#]*' + name + '=([^&#]*)'),
        match = document.location.href.match(re);
    if (typeof defVal === 'undefined') { defVal = null; }
    if (match) {
        return decodeURIComponent(match[1]);
    } else {
        return defVal;
    }
};


/*
 * Cookie handling. Dervied from: http://www.quirksmode.org/js/cookies.html
 */

// No days means only for this browser session
WebUtil.createCookie = function(name,value,days) {
    var date, expires;
    if (days) {
        date = new Date();
        date.setTime(date.getTime()+(days*24*60*60*1000));
        expires = "; expires="+date.toGMTString();
    }
    else {
        expires = "";
    }
    document.cookie = name+"="+value+expires+"; path=/";
};

WebUtil.readCookie = function(name, defaultValue) {
    var i, c, nameEQ = name + "=", ca = document.cookie.split(';');
    for(i=0; i < ca.length; i += 1) {
        c = ca[i];
        while (c.charAt(0) === ' ') { c = c.substring(1,c.length); }
        if (c.indexOf(nameEQ) === 0) { return c.substring(nameEQ.length,c.length); }
    }
    return (typeof defaultValue !== 'undefined') ? defaultValue : null;
};

WebUtil.eraseCookie = function(name) {
    WebUtil.createCookie(name,"",-1);
};

/*
 * Setting handling.
 */

WebUtil.initSettings = function(callback) {
    var callbackArgs = Array.prototype.slice.call(arguments, 1);
    if (window.chrome && window.chrome.storage) {
        window.chrome.storage.sync.get(function (cfg) {
            WebUtil.settings = cfg;
            console.log(WebUtil.settings);
            if (callback) {
                callback.apply(this, callbackArgs);
            }
        });
    } else {
        // No-op
        if (callback) {
            callback.apply(this, callbackArgs);
        }
    }
};

// No days means only for this browser session
WebUtil.writeSetting = function(name, value) {
    if (window.chrome && window.chrome.storage) {
        //console.log("writeSetting:", name, value);
        if (WebUtil.settings[name] !== value) {
            WebUtil.settings[name] = value;
            window.chrome.storage.sync.set(WebUtil.settings);
        }
    } else {
        localStorage.setItem(name, value);
    }
};

WebUtil.readSetting = function(name, defaultValue) {
    var value;
    if (window.chrome && window.chrome.storage) {
        value = WebUtil.settings[name];
    } else {
        value = localStorage.getItem(name);
    }
    if (typeof value === "undefined") {
        value = null;
    }
    if (value === null && typeof defaultValue !== undefined) {
        return defaultValue;
    } else {
        return value;
    }
};

WebUtil.eraseSetting = function(name) {
    if (window.chrome && window.chrome.storage) {
        window.chrome.storage.sync.remove(name);
        delete WebUtil.settings[name];
    } else {
        localStorage.removeItem(name);
    }
};

/*
 * Alternate stylesheet selection
 */
WebUtil.getStylesheets = function() { var i, links, sheets = [];
    links = document.getElementsByTagName("link");
    for (i = 0; i < links.length; i += 1) {
        if (links[i].title &&
            links[i].rel.toUpperCase().indexOf("STYLESHEET") > -1) {
            sheets.push(links[i]);
        }
    }
    return sheets;
};

// No sheet means try and use value from cookie, null sheet used to
// clear all alternates.
WebUtil.selectStylesheet = function(sheet) {
    var i, link, sheets = WebUtil.getStylesheets();
    if (typeof sheet === 'undefined') {
        sheet = 'default';
    }
    for (i=0; i < sheets.length; i += 1) {
        link = sheets[i];
        if (link.title === sheet) {    
            Util.Debug("Using stylesheet " + sheet);
            link.disabled = false;
        } else {
            //Util.Debug("Skipping stylesheet " + link.title);
            link.disabled = true;
        }
    }
    return sheet;
};
