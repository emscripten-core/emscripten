/*
 * noVNC: HTML5 VNC client
 * Copyright (C) 2011 Joel Martin
 * Licensed under LGPL-3 (see LICENSE.txt)
 *
 * See README.md for usage and integration instructions.
 */

"use strict";
/*jslint bitwise: false, white: false */
/*global window, console, document, navigator, ActiveXObject */

// Globals defined here
var Util = {};


/*
 * Make arrays quack
 */

Array.prototype.push8 = function (num) {
    this.push(num & 0xFF);
};

Array.prototype.push16 = function (num) {
    this.push((num >> 8) & 0xFF,
              (num     ) & 0xFF  );
};
Array.prototype.push32 = function (num) {
    this.push((num >> 24) & 0xFF,
              (num >> 16) & 0xFF,
              (num >>  8) & 0xFF,
              (num      ) & 0xFF  );
};

// IE does not support map (even in IE9)
//This prototype is provided by the Mozilla foundation and
//is distributed under the MIT license.
//http://www.ibiblio.org/pub/Linux/LICENSES/mit.license
if (!Array.prototype.map)
{
  Array.prototype.map = function(fun /*, thisp*/)
  {
    var len = this.length;
    if (typeof fun != "function")
      throw new TypeError();

    var res = new Array(len);
    var thisp = arguments[1];
    for (var i = 0; i < len; i++)
    {
      if (i in this)
        res[i] = fun.call(thisp, this[i], i, this);
    }

    return res;
  };
}

/* 
 * ------------------------------------------------------
 * Namespaced in Util
 * ------------------------------------------------------
 */

/*
 * Logging/debug routines
 */

Util._log_level = 'warn';
Util.init_logging = function (level) {
    if (typeof level === 'undefined') {
        level = Util._log_level;
    } else {
        Util._log_level = level;
    }
    if (typeof window.console === "undefined") {
        if (typeof window.opera !== "undefined") {
            window.console = {
                'log'  : window.opera.postError,
                'warn' : window.opera.postError,
                'error': window.opera.postError };
        } else {
            window.console = {
                'log'  : function(m) {},
                'warn' : function(m) {},
                'error': function(m) {}};
        }
    }

    Util.Debug = Util.Info = Util.Warn = Util.Error = function (msg) {};
    switch (level) {
        case 'debug': Util.Debug = function (msg) { console.log(msg); };
        case 'info':  Util.Info  = function (msg) { console.log(msg); };
        case 'warn':  Util.Warn  = function (msg) { console.warn(msg); };
        case 'error': Util.Error = function (msg) { console.error(msg); };
        case 'none':
            break;
        default:
            throw("invalid logging type '" + level + "'");
    }
};
Util.get_logging = function () {
    return Util._log_level;
};
// Initialize logging level
Util.init_logging();


// Set configuration default for Crockford style function namespaces
Util.conf_default = function(cfg, api, defaults, v, mode, type, defval, desc) {
    var getter, setter;

    // Default getter function
    getter = function (idx) {
        if ((type in {'arr':1, 'array':1}) &&
            (typeof idx !== 'undefined')) {
            return cfg[v][idx];
        } else {
            return cfg[v];
        }
    };

    // Default setter function
    setter = function (val, idx) {
        if (type in {'boolean':1, 'bool':1}) {
            if ((!val) || (val in {'0':1, 'no':1, 'false':1})) {
                val = false;
            } else {
                val = true;
            }
        } else if (type in {'integer':1, 'int':1}) {
            val = parseInt(val, 10);
        } else if (type === 'func') {
            if (!val) {
                val = function () {};
            }
        }
        if (typeof idx !== 'undefined') {
            cfg[v][idx] = val;
        } else {
            cfg[v] = val;
        }
    };

    // Set the description
    api[v + '_description'] = desc;

    // Set the getter function
    if (typeof api['get_' + v] === 'undefined') {
        api['get_' + v] = getter;
    }

    // Set the setter function with extra sanity checks
    if (typeof api['set_' + v] === 'undefined') {
        api['set_' + v] = function (val, idx) {
            if (mode in {'RO':1, 'ro':1}) {
                throw(v + " is read-only");
            } else if ((mode in {'WO':1, 'wo':1}) &&
                       (typeof cfg[v] !== 'undefined')) {
                throw(v + " can only be set once");
            }
            setter(val, idx);
        };
    }

    // Set the default value
    if (typeof defaults[v] !== 'undefined') {
        defval = defaults[v];
    } else if ((type in {'arr':1, 'array':1}) &&
            (! (defval instanceof Array))) {
        defval = [];
    }
    // Coerce existing setting to the right type
    //Util.Debug("v: " + v + ", defval: " + defval + ", defaults[v]: " + defaults[v]);
    setter(defval);
};

// Set group of configuration defaults
Util.conf_defaults = function(cfg, api, defaults, arr) {
    var i;
    for (i = 0; i < arr.length; i++) {
        Util.conf_default(cfg, api, defaults, arr[i][0], arr[i][1],
                arr[i][2], arr[i][3], arr[i][4]);
    }
};


/*
 * Cross-browser routines
 */

// Get DOM element position on page
Util.getPosition = function (obj) {
    var x = 0, y = 0;
    if (obj.offsetParent) {
        do {
            x += obj.offsetLeft;
            y += obj.offsetTop;
            obj = obj.offsetParent;
        } while (obj);
    }
    return {'x': x, 'y': y};
};

// Get mouse event position in DOM element
Util.getEventPosition = function (e, obj, scale) {
    var evt, docX, docY, pos;
    //if (!e) evt = window.event;
    evt = (e ? e : window.event);
    evt = (evt.changedTouches ? evt.changedTouches[0] : evt.touches ? evt.touches[0] : evt);
    if (evt.pageX || evt.pageY) {
        docX = evt.pageX;
        docY = evt.pageY;
    } else if (evt.clientX || evt.clientY) {
        docX = evt.clientX + document.body.scrollLeft +
            document.documentElement.scrollLeft;
        docY = evt.clientY + document.body.scrollTop +
            document.documentElement.scrollTop;
    }
    pos = Util.getPosition(obj);
    if (typeof scale === "undefined") {
        scale = 1;
    }
    return {'x': (docX - pos.x) / scale, 'y': (docY - pos.y) / scale};
};


// Event registration. Based on: http://www.scottandrew.com/weblog/articles/cbs-events
Util.addEvent = function (obj, evType, fn){
    if (obj.attachEvent){
        var r = obj.attachEvent("on"+evType, fn);
        return r;
    } else if (obj.addEventListener){
        obj.addEventListener(evType, fn, false); 
        return true;
    } else {
        throw("Handler could not be attached");
    }
};

Util.removeEvent = function(obj, evType, fn){
    if (obj.detachEvent){
        var r = obj.detachEvent("on"+evType, fn);
        return r;
    } else if (obj.removeEventListener){
        obj.removeEventListener(evType, fn, false);
        return true;
    } else {
        throw("Handler could not be removed");
    }
};

Util.stopEvent = function(e) {
    if (e.stopPropagation) { e.stopPropagation(); }
    else                   { e.cancelBubble = true; }

    if (e.preventDefault)  { e.preventDefault(); }
    else                   { e.returnValue = false; }
};


// Set browser engine versions. Based on mootools.
Util.Features = {xpath: !!(document.evaluate), air: !!(window.runtime), query: !!(document.querySelector)};

Util.Engine = {
    // Version detection break in Opera 11.60 (errors on arguments.callee.caller reference)
    //'presto': (function() {
    //         return (!window.opera) ? false : ((arguments.callee.caller) ? 960 : ((document.getElementsByClassName) ? 950 : 925)); }()),
    'presto': (function() { return (!window.opera) ? false : true; }()),

    'trident': (function() {
            return (!window.ActiveXObject) ? false : ((window.XMLHttpRequest) ? ((document.querySelectorAll) ? 6 : 5) : 4); }()),
    'webkit': (function() {
            try { return (navigator.taintEnabled) ? false : ((Util.Features.xpath) ? ((Util.Features.query) ? 525 : 420) : 419); } catch (e) { return false; } }()),
    //'webkit': (function() {
    //        return ((typeof navigator.taintEnabled !== "unknown") && navigator.taintEnabled) ? false : ((Util.Features.xpath) ? ((Util.Features.query) ? 525 : 420) : 419); }()),
    'gecko': (function() {
            return (!document.getBoxObjectFor && window.mozInnerScreenX == null) ? false : ((document.getElementsByClassName) ? 19 : 18); }())
};
if (Util.Engine.webkit) {
    // Extract actual webkit version if available
    Util.Engine.webkit = (function(v) {
            var re = new RegExp('WebKit/([0-9\.]*) ');
            v = (navigator.userAgent.match(re) || ['', v])[1];
            return parseFloat(v, 10);
        })(Util.Engine.webkit);
}

Util.Flash = (function(){
    var v, version;
    try {
        v = navigator.plugins['Shockwave Flash'].description;
    } catch(err1) {
        try {
            v = new ActiveXObject('ShockwaveFlash.ShockwaveFlash').GetVariable('$version');
        } catch(err2) {
            v = '0 r0';
        }
    }
    version = v.match(/\d+/g);
    return {version: parseInt(version[0] || 0 + '.' + version[1], 10) || 0, build: parseInt(version[2], 10) || 0};
}()); 
