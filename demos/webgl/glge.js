/*
GLGE WebGL Graphics Engine
Copyright (c) 2010, Paul Brunt
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of GLGE nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL PAUL BRUNT BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/**
 * @fileOverview
 * @name glge.js
 * @author me@paulbrunt.co.uk
 */



 if(!window["GLGE"]){
	/**
	* @namespace Holds the functionality of the library
	*/
	window["GLGE"]={};
}

(function(GLGE){


/**
* Function to augment one object with another
* @param {object} obj1 Source Object
* @param {object} obj2 Destination Object
*/
GLGE.augment=function(obj1,obj2){
	for(proto in obj1.prototype){
		obj2.prototype[proto]=obj1.prototype[proto];
	}
}


/**
* Moves all GLGE function to global
**/
GLGE.makeGlobal=function(){
	for(var key in GLGE){
		window[key]=GLGE[key];
	}
}

GLGE.New=function(createclass){
	if(GLGE[createclass].prototype.className!=""){
		return new GLGE[createclass]();
	}else{
		return false;
	}
}

/**
* @constant 
* @description Enumeration for TRUE
*/
GLGE.TRUE=1;
/**
* @constant 
* @description Enumeration for FALSE
*/
GLGE.FALSE=0;


/**
* @constant 
* @description Enumeration for tri rendering
*/
GLGE.DRAW_TRIS=1;
/**
* @constant 
* @description Enumeration for line rendering
*/
GLGE.DRAW_LINES=2;

/**
* @constant 
* @description Enumeration for line loop rendering
*/
GLGE.DRAW_LINELOOPS=3;
/**
* @constant 
* @description Enumeration for line loop rendering
*/
GLGE.DRAW_LINESTRIPS=4;
/**
* @constant 
* @description Enumeration for point rendering
*/
GLGE.DRAW_POINTS=5;


/**
* @constant 
* @description Enumeration for rendering using default shader
*/
GLGE.RENDER_DEFAULT=0;

/**
* @constant 
* @description Enumeration for rendering using shadow shader
*/
GLGE.RENDER_SHADOW=1;

/**
* @constant 
* @description Enumeration for rendering using pick shader
*/
GLGE.RENDER_PICK=2;

/**
* @constant 
* @description Enumeration for rendering using normal shader
*/
GLGE.RENDER_NORMAL=3;

/**
* @constant 
* @description Enumeration for no rendering
*/
GLGE.RENDER_NULL=4;

/**
* @constant 
* @description Enumeration for box bound text picking
*/
GLGE.TEXT_BOXPICK=1;
/**
* @constant 
* @description Enumeration for text bound text picking
*/
GLGE.TEXT_TEXTPICK=1;

/**
* @constant 
* @description Enumeration for euler rotaions mode
*/
GLGE.P_EULER=1;

/**
* @constant 
* @description Enumeration for quaternions mode
*/
GLGE.P_QUAT=2;

/**
* @constant 
* @description Enumeration for matrix rotation mode
*/
GLGE.P_MATRIX=3;

/**
* @constant 
* @description Enumeration for no value
*/
GLGE.NONE=0;

/**
* @constant 
* @description Enumeration for X-Axis
*/
GLGE.XAXIS=1;
/**
* @constant 
* @description Enumeration for Y-Axis
*/
GLGE.YAXIS=2;
/**
* @constant 
* @description Enumeration for Z-Axis
*/
GLGE.ZAXIS=3;

/**
* @constant 
* @description Enumeration for +X-Axis
*/
GLGE.POS_XAXIS=1;
/**
* @constant 
* @description Enumeration for -X-Axis
*/
GLGE.NEG_XAXIS=2;
/**
* @constant 
* @description Enumeration for +Y-Axis
*/
GLGE.POS_YAXIS=3;
/**
* @constant 
* @description Enumeration for -Y-Axis
*/
GLGE.NEG_YAXIS=4;
/**
* @constant 
* @description Enumeration for +Z-Axis
*/
GLGE.POS_ZAXIS=5;
/**
* @constant 
* @description Enumeration for -Z-Axis
*/
GLGE.NEG_ZAXIS=6;

/**
* @constant 
* @description Linear blending function
*/
GLGE.LINEAR_BLEND=function(value){
	return value;
}
/**
* @constant 
* @description Quadratic blending function
*/
GLGE.QUAD_BLEND=function(value){
	return value*value;
}
/**
* @constant 
* @description Special blending function
*/
GLGE.SPECIAL_BLEND=function(value){
	value=value*(2-value);
	return value*value;
}


GLGE.error=function(error){
	alert(error);
}

/**
* @namespace Holds the global asset store
*/
GLGE.Assets={};
GLGE.Assets.assets={};
 
GLGE.Assets.createUUID=function(){
	var data=["0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F"];
	var data2=["8","9","A","B"];
	uuid="";
	for(var i=0;i<38;i++){
		switch(i){
			case 8:uuid=uuid+"-";break;
			case 13:uuid=uuid+"-";break;
			case 18:uuid=uuid+"-";break;
			case 14:uuid=uuid+"4";break;
			case 19:uuid=uuid+data2[Math.round(Math.random()*3)];break;
			default:uuid=uuid+data[Math.round(Math.random()*15)];break;
		}
	}
	return uuid;
}
/**
* @function registers a new asset
*/
GLGE.Assets.registerAsset=function(obj,uid){
	if(!uid){
		uid=GLGE.Assets.createUUID();
	};
	obj.uid=uid;
	GLGE.Assets.assets[uid]=obj;
}
/**
* @function removes an asset
*/
GLGE.Assets.unregisterAsset=function(uid){
	delete GLGE.Assets.assets[uid];
}
/**
* @function finds an asset by uid
*/
GLGE.Assets.get=function(uid){
	var value=GLGE.Assets.assets[uid];
	if(value){
		return value;
	}else{
		return false;
	}
}

/**
* @function hashing function
* @private
*/
GLGE.fastHash=function(str){
	var s1=0;var s2=0;var s3=0;var s4=0;var s5=0;var s6=0;
	var c1=0;var c2=0;var c3=0;var c4=0;var c5=0;var c6=0;
	var i=0;
	var length=str.length;
	str+="000000";
	while(i<length){
		c1=str.charCodeAt(i++);c2=str.charCodeAt(i++);c3=str.charCodeAt(i++);
		c4=str.charCodeAt(i++);c5=str.charCodeAt(i++);c6=str.charCodeAt(i++);
		s1=(s5+c1+c2)%255;s2=(s6+c2+c3)%255;s3=(s1+c3+c4)%255;
		s4=(s2+c4+c5)%255;s5=(s3+c5+c6)%255;s6=(s4+c6+c1)%255;
	}
	var r=[String.fromCharCode(s1),String.fromCharCode(s2),String.fromCharCode(s3),
		String.fromCharCode(s4),String.fromCharCode(s5),String.fromCharCode(s6)];
	return r.join('');
}
/**
* @function check if shader is already created if not then create it
* @private
*/
GLGE.getGLShader=function(gl,type,str){
	var hash=GLGE.fastHash(str);
	if(!gl.shaderCache) gl.shaderCache={};
	if(!gl.shaderCache[hash]){
		var shader=gl.createShader(type);
		gl.shaderSource(shader, str);
		gl.compileShader(shader);
		if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
		      alert(gl.getShaderInfoLog(shader));
		      return;
		}
		gl.shaderCache[hash]=shader;
	}
	return gl.shaderCache[hash];
}

/**
* @function tries to re use programs
* @private
*/
GLGE.getGLProgram=function(gl,vShader,fShader){
	if(!gl.programCache) gl.programCache=[];
	var programCache=gl.programCache;
	for(var i=0; i<programCache.length;i++){
		if(programCache[i].fShader==fShader && programCache[i].vShader==vShader){
			return programCache[i].program;
		}
	}
	var program=gl.createProgram();
	gl.attachShader(program, vShader);
	gl.attachShader(program, fShader);
	gl.linkProgram(program);
	programCache.push({vShader:vShader,fShader:fShader,program:program});
	if(!program.uniformDetails){
		program.uniformDetails={};
		var uniforms = gl.getProgramParameter(program, gl.ACTIVE_UNIFORMS);
		for (var i=0;i<uniforms;++i) {
			var info=gl.getActiveUniform(program, i);
			program.uniformDetails[info.name]={loc:GLGE.getUniformLocation(gl,program,info.name),info:info};
		}
	}
	return program;
}


/**
* function to cache the uniform locations
* @param {glcontext} the gl context of the program
* @param {program} the shader program
* @param {string} the uniform name
* @private
*/
GLGE.getUniformLocation=function(gl,program, uniform){
	/*if(program.uniformDetails[uniform]){
		return program.uniformDetails[uniform].loc;
	}else{
		return gl.getUniformLocation(program, uniform);
	}*/
	if(!program.uniformCache) program.uniformCache={};
	if(!program.uniformChecked) program.uniformChecked={};
	if(!program.uniformChecked[uniform]){
		program.uniformCache[uniform]=gl.getUniformLocation(program, uniform);
		program.uniformChecked[uniform]=true;
	}
	return program.uniformCache[uniform];
};
/**
* function to cache the attribute locations
* @param {glcontext} the gl context of the program
* @param {program} the shader program
* @param {string} the attribe name
* @private
*/
GLGE.getAttribLocation=function(gl,program, attrib){
	if(!program.attribCache) program.attribCache={};
	if(!program.attribCache[attrib]){
		program.attribCache[attrib]=gl.getAttribLocation(program, attrib);
	}
	return program.attribCache[attrib];
}

/**
* @class class to implelemnt quick notation
*/
GLGE.QuickNotation=function(){
}
/**
* Call to set properties and add children to an object
* @example myObject._({LocX:10,LocY:20},child1,child2,.....);
*/
GLGE.QuickNotation.prototype._=function(){
	var argument;
	for(var i=0; i<arguments.length;i++){
		argument=arguments[i];
		if(typeof argument=="object"){
			if(argument.className && this["add"+argument.className]){
				this["add"+argument.className](argument);
			}else{
				for(var key in argument){
					if(this["set"+key]){
						this["set"+key](argument[key]);
					}
				}
			}
		}
	}
	return this;
}

/**
* @namespace GLGE Messaging System
*/
GLGE.Message={};
/**
* @function parses messages and updates the scene graph
*/
GLGE.Message.parseMessage=function(msg){
	switch(msg.command){
		case "create":
			var obj=new GLGE[msg.type](msg.uid);
			this.setAttributes(obj,msg.attributes);
			if(msg.children) GLGE.Message.addChildren(obj,msg.children);
			return obj;
			break;
		case "update":
			var obj=GLGE.Assets.get(msg.uid);
			this.setAttributes(obj,msg.attributes);
			if(msg.add) GLGE.Message.addChildren(obj,msg.add);
			if(msg.remove) GLGE.Message.removeChildren(obj,msg.remove);
			return obj;
			break;
	}
	return null;
}
/**
* @function parses the attributes from a message
* @private
*/
GLGE.Message.setAttributes=function(obj,attribs){
	if(attribs){
		for(var attrib in attribs){
			if(obj["set"+attrib]){
				//check to see if the attribute has to be parsed as a message
				if(attribs[attrib].command){
					attribs[attrib]=GLGE.Message.parseMessage(attribs[attrib]);
				}
				obj["set"+attrib](attribs[attrib]);
			}
		}
	}
	return this;
}
/**
* @function parses the children to add
* @private
*/
GLGE.Message.addChildren=function(obj,children){
	if(!(children instanceof Array)) children=[children];
	for(var i=0;i<children.length;i++){
		if(children[i].command){
			var asset=GLGE.Message.parseMessage(children[i]);
		}else{
			var asset=GLGE.Assets.get(children[i]);
		}
		obj["add"+asset.className](asset);
	}
}
/**
* @function parses the children to remove
* @private
*/
GLGE.Message.removeChildren=function(obj,children){
	if(!(children instanceof Array)) children=[children];
	for(var i=0;i<children.length;i++){
		var asset=GLGE.Assets.get(children[i]);
		obj["add"+asset.className](asset);
	}
}

GLGE.Message.toLoad=[];
GLGE.Message.messageLoader=function(url,callback,priority){
	GLGE.Message.toLoad.push([url,callback,priority]);
	if(GLGE.Message.toLoad.length==1) GLGE.Message.loadMessages();
}
GLGE.Message.loadMessages=function(){
	//TODO: use priority
	var nextDoc=GLGE.Message.toLoad.pop();
	var req=new XMLHttpRequest();
	req.onreadystatechange = function() {
		if(this.readyState  == 4){
			if(this.status  == 200 || this.status==0){
				nextDoc[1](this.responseText);
			}else{ 
				GLGE.error("Error loading Document: "+nextDoc[0]+" status "+this.status);
			}
		}
	}
	req.open("GET", nextDoc[0], true);
	req.send("");
	if(GLGE.Message.toLoad.length>0) GLGE.Message.loadMessages();
}



/**
* function to parse a colour input into RGB eg #ff00ff, red, rgb(100,100,100)
* @param {string} color the color to parse
*/
GLGE.colorParse=function(color){
	var red,green,blue,alpha;
	//defines the color names
	var color_names = {
		aliceblue: 'f0f8ff',		antiquewhite: 'faebd7',	aqua: '00ffff',
		aquamarine: '7fffd4',	azure: 'f0ffff',		beige: 'f5f5dc',
		bisque: 'ffe4c4',		black: '000000',		blanchedalmond: 'ffebcd',
		blue: '0000ff',			blueviolet: '8a2be2',	brown: 'a52a2a',
		burlywood: 'deb887',	cadetblue: '5f9ea0',		chartreuse: '7fff00',
		chocolate: 'd2691e',		coral: 'ff7f50',		cornflowerblue: '6495ed',
		cornsilk: 'fff8dc',		crimson: 'dc143c',		cyan: '00ffff',
		darkblue: '00008b',		darkcyan: '008b8b',		darkgoldenrod: 'b8860b',
		darkgray: 'a9a9a9',		darkgreen: '006400',	darkkhaki: 'bdb76b',
		darkmagenta: '8b008b',	darkolivegreen: '556b2f',	darkorange: 'ff8c00',
		darkorchid: '9932cc',	darkred: '8b0000',		darksalmon: 'e9967a',
		darkseagreen: '8fbc8f',	darkslateblue: '483d8b',	darkslategray: '2f4f4f',
		darkturquoise: '00ced1',	darkviolet: '9400d3',	deeppink: 'ff1493',
		deepskyblue: '00bfff',	dimgray: '696969',		dodgerblue: '1e90ff',
		feldspar: 'd19275',		firebrick: 'b22222',		floralwhite: 'fffaf0',
		forestgreen: '228b22',	fuchsia: 'ff00ff',		gainsboro: 'dcdcdc',
		ghostwhite: 'f8f8ff',		gold: 'ffd700',			goldenrod: 'daa520',
		gray: '808080',		green: '008000',		greenyellow: 'adff2f',
		honeydew: 'f0fff0',		hotpink: 'ff69b4',		indianred : 'cd5c5c',
		indigo : '4b0082',		ivory: 'fffff0',		khaki: 'f0e68c',
		lavender: 'e6e6fa',		lavenderblush: 'fff0f5',	lawngreen: '7cfc00',
		lemonchiffon: 'fffacd',	lightblue: 'add8e6',		lightcoral: 'f08080',
		lightcyan: 'e0ffff',		lightgoldenrodyellow: 'fafad2',	lightgrey: 'd3d3d3',
		lightgreen: '90ee90',	lightpink: 'ffb6c1',		lightsalmon: 'ffa07a',
		lightseagreen: '20b2aa',	lightskyblue: '87cefa',	lightslateblue: '8470ff',
		lightslategray: '778899',	lightsteelblue: 'b0c4de',	lightyellow: 'ffffe0',
		lime: '00ff00',			limegreen: '32cd32',	linen: 'faf0e6',
		magenta: 'ff00ff',		maroon: '800000',		mediumaquamarine: '66cdaa',
		mediumblue: '0000cd',	mediumorchid: 'ba55d3',	mediumpurple: '9370d8',
		mediumseagreen: '3cb371',	mediumslateblue: '7b68ee',	mediumspringgreen: '00fa9a',
		mediumturquoise: '48d1cc',	mediumvioletred: 'c71585',	midnightblue: '191970',
		mintcream: 'f5fffa',	mistyrose: 'ffe4e1',		moccasin: 'ffe4b5',
		navajowhite: 'ffdead',	navy: '000080',		oldlace: 'fdf5e6',
		olive: '808000',		olivedrab: '6b8e23',		orange: 'ffa500',
		orangered: 'ff4500',	orchid: 'da70d6',		palegoldenrod: 'eee8aa',
		palegreen: '98fb98',		paleturquoise: 'afeeee',	palevioletred: 'd87093',
		papayawhip: 'ffefd5',	peachpuff: 'ffdab9',		peru: 'cd853f',
		pink: 'ffc0cb',		plum: 'dda0dd',		powderblue: 'b0e0e6',
		purple: '800080',		red: 'ff0000',		rosybrown: 'bc8f8f',
		royalblue: '4169e1',		saddlebrown: '8b4513',	salmon: 'fa8072',
		sandybrown: 'f4a460',	seagreen: '2e8b57',		seashell: 'fff5ee',
		sienna: 'a0522d',		silver: 'c0c0c0',		skyblue: '87ceeb',
		slateblue: '6a5acd',		slategray: '708090',	snow: 'fffafa',
		springgreen: '00ff7f',	steelblue: '4682b4',		tan: 'd2b48c',
		teal: '008080',		thistle: 'd8bfd8',		tomato: 'ff6347',
		turquoise: '40e0d0',		violet: 'ee82ee',		violetred: 'd02090',
		wheat: 'f5deb3',		white: 'ffffff',		whitesmoke: 'f5f5f5',
		yellow: 'ffff00',		yellowgreen: '9acd32'
	};
	if(color_names[color]) color="#"+color_names[color];
	if(color.substr && color.substr(0,1)=="#"){
		color=color.substr(1);
		if(color.length==8){
			red=parseInt("0x"+color.substr(0,2))/255;
			green=parseInt("0x"+color.substr(2,2))/255;
			blue=parseInt("0x"+color.substr(4,2))/255;
			alpha=parseInt("0x"+color.substr(6,2))/255;
		}else if(color.length==4){
			red=parseInt("0x"+color.substr(0,1))/15;
			green=parseInt("0x"+color.substr(1,1))/15;
			blue=parseInt("0x"+color.substr(2,1))/15;
			alpha=parseInt("0x"+color.substr(3,1))/15;
		}else if(color.length==6){
			red=parseInt("0x"+color.substr(0,2))/255;
			green=parseInt("0x"+color.substr(2,2))/255;
			blue=parseInt("0x"+color.substr(4,2))/255;
			alpha=1;
		}else if(color.length==3){
			red=parseInt("0x"+color.substr(0,1))/15;
			green=parseInt("0x"+color.substr(1,1))/15;
			blue=parseInt("0x"+color.substr(2,1))/15;
			alpha=1;
		}
	}else if(color.substr && color.substr(0,4)=="rgb("){
		var colors=color.substr(4).split(",");
		red=parseInt(colors[0])/255;
		green=parseInt(colors[1])/255;
		blue=parseInt(colors[2])/255;
		alpha=1;
	}else if(color.substr && color.substr(0,5)=="rgba("){
		var colors=color.substr(4).split(",");
		red=parseInt(colors[0])/255;
		green=parseInt(colors[1])/255;
		blue=parseInt(colors[2])/255;
		alpha=parseInt(colors[3])/255;
	}else{
		red=0;
		green=0;
		blue=0;
		alpha=0;
	}
	return {r:red,g:green,b:blue,a:alpha};
}


/**
* @class A class to load json fragments from remote location or string
**/
GLGE.JSONLoader=function(){
}
GLGE.JSONLoader.prototype.downloadPriority=0;
/**
* Loads a json fragment from a url
* @param {string} url The URL to load
**/
GLGE.JSONLoader.prototype.setJSONSrc=function(url){
	var GLGEObj=this;
	GLGE.Message.messageLoader(url,function(text){
		GLGEObj.setJSONString(text);
	},this.downloadPriority);
}
/**
* Loads a json fragment from a string
* @param {string} string The URL to load
**/
GLGE.JSONLoader.prototype.setJSONString=function(string){
	var message = JSON.parse(string);
	//check to make sure this is the correct class type
	if(message.type==this.className){
		message.uid=this.uid;
		//we don't want to create a new one we want to update this one
		message.command="update";
		GLGE.Message.parseMessage(message);
	}
}
/**
* Sets the download priority
* @param {number} value The download priority
**/
GLGE.JSONLoader.prototype.setDownloadPriority=function(value){
	this.downloadPriority=value;
}
/**
* Gets the download priority
* @returns {number} The download priority
**/
GLGE.JSONLoader.prototype.getDownloadPriority=function(){
	return this.downloadPriority;
}


/**
* @class A events class
**/
GLGE.Events=function(){
}
/**
* Fires an event
* @param {string} event The name of the event to fire
* @param {object} data the events data
**/
GLGE.Events.prototype.fireEvent=function(event,data){
	if(this.events && this.events[event]){
		var events=this.events[event];
		for(var i=0;i<events.length;i++){
			events[i].call(this,data);
		}
	}
}
/**
* Adds an event listener
* @param {string} event The name of the event to listen for
* @param {function} fn the event callback
**/
GLGE.Events.prototype.addEventListener=function(event,fn){
	if(!this.events) this.events={};
	if(!this.events[event]) this.events[event]=[];
	this.events[event].push(fn);
}
/**
* Removes an event listener
* @param {function} fn the event callback to remove
**/
GLGE.Events.prototype.removeEventListener=function(event,fn){
	var idx=this.events[event].indexOf(fn);
	if(idx!=-1) this.events[event].splice(idx,1);
}

/**
* @class Document class to load scene, object, mesh etc from an external XML file 
* @param {string} url URL of the resource to load
*/
GLGE.Document=function(){
	this.listeners=[];
	this.documents=[];
}
GLGE.Document.prototype.listeners=null;
GLGE.Document.prototype.documents=null;
GLGE.Document.prototype.rootURL=null;
GLGE.Document.prototype.loadCount=0;
/**
* This is just a fix for a bug in webkit
* @param {string} id the id name to get
* @returns {object} node with teh specified id
* @private
*/
GLGE.Document.prototype.getElementById=function(id){
	var tags=this.getElementsByTagName("*");
	for(var i=0; i<tags.length;i++){
		if(tags[i].getAttribute("id")==id){
			return tags[i];
			break;
		}
	}
	return null;
}
/**
* Gets the absolute path given an import path and the path it's relative to
* @param {string} path the path to get the absolute path for
* @param {string} relativeto the path the supplied path is relativeto
* @returns {string} absolute path
* @private
*/
GLGE.Document.prototype.getAbsolutePath=function(path,relativeto){
	if(path.substr(0,7)=="http://" || path.substr(0,7)=="file://"  || path.substr(0,7)=="https://"){
		return path;
	}
	else
	{
		if(!relativeto){
			relativeto=window.location.href;
		}
		//find the path compoents
		var bits=relativeto.split("/");
		var domain=bits[2];
		var proto=bits[0];
		var initpath=[];
		for(var i=3;i<bits.length-1;i++){
			initpath.push(bits[i]);
		}
		//relative to domain
		if(path.substr(0,1)=="/"){
			initpath=[];
		}
		var locpath=path.split("/");
		for(i=0;i<locpath.length;i++){
			if(locpath[i]=="..") initpath.pop();
				else if(locpath[i]!="") initpath.push(locpath[i]);
		}
		return proto+"//"+domain+"/"+initpath.join("/");
	}
}
/**
* Loads the root document
* @param {string} url URL of the resource to load
*/
GLGE.Document.prototype.load=function(url){
	this.documents=[];
	this.rootURL=url;
	this.loadDocument(url,null);
}
/**
* Loads an additional documents into the collection
* @param {string} url URL of the resource to load
* @param {string} relativeto the path the URL is relative to, null for default
*/
GLGE.Document.prototype.loadDocument=function(url,relativeto){
	this.loadCount++;
	url=this.getAbsolutePath(url,relativeto);
	var req = new XMLHttpRequest();
	if(req) {
		req.docurl=url;
		req.docObj=this;
		req.overrideMimeType("text/xml");
		req.onreadystatechange = function() {
			if(this.readyState  == 4)
			{
				if(this.status  == 200 || this.status==0){
					this.responseXML.getElementById=this.docObj.getElementById;
					this.docObj.loaded(this.docurl,this.responseXML);
				}else{ 
					GLGE.error("Error loading Document: "+this.docurl+" status "+this.status);
				}
			}
		};
		req.open("GET", url, true);
		req.send("");
	}	
}
/**
* Trigered when a document has finished loading
* @param {string} url the absolute url of the document that has loaded
* @param {XMLDoc} responceXML the xml document that has finished loading
* @private
*/
GLGE.Document.prototype.loaded=function(url,responceXML){
	this.loadCount--;
	this.documents[url]={xml:responceXML};
	var imports=responceXML.getElementsByTagName("import");
	for(var i=0; i<imports.length;i++){
		if(!this.documents[this.getAbsolutePath(imports[i].getAttribute("url"),url)]){
			this.documents[this.getAbsolutePath(imports[i].getAttribute("url"),url)]={};
			this.loadDocument(imports[i].getAttribute("url"),url);
		}
	}
	if(this.loadCount==0){
		this.finishedLoading();
	}
}
/**
* Called when all documents have finished loading
* @private
*/
GLGE.Document.prototype.finishedLoading=function(){
	for(var i=0; i<this.listeners.length;i++){
		this.listeners[i](this.listeners.rootURL);
	}
	this["onLoad"]();
}
/**
* Called when all documents have finished loading
* @event
*/
GLGE.Document.prototype["onLoad"]=function(){};
/**
* Converts and attribute name into a class name
* @param {string} name attribute name to convert
* @private
*/
GLGE.Document.prototype.classString=function(name){
	if(!name) return false;
	var names=name.split("_");
	var converted="";
	for(var i=0;i<names.length;i++){
		converted=converted+names[i][0].toUpperCase()+names[i].substr(1);
	}
	return converted;
}
/**
* Sets the properties of an object based on the attributes of the corresponding dom element
* @param {object} Obj the DOM element to apply the attributes of
* @private
*/
GLGE.Document.prototype.setProperties=function(Obj){
	var set_method;
	var attribute_name;
	var value;
	for(var i=0; i<Obj.attributes.length; i++){
		value=false;
		set_method="set"+this.classString(Obj.attributes[i].nodeName);

		if(Obj.attributes[i].value[0]=="#"){
			value=this.getElement(Obj.attributes[i].value.substr(1),true);
		}
		if(!value){
			//if this is a GLGE contsant then set the constant value otherwise just literal
			if(typeof(GLGE[Obj.attributes[i].value]) != "undefined"){
				value=GLGE[Obj.attributes[i].value];
			}
			else
			{
				value=Obj.attributes[i].value;
			}
		}
		
		if(Obj.object[set_method]) Obj.object[set_method](value);
		//if a uid is set in the xml doc then make sure it's registered correctly in the assets
		if(Obj.attributes[i].nodeName=="uid"){
			GLGE.Assets.unregisterAsset(Obj.object.uid);
			Obj.object.uid=Obj.attributes[i].value;
			GLGE.Assets.registerAsset(Obj.object,Obj.attributes[i].value);
		}
	}
}
/**
* Adds child objects 
* @param {object} Obj the DOM element to apply the children of
* @private
*/
GLGE.Document.prototype.addChildren=function(Obj){
	//loop though and add the children
	var add_method;
	var child=Obj.firstChild;
	while(child){
		add_method="add"+this.classString(child.tagName);
		if(Obj.object[add_method]) Obj.object[add_method](this.getElement(child));
		child=child.nextSibling;
	}
}
/**
* Gets an object from the XML document based on the dom element 
* @param {string|domelement} ele the id of the element to get or the dom node
*/
GLGE.Document.prototype.getElement=function(ele,noerrors){
	var docele,doc;
	if(typeof(ele)=="string"){
		for(doc in this.documents){
			if(this.documents[doc].xml){
				docele=this.documents[doc].xml.getElementById(ele);
				if(docele){
					ele=docele;
					break;
				}
			}
		}
	}
	if(typeof(ele)=="string"){
		//if element is still a string at this point there there is an issue
		if(!noerrors) GLGE.error("Element "+ele+" not found in document");
		return false;
	}
	else
	{
		if(this["get"+this.classString(ele.tagName)]){
			return this["get"+this.classString(ele.tagName)](ele);
		}
		else
		{
			return this.getDefault(ele);
		}
	}
}
/**
* Parses the dom element and creates any objects that are required
* @param {domelement} ele the element to create the objects from
* @private
*/
GLGE.Document.prototype.getDefault=function(ele){
	if(!ele.object){
		if(GLGE[this.classString(ele.tagName)]){
			ele.object=new GLGE[this.classString(ele.tagName)]();
			this.setProperties(ele);
			this.addChildren(ele);
		}
		else
		{
			GLGE.error("XML Parse Error: GLGE Object not found"); 
		}
	}
	return ele.object;
}
/**
* Parses the dom element and creates a texture
* @param {domelement} ele the element to create the objects from
* @private
*/
GLGE.Document.prototype.getTexture=function(ele){
	if(!ele.object){
		var rel=this.getAbsolutePath(this.rootURL,null);
		ele.object=new GLGE[this.classString(ele.tagName)];
		ele.object.setSrc(this.getAbsolutePath(ele.getAttribute("src"),rel));
		ele.removeAttribute("src");
		this.setProperties(ele);
	}
	return ele.object;
}
GLGE.Document.prototype.getTextureVideo=GLGE.Document.prototype.getTexture;

/**
* Parses a document node into an array
* @param {node} the node to parse
* @private
*/
GLGE.Document.prototype.parseArray=function(node){
	var child=node.firstChild;
	var prev="";
	var output=[];
	var currentArray;
	var i;
	while(child){
		currentArray=(prev+child.nodeValue).split(",");
		child=child.nextSibling;
		if(currentArray[0]=="") currentArray.unshift();
		if(child) prev=currentArray.pop();
		for(i=0;i<currentArray.length;i++) output.push(currentArray[i]);
	}
	return output;
}

/**
* Parses the mesh dom to create the mesh object
* @param {domelement} ele the element to create the mesh from
* @private
*/
GLGE.Document.prototype.getMesh=function(ele){
	if(!ele.object){
		ele.object=new GLGE.Mesh();
		this.setProperties(ele);
		var child=ele.firstChild;
		while(child){
			switch(child.tagName){
				case "positions":
					ele.object.setPositions(this.parseArray(child));
					break;
				case "normals":
					ele.object.setNormals(this.parseArray(child));
					break;				
				case "uv1":
					ele.object.setUV(this.parseArray(child));
					break;				
				case "uv2":
					ele.object.setUV2(this.parseArray(child));
					break;
				case "faces":
					ele.object.setFaces(this.parseArray(child));
					break;
				case "joint_names":
					var names=this.parseArray(child);
					var jointObjects=[];
					for(var i=0;i<names.length;i++){
						if(names[i].substr(0,1)=="#"){
							jointObjects.push(this.getElement(names[i].substr(1)));
						}else{
							jointObjects.push(names[i]);
						}
					}
					ele.object.setJoints(jointObjects);
					break;
				case "bind_matrix":
					var mats=this.parseArray(child);
					var invBind=[];
					for(var i=0;i<mats.length;i++){
						invBind.push(GLGE.Mat4(mats[i].split(" ")));
					}
					ele.object.setInvBindMatrix(invBind);
					break;
				case "joints":
					ele.object.setVertexJoints(this.parseArray(child),child.getAttribute("count"));
					break;
				case "weights":
					ele.object.setVertexWeights(this.parseArray(child),child.getAttribute("count"));
					break;
			}
			child=child.nextSibling;
		}
	}
	return ele.object;
}

/**
* Adds a listener to be called when all documents have finished loading
* @param {function} listener the function to call when all loading in complete
*/
GLGE.Document.prototype.addLoadListener=function(listener){
	this.listeners.append(listener);
}
/**
* Removes a load listener
* @param {function} listener Listener to remove
*/
GLGE.Document.prototype.removeLoadListener=function(listener){
	for(var i=0; i<this.listeners.length; i++){
		if(this.listeners[i]===listener) this.listeners.splice(i,1);
	}
}

/**
* loads xml from a script tag
* @param {string} id the id of the element to load
*/
GLGE.Document.prototype.parseScript=function(id){
	this.rootURL=window.location.toString();
	var xmlScript = document.getElementById(id);
	if (!xmlScript) {
		return null;
	}
 
	var str = "";
	var k = xmlScript.firstChild;
	while (k) {
		if (k.nodeType == 3) {
			str += k.textContent;
		}
		k = k.nextSibling;
	}
	
	var parser=new DOMParser();
	var xmlDoc=parser.parseFromString(str,"text/xml");
	xmlDoc.getElementById=this.getElementById;
	
	this.documents["#"+id]={xml:xmlDoc};

	var imports=xmlDoc.getElementsByTagName("import");
	for(var i=0; i<imports.length;i++){
		if(!this.documents[this.getAbsolutePath(imports[i].getAttribute("url"),url)]){
			this.documents[this.getAbsolutePath(imports[i].getAttribute("url"),url)]={};
			this.loadDocument(imports[i].getAttribute("url"));
		}
	}
	if(this.loadCount==0){
		this.finishedLoading();
	}
}



/**
* @class Abstract class to agument objects that requires position, rotation and scale.
*/
GLGE.Placeable=function(){
}
GLGE.Placeable.prototype.locX=0;
GLGE.Placeable.prototype.locY=0;
GLGE.Placeable.prototype.locZ=0;
GLGE.Placeable.prototype.dLocX=0;
GLGE.Placeable.prototype.dLocY=0;
GLGE.Placeable.prototype.dLocZ=0;
GLGE.Placeable.prototype.quatX=0;
GLGE.Placeable.prototype.quatY=0;
GLGE.Placeable.prototype.quatZ=0;
GLGE.Placeable.prototype.quatW=0;
GLGE.Placeable.prototype.rotX=0;
GLGE.Placeable.prototype.rotY=0;
GLGE.Placeable.prototype.rotZ=0;
GLGE.Placeable.prototype.dRotX=0;
GLGE.Placeable.prototype.dRotY=0;
GLGE.Placeable.prototype.dRotZ=0;
GLGE.Placeable.prototype.scaleX=1;
GLGE.Placeable.prototype.scaleY=1;
GLGE.Placeable.prototype.scaleZ=1;
GLGE.Placeable.prototype.dScaleX=0;
GLGE.Placeable.prototype.dScaleY=0;
GLGE.Placeable.prototype.dScaleZ=0;
GLGE.Placeable.prototype.matrix=null;
GLGE.Placeable.prototype.rotOrder=GLGE.ROT_XYZ;
GLGE.Placeable.prototype.lookAt=null;
GLGE.Placeable.prototype.mode=GLGE.P_EULER;



/**
* Gets the root node object
* @returns {object}
*/
GLGE.Placeable.prototype.getRoot=function(){
	if(this.type==GLGE.G_ROOT){
		return this;
	}else if(this.parent){
		var value=this.parent.getRoot();
		if(!value) return this;
			else return value;
	}else{
		return this;
	}
}
/**
* Gets the id string of this text
* @returns {string}
*/
GLGE.Placeable.prototype.getRef=function(){
	if(this.id){
		return this.id;
	}else if(this.parent){
		return this.parent.getRef();
	}else{
		return null;
	}
}
/**
* Sets the id string
* @param {string} id The id string 
*/
GLGE.Placeable.prototype.setId=function(id){
    this.id=id;
    return this;
}
/**
* Gets the id string of this text
* @returns {string}
*/
GLGE.Placeable.prototype.getId=function(){
	return this.id
}
/**
* gets the object or poisition being looking at
* @param {array|object} value the location/object
*/
GLGE.Placeable.prototype.getLookat=function(){
	return this.lookAt;
}
/**
* sets the look at for this object, will be updated every frame
* @param {array|object} value the location/objec to look at
*/
GLGE.Placeable.prototype.setLookat=function(value){
	this.lookAt=value;
	return this;
}
/**
* Points the object in the direction of the coords or placeable value
* @param {array|object} value the location/objec to look at
*/
GLGE.Placeable.prototype.Lookat=function(value){
	var objpos;
	var pos=this.getPosition();
	if(value.getPosition){
		objpos=value.getPosition();
	}else{
		objpos={x:value[0],y:value[1],z:value[2]};
	}
	
	var coord=[pos.x-objpos.x,pos.y-objpos.y,pos.z-objpos.z];
	var zvec=GLGE.toUnitVec3(coord);
	var xvec=GLGE.toUnitVec3(GLGE.crossVec3([0,0,1],zvec));
	var yvec=GLGE.toUnitVec3(GLGE.crossVec3(zvec,xvec));		
	this.setRotMatrix(GLGE.Mat4([xvec[0], yvec[0], zvec[0], 0,
					xvec[1], yvec[1], zvec[1], 0,
					xvec[2], yvec[2], zvec[2], 0,
					0, 0, 0, 1]));
}
/**
* Gets the euler rotation order
* @returns {number} the objects rotation matrix
*/
GLGE.Placeable.prototype.getRotOrder=function(){
	return this.rotOrder;
}
/**
* Sets the euler rotation order
* @param {number} value the order to rotate GLGE.ROT_XYZ,GLGE.ROT_XZY,etc..
*/
GLGE.Placeable.prototype.setRotOrder=function(value){
	this.rotOrder=value;
	this.matrix=null;
	this.rotmatrix=null;
	return this;
}
/**
* Gets the rotaion matrix 
* @returns {matrix} the objects rotation matrix
*/
GLGE.Placeable.prototype.getRotMatrix=function(){
	if(!this.rotmatrix){
		var rotation=this.getRotation();
		if(this.mode==GLGE.P_EULER) this.rotmatrix=GLGE.rotateMatrix(rotation.x,rotation.y,rotation.z,this.rotOrder);
		if(this.mode==GLGE.P_QUAT)	this.rotmatrix=GLGE.quatRotation(rotation.x,rotation.y,rotation.z,rotation.w);
	}
	return this.rotmatrix;
}
/**
* Sets the rotation matrix 
* @param {matrix} the objects rotation matrix
*/
GLGE.Placeable.prototype.setRotMatrix=function(matrix){
	this.mode=GLGE.P_MATRIX;
	this.rotmatrix=matrix;
	this.updateMatrix();
	return this;
}
/**
* Sets the x location of the object
* @param {number} value The value to assign to the x position
*/
GLGE.Placeable.prototype.setLocX=function(value){this.locX=value; this.updateMatrix();return this;}
/**
* Sets the y location of the object
* @param {number} value The value to assign to the y position
*/
GLGE.Placeable.prototype.setLocY=function(value){this.locY=value;this.updateMatrix();return this;}
/**
* Sets the z location of the object
* @param {number} value The value to assign to the z position
*/
GLGE.Placeable.prototype.setLocZ=function(value){this.locZ=value;this.updateMatrix();return this;}
/**
* Sets the location of the object
* @param {number} x The value to assign to the x position
* @param {number} y The value to assign to the y position
* @param {number} z The value to assign to the z position
*/
GLGE.Placeable.prototype.setLoc=function(x,y,z){this.locX=x;this.locY=y;this.locZ=z;this.updateMatrix();return this;}
/**
* Sets the x location displacement of the object, usefull for animation
* @param {number} value The value to assign to the x displacement
*/
GLGE.Placeable.prototype.setDLocX=function(value){this.dLocX=value;this.updateMatrix();return this;}
/**
* Sets the y location displacement of the object, usefull for animation
* @param {number} value The value to assign to the y displacement
*/
GLGE.Placeable.prototype.setDLocY=function(value){this.dLocY=value;this.updateMatrix();return this;}
/**
* Sets the z location displacement of the object, usefull for animation
* @param {number} value The value to assign to the z displacement
*/
GLGE.Placeable.prototype.setDLocZ=function(value){this.dLocZ=value;this.updateMatrix();return this;}
/**
* Sets the location displacement of the object, useful for animation
* @param {number} x The value to assign to the x position
* @param {number} y The value to assign to the y position
* @param {number} z The value to assign to the z position
*/
GLGE.Placeable.prototype.setDLoc=function(x,y,z){this.dLocX=x;this.dLocY=y;this.dLocZ=z;this.updateMatrix();return this;}
/**
* Sets the x quat value
* @param {number} value the x quat value
*/
GLGE.Placeable.prototype.setQuatX=function(value){this.mode=GLGE.P_QUAT;this.quatX=parseFloat(value);this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the y quat value
* @param {number} value the y quat value
*/
GLGE.Placeable.prototype.setQuatY=function(value){this.mode=GLGE.P_QUAT;this.quatY=parseFloat(value);this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the z quat value
* @param {number} value the z quat value
*/
GLGE.Placeable.prototype.setQuatZ=function(value){this.mode=GLGE.P_QUAT;this.quatZ=parseFloat(value);this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the w quat value
* @param {number} value the w quat value
*/
GLGE.Placeable.prototype.setQuatW=function(value){this.mode=GLGE.P_QUAT;this.quatW=parseFloat(value);this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the quaternions
* @param {number} x The value to assign to the x 
* @param {number} y The value to assign to the y 
* @param {number} z The value to assign to the z 
* @param {number} w The value to assign to the w
*/
GLGE.Placeable.prototype.setQuat=function(x,y,z,w){this.mode=GLGE.P_QUAT;this.quatX=x;this.quatY=y;this.quatZ=z;this.quatW=w;this.updateMatrix();this.rotmatrix=null;return this;}

/**
* Sets the x rotation of the object
* @param {number} value The value to assign to the x rotation
*/
GLGE.Placeable.prototype.setRotX=function(value){this.mode=GLGE.P_EULER;this.rotX=value;this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the y rotation of the object
* @param {number} value The value to assign to the y rotation
*/
GLGE.Placeable.prototype.setRotY=function(value){this.mode=GLGE.P_EULER;this.rotY=value;this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the z rotation of the object
* @param {number} value The value to assign to the z rotation
*/
GLGE.Placeable.prototype.setRotZ=function(value){this.mode=GLGE.P_EULER;this.rotZ=value;this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the rotation of the object
* @param {number} x The value to assign to the x rotation
* @param {number} y The value to assign to the y rotation
* @param {number} z The value to assign to the z rotation
*/
GLGE.Placeable.prototype.setRot=function(x,y,z){this.mode=GLGE.P_EULER;this.rotX=x;this.rotY=y;this.rotZ=z;this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the x rotation displacement of the object, usefull for animation
* @param {number} value The value to assign to the x displacement
*/
GLGE.Placeable.prototype.setDRotX=function(value){this.mode=GLGE.P_EULER;this.dRotX=value;this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the y rotation displacement of the object, usefull for animation
* @param {number} value The value to assign to the y displacement
*/
GLGE.Placeable.prototype.setDRotY=function(value){this.mode=GLGE.P_EULER;this.dRotY=value;this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the z rotation displacement of the object, usefull for animation
* @param {number} value The value to assign to the z displacement
*/
GLGE.Placeable.prototype.setDRotZ=function(value){this.mode=GLGE.P_EULER;this.dRotZ=value;this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the rotation displacement of the object, useful for animation
* @param {number} x The value to assign to the x rotation
* @param {number} y The value to assign to the y rotation
* @param {number} z The value to assign to the z rotation
*/
GLGE.Placeable.prototype.setDRot=function(x,y,z){this.mode=GLGE.P_EULER;this.dRotX=x;this.dRotY=y;this.dRotZ=z;this.updateMatrix();this.rotmatrix=null;return this;}
/**
* Sets the x scale of the object
* @param {number} value The value to assign to the x scale
*/
GLGE.Placeable.prototype.setScaleX=function(value){this.scaleX=value;this.updateMatrix();return this;}
/**
* Sets the y scale of the object
* @param {number} value The value to assign to the y scale
*/
GLGE.Placeable.prototype.setScaleY=function(value){this.scaleY=value;this.updateMatrix();return this;}
/**
* Sets the z scale of the object
* @param {number} value The value to assign to the z scale
*/
GLGE.Placeable.prototype.setScaleZ=function(value){this.scaleZ=value;this.updateMatrix();return this;}
/**
* Sets the scale of the object
* @param {number} x The value to assign to the x scale
* @param {number} y The value to assign to the y scale
* @param {number} z The value to assign to the z scale
*/
GLGE.Placeable.prototype.setScale=function(x,y,z){if(!y){y=x;z=x}; this.scaleX=x;this.scaleY=y;this.scaleZ=z;this.updateMatrix();return this;}
/**
* Sets the x scale displacement of the object, usefull for animation
* @param {number} value The value to assign to the x displacement
*/
GLGE.Placeable.prototype.setDScaleX=function(value){this.dScaleX=value;this.updateMatrix();return this;}
/**
* Sets the y scale displacement of the object, usefull for animation
* @param {number} value The value to assign to the y displacement
*/
GLGE.Placeable.prototype.setDScaleY=function(value){this.dScaleY=value;this.updateMatrix();return this;}
/**
* Sets the z scale displacement of the object, usefull for animation
* @param {number} value The value to assign to the z displacement
*/
GLGE.Placeable.prototype.setDScaleZ=function(value){this.dScaleZ=value;this.updateMatrix();return this;}
/**
* Sets the scale displacement of the object, useful for animation
* @param {number} x The value to assign to the x scale
* @param {number} y The value to assign to the y scale
* @param {number} z The value to assign to the z scale
*/
GLGE.Placeable.prototype.setDScale=function(x,y,z){this.dScaleX=x;this.dScaleY=y;this.dScaleZ=z;this.updateMatrix();return this;}
/**
* Gets the x location of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getLocX=function(){return this.locX;}
/**
* Gets the y location of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getLocY=function(){return this.locY;}
/**
* Gets the z location of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getLocZ=function(){return this.locZ;}
/**
* Gets the x location displacement of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getDLocX=function(){return this.dLocX;}
/**
* Gets the y location displacement of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getDLocY=function(){return this.dLocY;}
/**
* Gets the z location displacement of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getDLocZ=function(){return this.dLocZ;}
/**
* Gets the x quat of the rotation
* @returns {number}
*/
GLGE.Placeable.prototype.getQuatX=function(){return this.quatX;}
/**
* Gets the y quat of the rotation
* @returns {number}
*/
GLGE.Placeable.prototype.getQuatY=function(){return this.quatY;}
/**
* Gets the z quat of the rotation
* @returns {number}
*/
GLGE.Placeable.prototype.getQuatZ=function(){return this.quatZ;}
/**
* Gets the w quat of the rotation
* @returns {number}
*/
GLGE.Placeable.prototype.getQuatW=function(){return this.quatW;}
/**
* Gets the x rotation of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getRotX=function(){return this.rotX;}
/**
* Gets the y rotation of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getRotY=function(){return this.rotY;}
/**
* Gets the z rotation of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getRotZ=function(){return this.rotZ;}
/**
* Gets the x rotaional displacement of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getDRotX=function(){return this.dRotX;}
/**
* Gets the y rotaional displacement of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getDRotY=function(){return this.dRotY;}
/**
* Gets the z rotaional displacement of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getDRotZ=function(){return this.dRotZ;}
/**
* Gets the x scale of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getScaleX=function(){return this.scaleX;}
/**
* Gets the y scale of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getScaleY=function(){return this.scaleY;}
/**
* Gets the z scale of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getScaleZ=function(){return this.scaleZ;}
/**
* Gets the x scale displacement of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getDScaleX=function(){return this.dScaleX;}
/**
* Gets the y scale displacement of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getDScaleY=function(){return this.dScaleY;}
/**
* Gets the z scale displacement of the object
* @returns {number}
*/
GLGE.Placeable.prototype.getDScaleZ=function(){return this.dScaleZ;}
/**
* Gets the position of the object
* @returns {array}
*/
GLGE.Placeable.prototype.getPosition=function(){
	var position={};
	position.x=parseFloat(this.locX)+parseFloat(this.dLocX);
	position.y=parseFloat(this.locY)+parseFloat(this.dLocY);
	position.z=parseFloat(this.locZ)+parseFloat(this.dLocZ);
	return position;
}
/**
* Gets the rotation of the object
* @returns {object}
*/
GLGE.Placeable.prototype.getRotation=function(){
	var rotation={};
	if(this.mode==GLGE.P_EULER){
		rotation.x=parseFloat(this.rotX)+parseFloat(this.dRotX);
		rotation.y=parseFloat(this.rotY)+parseFloat(this.dRotY);
		rotation.z=parseFloat(this.rotZ)+parseFloat(this.dRotZ);
	}
	if(this.mode==GLGE.P_QUAT){
		rotation.x=parseFloat(this.quatX);
		rotation.y=parseFloat(this.quatY);
		rotation.z=parseFloat(this.quatZ);
		rotation.w=parseFloat(this.quatW);
	}
	return rotation;
}
/**
* Gets the scale of the object
* @returns {object}
*/
GLGE.Placeable.prototype.getScale=function(){
	var scale={};
	scale.x=parseFloat(this.scaleX)+parseFloat(this.dScaleX);
	scale.y=parseFloat(this.scaleY)+parseFloat(this.dScaleY);
	scale.z=parseFloat(this.scaleZ)+parseFloat(this.dScaleZ);
	return scale;
}
/**
* Updates the model matrix
* @private
*/
GLGE.Placeable.prototype.updateMatrix=function(){
	this.matrix=null;
	if(this.children){
		for(var i=0;i<this.children.length;i++){
			this.children[i].updateMatrix();
		}
	}
}
/**
* Gets the model matrix to transform the model within the world
*/
GLGE.Placeable.prototype.getModelMatrix=function(){
	if(!this.matrix){
		this.invmatrix=null;
		this.transmatrix=null;
		this.transinvmatrix=null;
		var position=this.getPosition();
		var scale=this.getScale();
		var matrix=GLGE.mulMat4(GLGE.translateMatrix(position.x,position.y,position.z),GLGE.mulMat4(this.getRotMatrix(),GLGE.scaleMatrix(scale.x,scale.y,scale.z)));
		if(this.parent) matrix=GLGE.mulMat4(this.parent.getModelMatrix(),matrix);
		this.matrix=matrix;
	}
	return this.matrix;
}
/**
* Gets the model inverse matrix to transform the model within the world
*/
GLGE.Placeable.prototype.getInverseModelMatrix=function(){
	if(!this.matrix){
		this.getModelMatrix();
	}
	if(!this.invmatrix){
		this.invmatrix=GLGE.transposeMat4(this.matrix);
	}
	return this.invmatrix;
}
/**
* Gets the model transposed matrix to transform the model within the world
*/
GLGE.Placeable.prototype.getTransposeModelMatrix=function(){
	if(!this.matrix){
		this.getModelMatrix();
	}
	if(!this.transmatrix){
		this.transmatrix=GLGE.transposeMat4(this.matrix);
	}
	return this.transmatrix;
}
/**
* Gets the model inverse transposed matrix to transform the model within the world
*/
GLGE.Placeable.prototype.getTransposeInverseModelMatrix=function(){
	if(!this.matrix){
		this.getModelMatrix();
	}
	if(!this.transinvmatrix){
		this.invtransmatrix=GLGE.transposeMat4(this.getInverseModelMatrix());
	}
	return this.transinvmatrix;
}

/**
* @class Animation class to agument animatiable objects 
* @augments GLGE.Events
*/
GLGE.Animatable=function(){
}
/**
 * @name GLGE.Animatable#animFinished
 * @event
 * @param {object} data
 */
GLGE.augment(GLGE.Events,GLGE.Animatable);

GLGE.Animatable.prototype.animationStart=null;
GLGE.Animatable.prototype.animation=null;
GLGE.Animatable.prototype.blendStart=0;
GLGE.Animatable.prototype.blendTime=0;
GLGE.Animatable.prototype.lastFrame=null;
GLGE.Animatable.prototype.frameRate=25;
GLGE.Animatable.prototype.loop=GLGE.TRUE;
GLGE.Animatable.prototype.paused=GLGE.FALSE;
GLGE.Animatable.prototype.pausedTime=null;
GLGE.Animatable.prototype.blendFunction=GLGE.LINEAR_BLEND;

/**
* Creates and sets an animation to blend to the properties. Useful for blending to a specific location for example:
* blendto({LocX:10,LocY:5,LocZ:10},2000);
* @param {object} properties The properties to blend
* @param {number} duration the duration of the blend
* @param {function} blendFunction[optional] the function used for blending defaults to GLGE.LINEAR_BLEND
*/
GLGE.Animatable.prototype.blendTo=function(properties,duration,blendFunction){
	if(!blendFunction) blendFunction=GLGE.LINEAR_BLEND;
	var animation=new GLGE.AnimationVector();
	var curve;
	var point;
	for(prop in properties){
		curve=new GLGE.AnimationCurve();
		curve.setChannel(prop);
		point=new GLGE.LinearPoint();
		point.setX(1);
		point.setY(properties[prop]);
		curve.addPoint(point);
		animation.addAnimationCurve(curve);
	}
	this.setBlendFunction(blendFunction);
	this.setAnimation(animation,duration);
	return this;
}
/**
* Sets the animation blending function
* @param {function} value The blending function
*/
GLGE.Animatable.prototype.setBlendFunction=function(value){
	this.blendFunction=value;
	return this;
}
/**
* Gets the animation blending function
* @returns {function} the blending function
*/
GLGE.Animatable.prototype.getBlendFunction=function(){
	return this.blendFunction;
}

/**
* Sets the name of this object used for skinning
* @param {String} value The name to set
*/
GLGE.Animatable.prototype.setName=function(value){
	this.name=value;
	return this;
}
/**
* Gets the name of this object used for skinning
* @returns {String} the name
*/
GLGE.Animatable.prototype.getName=function(){
	return this.name;
}
/**
* gets the frame at the specified time
* @param {number} now the current time
*/
 GLGE.Animatable.prototype.getFrameNumber=function(now){
	var frame;
	if(!now) now=parseInt(new Date().getTime());
	if(this.animation.frames>1){
		if(this.loop){
			frame=((parseFloat(now)-parseFloat(this.animationStart))/1000*this.frameRate)%(this.animation.frames-1)+1; 
		}else{
			frame=((parseFloat(now)-parseFloat(this.animationStart))/1000*this.frameRate)+1; 
			if(frame>=this.animation.frames){
				frame=this.animation.frames;
			}
		}
	}else{
		frame=1;
	}

	return Math.round(frame);
}
 
 /**
* gets the initial values for the animation vector for blending
* @param {GLGE.AnimationVector} animation The animation
* @private
*/
 GLGE.Animatable.prototype.getInitialValues=function(animation,time){
	var initValues={};
	
	if(this.animation){
		this.lastFrame=null;
		this.animate(time,true);
	}
	
	for(var property in animation.curves){
		if(this["get"+property]){
			initValues[property]=this["get"+property]();
		}
	}
	
	return initValues;
}
 
/**
* update animated properties on this object
*/
GLGE.Animatable.prototype.animate=function(now,nocache){
	if(!this.paused && this.animation){
		if(!now) now=parseInt(new Date().getTime());
		var frame=this.getFrameNumber(now);
		
		if(!this.animation.animationCache) this.animation.animationCache={};
		if(frame!=this.lastFrame || this.blendTime!=0){
			this.lastFrame=frame;
			if(this.blendTime==0){
				if(!this.animation.animationCache[frame] || nocache){
					this.animation.animationCache[frame]=[];
					for(property in this.animation.curves){
						if(this["set"+property]){
							var value=this.animation.curves[property].getValue(parseFloat(frame));
							switch(property){
								case "QuatX":
								case "QuatY":
								case "QuatZ":
								case "QuatW":
								case "RotX":
								case "RotY":
								case "RotZ":
										var rot=true;
									break;
								default:
									this.animation.animationCache[frame].push({property:property,value:value});
									break;
							}
							this["set"+property](value);
						}	
					}
					if(rot){
						value=this.getRotMatrix();
						this.animation.animationCache[frame].push({property:"RotMatrix",value:value});
					}
				}else{
					var cache=this.animation.animationCache[frame];
					for(var i=0;i<cache.length;i++){
						if(this["set"+cache[i].property]) this["set"+cache[i].property](cache[i].value);
					}
				}
			}else{
				var time=now-this.animationStart;
				if(time<this.blendTime){
					var blendfactor=time/this.blendTime;
					blendfactor=this.blendFunction(blendfactor);
					for(property in this.animation.curves){
						if(this["set"+property]){
							var value=this.animation.curves[property].getValue(parseFloat(frame));
							value=value*blendfactor+this.blendInitValues[property]*(1-blendfactor);
							this["set"+property](value);
						}	
					}
				}else{
					this.blendTime=0;
				}
			}
		}
	}
	if(this.children){
		for(var i=0; i<this.children.length;i++){
			if(this.children[i].animate){
				this.children[i].animate(now,nocache);
			}
		}
	}
	if(this.animation && !this.animFinished && this.blendTime==0 && this.animation.frames==frame && !nocache){
		this.animFinished=true;
		this.fireEvent("animFinished",{});
	}
}
/**
* Sets the animation vector of this object
* @param {GLGE.AnimationVector} animationVector the animation to apply to this object
* @param {number} blendDuration [Optional] the time in milliseconds to blend into this animation
* @param {number} starttime [Optional] the starting time of the animation
*/
GLGE.Animatable.prototype.setAnimation=function(animationVector,blendDuration,starttime){
	if(starttime==null) starttime=parseInt(new Date().getTime());
	if(!blendDuration) blendDuration=0;
	if(blendDuration>0){
		this.blendInitValues=this.getInitialValues(animationVector,starttime);
		this.blendTime=blendDuration;
	}
	this.animationStart=starttime;
	this.lastFrame=null;
	this.animation=animationVector;
	this.animFinished=false;
	return this;
}
/**
* Gets the animation vector of this object
* @returns {AnimationVector}
*/
GLGE.Animatable.prototype.getAnimation=function(){
	return this.animation;
}
/**
* Sets the frame rate of the animation
* @param  {number} value the frame rate to set
*/
GLGE.Animatable.prototype.setFrameRate=function(value){
	this.frameRate=value;
	return this;
}
/**
* Gets the frame rate of the animation
* @return {number} the current frame rate
*/
GLGE.Animatable.prototype.getFrameRate=function(){
	return this.frameRate;
}
/**
* Sets the loop flag to GLGE.TRUE or GLGE.FALSE
* @param  {boolean} value 
*/
GLGE.Animatable.prototype.setLoop=function(value){
	this.loop=value;
	return this;
}
/**
* Gets the loop flag
* @return {boolean}
*/
GLGE.Animatable.prototype.getLoop=function(){
	return this.loop;
}
/**
* @function is looping? @see GLGE.Animatable#getLoop
*/
GLGE.Animatable.prototype.isLooping=GLGE.Animatable.prototype.getLoop;
 
/**
* Sets the paused flag to GLGE.TRUE or GLGE.FALSE
* @param  {boolean} value 
*/
GLGE.Animatable.prototype.setPaused=function(value){
	if(value) this.pauseTime=parseInt(new Date().getTime());
		else this.animationStart=this.animationStart+(parseInt(new Date().getTime())-this.pauseTime);
	this.paused=value;
	return this;
}
/**
* Gets the paused flag
* @return {boolean}
*/
GLGE.Animatable.prototype.getPaused=function(){
	return this.paused;
}
/**
* Toggles the paused flag
* @return {boolean} returns the resulting flag state
*/
GLGE.Animatable.prototype.togglePaused=function(){
	this.setPaused(!this.getPaused());
	return this.paused;
}
closure_export();


/**
* @class A bezier class to add points to the Animation Curve 
* @param {string} uid a unique string to identify this object
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.BezTriple=function(uid){
	GLGE.Assets.registerAsset(this,uid);
};
GLGE.augment(GLGE.QuickNotation,GLGE.BezTriple);
GLGE.augment(GLGE.JSONLoader,GLGE.BezTriple);

GLGE.BezTriple.prototype.className="BezTriple";
/**
* set the x1-coord
* @param {number} x x1-coord control point
*/
GLGE.BezTriple.prototype.setX1=function(x){
	this.x1=parseFloat(x);
	return this;
};
/**
* set the y1-coord
* @param {number} y y1-coord control point
*/
GLGE.BezTriple.prototype.setY1=function(y){
	this.y1=parseFloat(y);
	return this;
};
/**
* set the x2-coord
* @param {number} x x2-coord control point
*/
GLGE.BezTriple.prototype.setX2=function(x){
	this.x=parseFloat(x);
	return this;
};
/**
* set the y2-coord
* @param {number} y y2-coord control point
*/
GLGE.BezTriple.prototype.setY2=function(y){
	this.y=parseFloat(y);
	return this;
};
/**
* set the x3-coord
* @param {number} x x3-coord control point
*/
GLGE.BezTriple.prototype.setX3=function(x){
	this.x3=parseFloat(x);
	return this;
};
/**
* set the y3-coord
* @param {number} y y3-coord control point
*/
GLGE.BezTriple.prototype.setY3=function(y){
	this.y3=parseFloat(y);
	return this;
};


/**
* @class A LinearPoint class to add points to the Animation Curve 
* @param {string} uid unique string for this class
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.LinearPoint=function(uid){
	GLGE.Assets.registerAsset(this,uid);
};
GLGE.augment(GLGE.QuickNotation,GLGE.LinearPoint);
GLGE.augment(GLGE.JSONLoader,GLGE.LinearPoint);
GLGE.LinearPoint.prototype.className="LinearPoint";
/**
* set the x-coord
* @param {number} x x-coord control point
*/
GLGE.LinearPoint.prototype.setX=function(x){
	this.x=parseFloat(x);
	return this;
};
/**
* set the y-coord
* @param {number} y y-coord control point
*/
GLGE.LinearPoint.prototype.setY=function(y){
	this.y=parseFloat(y);
	return this;
};


/**
* @class A StepPoint class to add points to the Animation Curve 
* @param {number} x x-coord control point
* @param {object} value value of control point
*/
GLGE.StepPoint=function(x,value){
	this.x=parseFloat(x);
	this.y=value;
};

/**
* @class A curve which interpolates between control points
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.AnimationCurve=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.keyFrames=[];
	this.solutions={};
};
GLGE.augment(GLGE.QuickNotation,GLGE.AnimationCurve);
GLGE.augment(GLGE.JSONLoader,GLGE.AnimationCurve);
GLGE.AnimationCurve.prototype.className="AnimationCurve";
GLGE.AnimationCurve.prototype.keyFrames=null;
/**
* Adds a point to the curve
* @param {object} point The point to add
* @returns {Number} Index of the newly added point
*/
GLGE.AnimationCurve.prototype.addPoint=function(point){
	this.keyFrames.push(point);
	return this.keyFrames.length-1;
};
GLGE.AnimationCurve.prototype.addStepPoint=GLGE.AnimationCurve.prototype.addPoint;
GLGE.AnimationCurve.prototype.addLinearPoint=GLGE.AnimationCurve.prototype.addPoint;
GLGE.AnimationCurve.prototype.addBezTriple=GLGE.AnimationCurve.prototype.addPoint;
/**
* Get the value of the curve at any point
* @param {Number} frame The frame(x-coord) to return the value for
* @returns {Number} The value of the curve at the given point
*/
GLGE.AnimationCurve.prototype.coord=function(x,y){
	return {x:x,y:y}
}
/**
* Sets the animation channel this curve animates
* @param {string} channel The property to animate
*/
GLGE.AnimationCurve.prototype.setChannel=function(channel){
	this.channel=channel
}
GLGE.AnimationCurve.prototype.getValue=function(frame){
	var startKey;
	var endKey;
	var preStartKey;
	var preEndKey;
	if(frame<this.keyFrames[0].x) return this.keyFrames[0].y;
	for(var i=0; i<this.keyFrames.length;i++){
		if(this.keyFrames[i].x==frame){
			return this.keyFrames[i].y;
		}
		if(this.keyFrames[i].x<=frame && (startKey==undefined || this.keyFrames[i].x>this.keyFrames[startKey].x)){
			preStartKey=startKey;
			startKey=i;
		}else if(this.keyFrames[i].x<=frame && (preStartKey==undefined || this.keyFrames[i].x>this.keyFrames[preStartKey].x)){
			preStartKey=i;
		}
		if(this.keyFrames[i].x>frame && (endKey==undefined || this.keyFrames[i].x<=this.keyFrames[endKey].x)){
			preEndKey=endKey;
			endKey=i;
		}else if(this.keyFrames[i].x>frame && (preEndKey==undefined || this.keyFrames[i].x<=this.keyFrames[preEndKey].x)){
			preEndKey=i;
		}
	}
	if(startKey==undefined){
		startKey=endKey;
		endKey=preEndKey;
	}
	if(endKey==undefined){
		endKey=startKey;
		startKey=preStartKey;
	}
	if(this.keyFrames[startKey] instanceof GLGE.BezTriple && this.keyFrames[endKey] instanceof GLGE.BezTriple){
		var C1=this.coord(this.keyFrames[startKey].x,this.keyFrames[startKey].y);
		var C2=this.coord(this.keyFrames[startKey].x3,this.keyFrames[startKey].y3);
		var C3=this.coord(this.keyFrames[endKey].x1,this.keyFrames[endKey].y1);
		var C4=this.coord(this.keyFrames[endKey].x,this.keyFrames[endKey].y);
		return this.atX(frame,C1,C2,C3,C4).y;
	}
	if(this.keyFrames[startKey] instanceof GLGE.LinearPoint && this.keyFrames[endKey] instanceof GLGE.BezTriple){
		var C1=this.coord(this.keyFrames[startKey].x,this.keyFrames[startKey].y);
		var C2=this.coord(this.keyFrames[endKey].x1,this.keyFrames[endKey].y1);
		var C3=this.coord(this.keyFrames[endKey].x1,this.keyFrames[endKey].y1);
		var C4=this.coord(this.keyFrames[endKey].x,this.keyFrames[endKey].y);
		return this.atX(frame,C1,C2,C3,C4).y;
	}
	if(this.keyFrames[startKey] instanceof GLGE.BezTriple && this.keyFrames[endKey] instanceof GLGE.LinearPoint){
		var C1=this.coord(this.keyFrames[startKey].x,this.keyFrames[startKey].y);
		var C2=this.coord(this.keyFrames[startKey].x3,this.keyFrames[startKey].y3);
		var C3=this.coord(this.keyFrames[startKey].x3,this.keyFrames[startKey].y3);
		var C4=this.coord(this.keyFrames[endKey].x,this.keyFrames[endKey].y);
		return this.atX(frame,C1,C2,C3,C4).y;
	}
	if(this.keyFrames[startKey] instanceof GLGE.LinearPoint && this.keyFrames[endKey] instanceof GLGE.LinearPoint){
		var value=(frame-this.keyFrames[startKey].x)*(this.keyFrames[endKey].y-this.keyFrames[startKey].y)/(this.keyFrames[endKey].x-this.keyFrames[startKey].x)+this.keyFrames[startKey].y;
		return value;
	}
	if(this.keyFrames[startKey] instanceof GLGE.StepPoint){
		return this.keyFrames[startKey].y
	}
	if(!this.keyFrames.preStartKey) this.keyFrames.preStartKey=this.keyFrames[0].y;
	return this.keyFrames.preStartKey;
};
/**
* Function used to calculate bezier curve
* @private
*/
GLGE.AnimationCurve.prototype.B1=function(t) { return t*t*t };
/**
* Function used to calculate bezier curve
* @private
*/
GLGE.AnimationCurve.prototype.B2=function(t) { return 3*t*t*(1-t) };
/**
* Function used to calculate bezier curve
* @private
*/
GLGE.AnimationCurve.prototype.B3=function(t) { return 3*t*(1-t)*(1-t) };
/**
* Function used to calculate bezier curve
* @private
*/
GLGE.AnimationCurve.prototype.B4=function(t) { return (1-t)*(1-t)*(1-t) };
/**
* Gets the value of a bezier curve at a given point
* @private
*/
GLGE.AnimationCurve.prototype.getBezier=function(t,C1,C2,C3,C4) {
	var pos = {};
	pos.x = C1.x*this.B1(t) + C2.x*this.B2(t) + C3.x*this.B3(t) + C4.x*this.B4(t);
	pos.y = C1.y*this.B1(t) + C2.y*this.B2(t) + C3.y*this.B3(t) + C4.y*this.B4(t);
	return pos;
};
/**
* Solves cubic equation to get the parametic value of the curve at a specified point
* @private
*/
GLGE.AnimationCurve.prototype.Quad3Solve=function(a,b,c,d){
	ref=a+"-"+b+"-"+"-"+c+"-"+d;
	if(this.solutions[ref]){
		return this.solutions[ref];
	}
	else
	{
		b /= a;c /= a;d /= a;
		var q, r, d1, s, t, t1, r13;
		q = (3.0*c - (b*b))/9.0;
		r = -(27.0*d) + b*(9.0*c - 2.0*(b*b));
		r /= 54.0;
		t1 = (b/3.0);
		discrim = q*q*q + r*r;
		result=[];
				
		if (discrim > 0) { 
		// one real, two complex
		 s = r + Math.sqrt(discrim);
		 s = ((s < 0) ? -Math.pow(-s, (1.0/3.0)) : Math.pow(s, (1.0/3.0)));
		 t = r - Math.sqrt(discrim);
		 t = ((t < 0) ? -Math.pow(-t, (1.0/3.0)) : Math.pow(t, (1.0/3.0)));
		 result[0] = -t1 + s + t;
		 t1 = t1 + (s + t)/2.0;
		 result[1] = result[2] = -t1;
		 t1 = Math.sqrt(3.0)*(-t + s)/2;
		} 
		else if (discrim == 0){ 
		// All roots real
		 r13 = ((r < 0) ? -Math.pow(-r,(1.0/3.0)) : Math.pow(r,(1.0/3.0)));
		 result[1] = -t1 + 2.0*r13;
		 result[1] = result[2]  = -(r13 + t1);
		} 
		else
		{
			q = -q;
			d1 = q*q*q;
			d1 = Math.acos(r/Math.sqrt(1));
			r13 = 2.0*Math.sqrt(q);


			result[0] = -t1 + r13*Math.cos(d1/3.0);
			result[1] = -t1 + r13*Math.cos((d1 + 2.0*Math.PI)/3.0);
			result[2] = -t1 + r13*Math.cos((d1 + 4.0*Math.PI)/3.0);
		}
		var toreturn=false;
		//determine which is the correct result
		if(result[0]>=0 && result[0]<=1) toreturn=result[0];
		if(!toreturn && result[1]>=0 && result[1]<=1) toreturn=result[1];
		if(!toreturn && result[2]>=0 && result[2]<=1) toreturn=result[2];
		//cache result for next time
		this.solutions[ref]=toreturn;
		
		return toreturn;
	}
};
/**
* Get the value of the a single bezier curve 
* @param {Number} x xcoord of point to get
* @param {Number} C1 First bezier control point
* @param {Number} C2 Second bezier control point
* @param {Number} C3 Third bezier control point
* @param {Number} C4 Forth bezier control point
* @returns {Number} The value of the curve at the given x
*/
GLGE.AnimationCurve.prototype.atX=function(x,C1,C2,C3,C4){
	a=C1.x-C2.x*3+C3.x*3-C4.x;
	b=C2.x*3-C3.x*6+C4.x*3;
	c=C3.x*3-C4.x*3;
	d=C4.x-x;
	return this.getBezier(this.Quad3Solve(a,b,c,d),C1,C2,C3,C4);
};

/**
* @class The AnimationVectors class allows you to specify the 2D Animation curves that define specific channels of animation within the engine. 
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.AnimationVector=function(uid){
    GLGE.Assets.registerAsset(this,uid);
    this.curves=[];
}
GLGE.augment(GLGE.QuickNotation,GLGE.AnimationVector);
GLGE.augment(GLGE.JSONLoader,GLGE.AnimationVector);
GLGE.AnimationVector.prototype.curves=[];
GLGE.AnimationVector.prototype.frames=250;

/**
* Adds an Animation Curve to a channel 
* @param {String} channel The name of the curve to be added
* @param {GLGE.AnimationCurve} curve The animation curve to add
*/
GLGE.AnimationVector.prototype.addAnimationCurve=function(curve){
	this.curves[curve.channel]=curve;
	return this;
}
/**
* Removes an Animation Curve form a channel
* @param {String} channel The name of the curve to be removed
*/
GLGE.AnimationVector.prototype.removeAnimationCurve=function(name){
	delete(this.curves[name]);
}
/**
* Sets the number of frames in the animation
* @param {number} value The number of frames in the animation
*/
GLGE.AnimationVector.prototype.setFrames=function(value){
	this.frames=value;
	return this;
}
/**
* Sets the number of frames in the animation
* @returns {number} The number of frames in the animation
*/
GLGE.AnimationVector.prototype.getFrames=function(){
	return this.frames;
}


/**
* @constant 
* @description Enumeration for node group type
*/
GLGE.G_NODE=1;
/**
* @constant 
* @description Enumeration for root group type
*/
GLGE.G_ROOT=2;
/**
* @class Group class to allow object transform hierarchies 
* @augments GLGE.Animatable
* @augments GLGE.Placeable
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.Group=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.children=[];
}
GLGE.augment(GLGE.Placeable,GLGE.Group);
GLGE.augment(GLGE.Animatable,GLGE.Group);
GLGE.augment(GLGE.QuickNotation,GLGE.Group);
GLGE.augment(GLGE.JSONLoader,GLGE.Group);
GLGE.Group.prototype.children=null;
GLGE.Group.prototype.className="Group";
GLGE.Group.prototype.type=GLGE.G_NODE;
/**
* Sets the action for this Group
* @param {GLGE.Action} action the action to apply
*/
GLGE.Group.prototype.setAction=function(action,blendTime,loop){
	action.start(blendTime,loop,this.getNames());
	return this;
}
/**
* Gets the name of the object and names of any sub objects
* @returns an object of name
*/
GLGE.Group.prototype.getNames=function(names){
	if(!names) names={};
	var thisname=this.getName();
	if(thisname!="") names[thisname]=this;
	for(var i=0;i<this.children.length;i++){
		if(this.children[i].getNames){
			this.children[i].getNames(names);
		}
	}
	return names;
}
/**
* Gets the bounding volume for this group
* @returns {GLGE.BoundingVolume} 
*/
GLGE.Group.prototype.getBoundingVolume=function(){
	this.boundingVolume=new GLGE.BoundingVolume(0,0,0,0,0,0);
	for(var i=0; i<this.children.length;i++){
		if(this.children[i].getBoundingVolume){
			this.boundingVolume.addBoundingVolume(this.children[i].getBoundingVolume());
		}else if(this.children[i].getLocX){
			//if now bounding rec for this child but has a position then assume a point such as a light
			var x=parseFloat(this.children[i].getLocX());
			var y=parseFloat(this.children[i].getLocY());
			var z=parseFloat(this.children[i].getLocZ());
			this.boundingVolume.addBoundingVolume(new GLGE.BoundingVolume(x,x,y,y,z,z));
		}
	}
	this.boundingVolume.applyMatrixScale(this.getModelMatrix());
	
	return this.boundingVolume;
}
/**
* Gets a list of all objects in this group
* @param {array} pointer to an array [optional]
* @returns {GLGE.Object[]} an array of GLGE.Objects
*/
GLGE.Group.prototype.getObjects=function(objects){
	if(!objects) objects=[];
	for(var i=0; i<this.children.length;i++){
		if(this.children[i].className=="Object" || this.children[i].className=="Text" || this.children[i].toRender){
		if(this.children[i].renderFirst) objects.unshift(this.children[i]);
			else	objects.push(this.children[i]);
		}else if(this.children[i].getObjects){
			this.children[i].getObjects(objects);
		}
	}
	return objects;
}
/**
* Gets a list of all lights in this group
* @param {array} pointer to an array [optional]
* @returns {GLGE.Lights[]} an array of GLGE.Lights
*/
GLGE.Group.prototype.getLights=function(lights){
	if(!lights) lights=[];
	for(var i=0; i<this.children.length;i++){
		if(this.children[i].className=="Light"){
			lights.push(this.children[i]);
		}else if(this.children[i].getLights){
			this.children[i].getLights(lights);
		}
	}
	return lights;
}


/**
* Adds a new object to this group
* @param {object} object the object to add to this group
*/
GLGE.Group.prototype.addChild=function(object){
	if(object.parent) object.parent.removeChild(object);
	object.matrix=null; //clear any cache
	object.parent=this;
	this.children.push(object);
	return this;
}
GLGE.Group.prototype.addObject=GLGE.Group.prototype.addChild;
GLGE.Group.prototype.addObjectInstance=GLGE.Group.prototype.addChild;
GLGE.Group.prototype.addGroup=GLGE.Group.prototype.addChild;
GLGE.Group.prototype.addText=GLGE.Group.prototype.addChild;
GLGE.Group.prototype.addSkeleton=GLGE.Group.prototype.addChild;
GLGE.Group.prototype.addLight=GLGE.Group.prototype.addChild;
GLGE.Group.prototype.addCamera=GLGE.Group.prototype.addChild;
GLGE.Group.prototype.addWavefront=GLGE.Group.prototype.addChild;
/**
* Removes an object or sub group from this group
* @param {object} object the item to remove
*/
GLGE.Group.prototype.removeChild=function(object){
	for(var i=0;i<this.children.length;i++){
		if(this.children[i]==object){
			this.children.splice(i, 1);
			if(this.scene && this.scene["remove"+object.className]){
				this.scene["remove"+object.className](object);
			}
			break;
		}
	}
}
/**
* Gets an array of all children in this group
*/
GLGE.Group.prototype.getChildren=function(){
	return this.children;
}
/**
* Initiallize all the GL stuff needed to render to screen
* @private
*/
GLGE.Group.prototype.GLInit=function(gl){
	this.gl=gl;
	for(var i=0;i<this.children.length;i++){
		if(this.children[i].GLInit){
			this.children[i].GLInit(gl);
		}
	}
}
/**
* Renders the group to the render buffer
* @private
*/
GLGE.Group.prototype.GLRender=function(gl,renderType){
	//animate this object
	if(renderType==GLGE.RENDER_DEFAULT){
		if(this.animation) this.animate();
	}
	if(!this.gl){
		this.GLInit(gl);
	}
	for(var i=0;i<this.children.length;i++){
		if(this.children[i].GLRender){
			this.children[i].GLRender(gl,renderType);
		}
	}
}


closure_export();
 
/**
* @class Class defining a channel of animation for an action
* @param {string} uid a unique reference string for this object
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.ActionChannel=function(uid){
	GLGE.Assets.registerAsset(this,uid);
}
GLGE.augment(GLGE.QuickNotation,GLGE.ActionChannel);
GLGE.augment(GLGE.JSONLoader,GLGE.ActionChannel);
/**
* Sets the name/object of the bone channel
* @param {string} name the name of the bone channel
*/
GLGE.ActionChannel.prototype.setTarget=function(object){
	this.target=object;
};
/**
* Sets the animation for this channel
* @param {GLGE.AnimationVector} animation the animation vector for this channel
*/
GLGE.ActionChannel.prototype.setAnimation=function(animation){
	this.animation=animation;
};
/**
* Gets the name/object of the bone channel
* @returns {string} the name of the bone channel
*/
GLGE.ActionChannel.prototype.getTarget=function(){
	return this.target;
};
/**
* Gets the animation vector for this channel
* @returns {GLGE.AnimationVector} the animation vector for this channel
*/
GLGE.ActionChannel.prototype.getAnimation=function(){
	return this.animation;
};

/**
* @class Class to describe and action on a skeleton
* @param {string} uid a unique reference string for this object
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.Action=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.channels=[];
};
GLGE.augment(GLGE.QuickNotation,GLGE.Action);
GLGE.augment(GLGE.JSONLoader,GLGE.Action);
/**
 * @name Action#animFinished
 * @event
 * @param {object} data
 */
GLGE.augment(GLGE.Events,GLGE.Action);

/**
* Starts playing the action
*/
GLGE.Action.prototype.start=function(blendTime,loop,names){
	if(!loop) loop=false;
	if(!blendTime) blendTime=0;
	var channels=this.channels;
	var start=(new Date()).getTime();
	this.animFinished=false;
	
	for(var i=0;i<channels.length;i++){
		var animation=channels[i].getAnimation();
		var action=this;
		var channel=channels[i];
		var target=channel.getTarget();
		if(typeof target=="string"){
			if(names && names[target]){
				target=names[target];
			}
		}
		var closure={};
		closure.finishEvent=function(data){
			target.removeEventListener("animFinished",closure.finishEvent);
			if(!action.animFinished && target.animation==animation){
				action.fireEvent("animFinished",{});
				action.animFinished=true;
			}
		}
		target.addEventListener("animFinished",closure.finishEvent);
		
		target.setAnimation(animation,blendTime,start);
		target.setLoop(loop);

	}
};
/**
* Adds and action channel to this action
* @param {GLGE.ActionChannel} channel the channel to be added
*/
GLGE.Action.prototype.addActionChannel=function(channel){
	this.channels.push(channel);
	return this;
};
/**
* Removes and action channel to this action
* @param {GLGE.ActionChannel} channel the channel to be removed
*/
GLGE.Action.prototype.removeActionChannel=function(channel){
	for(var i=0;i<this.channels.length;i++){
		if(this.channels[i]==channels){
			this.channels.splice(i,1);
			break;
		}
	}
};



/**
* @class Text that can be rendered in a scene
* @augments GLGE.Animatable
* @augments GLGE.Placeable
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.Text=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.canvas=document.createElement("canvas");
	this.color={r:1.0,g:1.0,b:1.0};
}
GLGE.augment(GLGE.Placeable,GLGE.Text);
GLGE.augment(GLGE.Animatable,GLGE.Text);
GLGE.augment(GLGE.QuickNotation,GLGE.Text);
GLGE.augment(GLGE.JSONLoader,GLGE.Text);
GLGE.Text.prototype.className="Text";
GLGE.Text.prototype.zTrans=true;
GLGE.Text.prototype.canvas=null;
GLGE.Text.prototype.aspect=1.0;
GLGE.Text.prototype.color=null;
GLGE.Text.prototype.text="";
GLGE.Text.prototype.font="Times";
GLGE.Text.prototype.size=100;
GLGE.Text.prototype.pickType=GLGE.TEXT_TEXTPICK;

/**
* Gets the pick type for this text
* @returns {string} the pick type
*/
GLGE.Text.prototype.getPickType=function(){
	return this.pickType;
};
/**
* Sets the pick type GLGE.TEXT_BOXPICK for picking based on bound box or GLGE.TEXT_TEXTPICK for pixel perfect text picking
* @param {Number} value the picking type
*/
GLGE.Text.prototype.setPickType=function(value){
	this.pickType=value;
	return this;
};
/**
* Gets the font of the text
* @returns {string} the font of the text
*/
GLGE.Text.prototype.getFont=function(){
	return this.size;
};
/**
* Sets the font of the text
* @param {Number} value the font of the text
*/
GLGE.Text.prototype.setFont=function(value){
	this.font=value;
	if(this.gl) this.updateCanvas(this.gl);
	return this;
};
/**
* Gets the size of the text
* @returns {string} the size of the text
*/
GLGE.Text.prototype.getSize=function(){
	return this.size;
};
/**
* Sets the size of the text
* @param {Number} value the size of the text
*/
GLGE.Text.prototype.setSize=function(value){
	this.size=value;
	if(this.gl) this.updateCanvas(this.gl);
	return this;
};
/**
* Gets the rendered text
* @returns {string} the text rendered
*/
GLGE.Text.prototype.getText=function(){
	return this.text;
};
/**
* Sets the text to be rendered
* @param {Number} value the text to render
*/
GLGE.Text.prototype.setText=function(value){
	this.text=value;
	if(this.gl) this.updateCanvas(this.gl);
	return this;
};
/**
* Sets the base colour of the text
* @param {string} color The colour of the material
*/
GLGE.Text.prototype.setColor=function(color){
	color=GLGE.colorParse(color);
	this.color={r:color.r,g:color.g,b:color.b};
	return this;
};
/**
* Sets the red base colour of the text
* @param {Number} r The new red level 0-1
*/
GLGE.Text.prototype.setColorR=function(value){
	this.color.r=value;
	return this;
};
/**
* Sets the green base colour of the text
* @param {Number} g The new green level 0-1
*/
GLGE.Text.prototype.setColorG=function(value){
	this.color.g=value;
	return this;
};
/**
* Sets the blue base colour of the text
* @param {Number} b The new blue level 0-1
*/
GLGE.Text.prototype.setColorB=function(value){
	this.color.b=value;
	return this;
};
/**
* Gets the current base color of the text
* @return {[r,g,b]} The current base color
*/
GLGE.Text.prototype.getColor=function(){
	return this.color;
	return this;
};

/**
* Sets the Z Transparency of this text
* @param {boolean} value Does this object need blending?
*/
GLGE.Text.prototype.setZtransparent=function(value){
	this.zTrans=value;
	return this;
}
/**
* Gets the z transparency
* @returns boolean
*/
GLGE.Text.prototype.isZtransparent=function(){
	return this.zTrans;
}
/**
* Creates the shader program for the object
* @private
*/
GLGE.Text.prototype.GLGenerateShader=function(gl){
	if(this.GLShaderProgram) gl.deleteProgram(this.GLShaderProgram);

	//Vertex Shader
	var vertexStr="";
	vertexStr+="attribute vec3 position;\n";
	vertexStr+="attribute vec2 uvcoord;\n";
	vertexStr+="varying vec2 texcoord;\n";
	vertexStr+="uniform mat4 Matrix;\n";
	vertexStr+="uniform mat4 PMatrix;\n";
	vertexStr+="varying vec4 pos;\n";
	
	vertexStr+="void main(void){\n";
	vertexStr+="texcoord=uvcoord;\n";    
	vertexStr+="pos = Matrix * vec4(position,1.0);\n";
	vertexStr+="gl_Position = PMatrix * pos;\n";
	vertexStr+="}\n";
	
	//Fragment Shader
	var fragStr="#ifdef GL_ES\nprecision highp float;\n#endif\n";
	fragStr=fragStr+"uniform sampler2D TEXTURE;\n";
	fragStr=fragStr+"varying vec2 texcoord;\n";
	fragStr=fragStr+"varying vec4 pos;\n";
	fragStr=fragStr+"uniform float far;\n";
	fragStr=fragStr+"uniform int picktype;\n";
	fragStr=fragStr+"uniform vec3 pickcolor;\n";
	fragStr=fragStr+"uniform vec3 color;\n";
	fragStr=fragStr+"void main(void){\n";
	fragStr=fragStr+"float alpha=texture2D(TEXTURE,texcoord).a;\n";
	fragStr=fragStr+"if(picktype=="+GLGE.TEXT_BOXPICK+"){gl_FragColor = vec4(pickcolor,1.0);}"
	fragStr=fragStr+"else if(picktype=="+GLGE.TEXT_TEXTPICK+"){gl_FragColor = vec4(pickcolor,alpha);}"
	fragStr=fragStr+"else{gl_FragColor = vec4(color.rgb*alpha,alpha);};\n";
	fragStr=fragStr+"}\n";
	
	this.GLFragmentShader=gl.createShader(gl.FRAGMENT_SHADER);
	this.GLVertexShader=gl.createShader(gl.VERTEX_SHADER);


	gl.shaderSource(this.GLFragmentShader, fragStr);
	gl.compileShader(this.GLFragmentShader);
	if (!gl.getShaderParameter(this.GLFragmentShader, gl.COMPILE_STATUS)) {
	      GLGE.error(gl.getShaderInfoLog(this.GLFragmentShader));
	      return;
	}
	
	//set and compile the vertex shader
	//need to set str
	gl.shaderSource(this.GLVertexShader, vertexStr);
	gl.compileShader(this.GLVertexShader);
	if (!gl.getShaderParameter(this.GLVertexShader, gl.COMPILE_STATUS)) {
		GLGE.error(gl.getShaderInfoLog(this.GLVertexShader));
		return;
	}
	
	this.GLShaderProgram = gl.createProgram();
	gl.attachShader(this.GLShaderProgram, this.GLVertexShader);
	gl.attachShader(this.GLShaderProgram, this.GLFragmentShader);
	gl.linkProgram(this.GLShaderProgram);	
}
/**
* Initiallize all the GL stuff needed to render to screen
* @private
*/
GLGE.Text.prototype.GLInit=function(gl){
	this.gl=gl;
	this.createPlane(gl);
	this.GLGenerateShader(gl);
	
	this.glTexture=gl.createTexture();
	this.updateCanvas(gl);
}
/**
* Updates the canvas texture
* @private
*/
GLGE.Text.prototype.updateCanvas=function(gl){
	var canvas = this.canvas;
	canvas.width=1;
	canvas.height=this.size*1.2;
	var ctx = canvas.getContext("2d");
	ctx.font = this.size+"px "+this.font;
	canvas.width=ctx.measureText(this.text).width;
	canvas.height=this.size*1.2;
	 ctx = canvas.getContext("2d");
	ctx.textBaseline="top";
	ctx.font = this.size+"px "+this.font;
	this.aspect=canvas.width/canvas.height;
	ctx.fillText(this.text, 0, 0);   
	
	gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
	//TODO: fix this when minefield is upto spec
	try{gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);}
	catch(e){gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas,null);}
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
	gl.generateMipmap(gl.TEXTURE_2D);
	gl.bindTexture(gl.TEXTURE_2D, null);
}

/**
* Renders the text to the render buffer
* @private
*/
GLGE.Text.prototype.GLRender=function(gl,renderType,pickindex){
	if(!this.gl){
		this.GLInit(gl);
	}
	if(renderType==GLGE.RENDER_DEFAULT || renderType==GLGE.RENDER_PICK){	
		//if look at is set then look
		if(this.lookAt) this.Lookat(this.lookAt);
		
		gl.useProgram(this.GLShaderProgram);

		var attribslot;
		//disable all the attribute initially arrays - do I really need this?
		for(var i=0; i<8; i++) gl.disableVertexAttribArray(i);
		attribslot=GLGE.getAttribLocation(gl,this.GLShaderProgram, "position");

		gl.bindBuffer(gl.ARRAY_BUFFER, this.posBuffer);
		gl.enableVertexAttribArray(attribslot);
		gl.vertexAttribPointer(attribslot, this.posBuffer.itemSize, gl.FLOAT, false, 0, 0);
		
		attribslot=GLGE.getAttribLocation(gl,this.GLShaderProgram, "uvcoord");
		gl.bindBuffer(gl.ARRAY_BUFFER, this.uvBuffer);
		gl.enableVertexAttribArray(attribslot);
		gl.vertexAttribPointer(attribslot, this.uvBuffer.itemSize, gl.FLOAT, false, 0, 0);
		
		gl.activeTexture(gl["TEXTURE0"]);
		gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
		gl.uniform1i(GLGE.getUniformLocation(gl,this.GLShaderProgram, "TEXTURE"), 0);	
		
		if(!pickindex) pickindex=0;
		var b = pickindex >> 16 & 0xFF; 
		var g = pickindex >> 8 & 0xFF; 
		var r = pickindex & 0xFF;
		gl.uniform3f(GLGE.getUniformLocation(gl,this.GLShaderProgram, "pickcolor"), r/255,g/255,b/255);
			
		if(renderType==GLGE.RENDER_PICK){
			gl.uniform1i(GLGE.getUniformLocation(gl,this.GLShaderProgram, "picktype"), this.pickType);	
		}else{
			gl.uniform1i(GLGE.getUniformLocation(gl,this.GLShaderProgram, "picktype"), 0);	
		}
		
		if(!this.GLShaderProgram.glarrays) this.GLShaderProgram.glarrays={};

		
		//generate and set the modelView matrix
		var scalefactor=this.size/100;
		var mMatrix=GLGE.mulMat4(gl.scene.camera.getViewMatrix(),GLGE.mulMat4(this.getModelMatrix(),GLGE.scaleMatrix(this.aspect*scalefactor,scalefactor,scalefactor)));
		var mUniform = GLGE.getUniformLocation(gl,this.GLShaderProgram, "Matrix");
		if(!this.GLShaderProgram.glarrays.mMatrix) this.GLShaderProgram.glarrays.mMatrix=new Float32Array(mMatrix);
			else GLGE.mat4gl(mMatrix,this.GLShaderProgram.glarrays.mMatrix);
		gl.uniformMatrix4fv(mUniform, true, this.GLShaderProgram.glarrays.mMatrix);
		
		var mUniform = GLGE.getUniformLocation(gl,this.GLShaderProgram, "PMatrix");

		if(!this.GLShaderProgram.glarrays.pMatrix) this.GLShaderProgram.glarrays.pMatrix=new Float32Array(gl.scene.camera.getProjectionMatrix());
			else GLGE.mat4gl(gl.scene.camera.getProjectionMatrix(),this.GLShaderProgram.glarrays.pMatrix);
		gl.uniformMatrix4fv(mUniform, true, this.GLShaderProgram.glarrays.pMatrix);

		
		var farUniform = GLGE.getUniformLocation(gl,this.GLShaderProgram, "far");
		gl.uniform1f(farUniform, gl.scene.camera.getFar());
		//set the color
		gl.uniform3f(GLGE.getUniformLocation(gl,this.GLShaderProgram, "color"), this.color.r,this.color.g,this.color.b);
		
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.GLfaces);
		gl.drawElements(gl.TRIANGLES, this.GLfaces.numItems, gl.UNSIGNED_SHORT, 0);
	}
}
/**
* creates the plane mesh to draw
* @private
*/
GLGE.Text.prototype.createPlane=function(gl){
	//create the vertex positions
	if(!this.posBuffer) this.posBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.posBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([1,1,0,-1,1,0,-1,-1,0,1,-1,0]), gl.STATIC_DRAW);
	this.posBuffer.itemSize = 3;
	this.posBuffer.numItems = 4;
	//create the vertex uv coords
	if(!this.uvBuffer) this.uvBuffer = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.uvBuffer);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([0,0,1,0,1,1,0,1]), gl.STATIC_DRAW);
	this.uvBuffer.itemSize = 2;
	this.uvBuffer.numItems = 4;
	//create the faces
	if(!this.GLfaces) this.GLfaces = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.GLfaces);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array([0,1,2,2,3,0]), gl.STATIC_DRAW);
	this.GLfaces.itemSize = 1;
	this.GLfaces.numItems = 6;
}



/**
* @class Creates a new load for a multimaterial
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.ObjectLod=function(uid){
	GLGE.Assets.registerAsset(this,uid);
}
GLGE.augment(GLGE.QuickNotation,GLGE.ObjectLod);
GLGE.augment(GLGE.JSONLoader,GLGE.ObjectLod);
GLGE.ObjectLod.prototype.mesh=null;
GLGE.ObjectLod.prototype.className="ObjectLod";
GLGE.ObjectLod.prototype.material=null;
GLGE.ObjectLod.prototype.program=null;
GLGE.ObjectLod.prototype.GLShaderProgramPick=null;
GLGE.ObjectLod.prototype.GLShaderProgramShadow=null;
GLGE.ObjectLod.prototype.GLShaderProgram=null;
GLGE.ObjectLod.prototype.pixelSize=0;

/**
* sets the mesh
* @param {GLGE.Mesh} mesh 
*/
GLGE.ObjectLod.prototype.setMesh=function(mesh){
	if(typeof mesh=="string")  mesh=GLGE.Assets.get(mesh);
	
	//remove event listener from current material
	if(this.mesh){
		this.mesh.removeEventListener("shaderupdate",this.meshupdated);
	}
	var multiMaterial=this;
	this.meshupdated=function(event){
		multiMaterial.GLShaderProgram=null;
	};
	//set event listener for new material
	mesh.addEventListener("shaderupdate",this.meshupdated);
	
	this.GLShaderProgram=null;
	this.mesh=mesh;
	return this;
}
/**
* gets the mesh
* @returns {GLGE.Mesh}
*/
GLGE.ObjectLod.prototype.getMesh=function(){
	return this.mesh;
}
/**
* sets the material
* @param {GLGE.Material} material 
*/
GLGE.ObjectLod.prototype.setMaterial=function(material){
	if(typeof material=="string")  material=GLGE.Assets.get(material);
	
	//remove event listener from current material
	if(this.material){
		this.material.removeEventListener("shaderupdate",this.materialupdated);
	}
	var ObjectLOD=this;
	this.materialupdated=function(event){
		ObjectLOD.GLShaderProgram=null;
	};
	//set event listener for new material
	material.addEventListener("shaderupdate",this.materialupdated);
	
	this.GLShaderProgram=null;
	this.material=material;
	return this;
}
/**
* gets the material
* @returns {GLGE.Material}
*/
GLGE.ObjectLod.prototype.getMaterial=function(){
	return this.material;
}

/**
* gets the pixelsize limit for this lod
* @returns {number}
*/
GLGE.ObjectLod.prototype.getPixelSize=function(){
	return this.pixelSize;
}
/**
* sets the pixelsize limit for this lod
* @returns {number}
*/
GLGE.ObjectLod.prototype.setPixelSize=function(value){
	this.pixelSize=value;
}


/**
* @class Creates a new mesh/material to add to an object
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.MultiMaterial=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.lods=[new GLGE.ObjectLod]
}
GLGE.augment(GLGE.QuickNotation,GLGE.MultiMaterial);
GLGE.augment(GLGE.JSONLoader,GLGE.MultiMaterial);
GLGE.MultiMaterial.prototype.className="MultiMaterial";
/**
* sets the mesh
* @param {GLGE.Mesh} mesh 
*/
GLGE.MultiMaterial.prototype.setMesh=function(mesh){
	this.lods[0].setMesh(mesh);
	return this;
}
/**
* gets the mesh
* @returns {GLGE.Mesh}
*/
GLGE.MultiMaterial.prototype.getMesh=function(){
	return this.lods[0].getMesh();
}
/**
* sets the material
* @param {GLGE.Material} material 
*/
GLGE.MultiMaterial.prototype.setMaterial=function(material){
	this.lods[0].setMaterial(material);
	return this;
}
/**
* gets the material
* @returns {GLGE.Material}
*/
GLGE.MultiMaterial.prototype.getMaterial=function(){
	return this.lods[0].getMaterial();
}

/**
* returns the load for a given pixel size
* @param {number} pixelsize the current pixel size of the object
* @returns {GLGE.ObjectLod}
*/
GLGE.MultiMaterial.prototype.getLOD=function(pixelsize){
	var currentSize=0;
	var currentLOD=this.lods[0];
	if(this.lods.length>1){
		for(var i=0; i<this.lods.length;i++){
			var size=this.lods[i].pixelSize
			if(size>currentSize && size<pixelsize){
				currentSize=size;
				currentLOD=this.lods[i];
			}
		}
	}
	return currentLOD;
}

/**
* adds a lod to this multimaterial
* @param {GLGE.ObjectLod} lod the lod to add
*/
GLGE.MultiMaterial.prototype.addObjectLod=function(lod){
	this.lods.push(lod);
	return this;
}

/**
* removes a lod to this multimaterial
* @param {GLGE.ObjectLod} lod the lod to remove
*/
GLGE.MultiMaterial.prototype.removeObjectLod=function(lod){
	var idx=this.lods.indexOf(lod);
	if(idx) this.lods.splice(idx,1);
	return this;
}



/**
* @class An additional instance of an object that can be rendered in a scene
* @augments GLGE.Animatable
* @augments GLGE.Placeable
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.ObjectInstance=function(uid){
	GLGE.Assets.registerAsset(this,uid);
}
GLGE.augment(GLGE.Placeable,GLGE.ObjectInstance);
GLGE.augment(GLGE.Animatable,GLGE.ObjectInstance);
GLGE.augment(GLGE.QuickNotation,GLGE.ObjectInstance);
GLGE.augment(GLGE.JSONLoader,GLGE.ObjectInstance);
GLGE.ObjectInstance.prototype.parentObject=null;
GLGE.ObjectInstance.prototype.className="ObjectInstance";
/**
* Sets the parent object to instance
* @param {GLGE.Object} value the object to instance
*/
GLGE.ObjectInstance.prototype.setObject=function(value){
	if(this.parentObject) this.parentObject.removeInstance(this);
	this.parentObject=value;
	value.addInstance(this);
	return this;
}
/**
* Gets the Object being instanced
* @returns boolean
*/
GLGE.ObjectInstance.prototype.getObject=function(){
	return this.parentObject;
}




/**
* @class An object that can be rendered in a scene
* @augments GLGE.Animatable
* @augments GLGE.Placeable
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.Object=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.multimaterials=[];
	this.instances=[];
}
GLGE.augment(GLGE.Placeable,GLGE.Object);
GLGE.augment(GLGE.Animatable,GLGE.Object);
GLGE.augment(GLGE.QuickNotation,GLGE.Object);
GLGE.augment(GLGE.JSONLoader,GLGE.Object);
GLGE.Object.prototype.className="Object";
GLGE.Object.prototype.mesh=null;
GLGE.Object.prototype.skeleton=null;
GLGE.Object.prototype.scene=null;
GLGE.Object.prototype.transformMatrix=GLGE.identMatrix();
GLGE.Object.prototype.material=null;
GLGE.Object.prototype.gl=null;
GLGE.Object.prototype.multimaterials=null;
GLGE.Object.prototype.instances=null;
GLGE.Object.prototype.zTrans=false;
GLGE.Object.prototype.id="";
GLGE.Object.prototype.pickable=true;
GLGE.Object.prototype.drawType=GLGE.DRAW_TRIS;
GLGE.Object.prototype.pointSize=1;

//shadow fragment
var shfragStr=[];
shfragStr.push("#ifdef GL_ES\nprecision highp float;\n#endif\n");
shfragStr.push("void main(void)\n");
shfragStr.push("{\n");
shfragStr.push("vec4 rgba=fract((gl_FragCoord.z/gl_FragCoord.w)/10000.0 * vec4(16777216.0, 65536.0, 256.0, 1.0));\n");
shfragStr.push("gl_FragColor=rgba-rgba.rrgb*vec4(0.0,0.00390625,0.00390625,0.00390625);\n");
shfragStr.push("}\n");
GLGE.Object.prototype.shfragStr=shfragStr.join("");

//normal fragment
var nfragStr=[];
nfragStr.push("#ifdef GL_ES\nprecision highp float;\n#endif\n");
nfragStr.push("varying vec3 n;\n");
nfragStr.push("void main(void)\n");
nfragStr.push("{\n");
nfragStr.push("gl_FragColor=vec4(n,1.0);\n");
nfragStr.push("}\n");
GLGE.Object.prototype.nfragStr=nfragStr.join("");

//picking fragment
var pkfragStr=[];
pkfragStr.push("#ifdef GL_ES\nprecision highp float;\n#endif\n");
pkfragStr.push("uniform float far;\n");
pkfragStr.push("uniform vec3 pickcolor;\n");
pkfragStr.push("varying vec3 n;\n");
pkfragStr.push("varying vec4 UVCoord;\n");
pkfragStr.push("void main(void)\n");
pkfragStr.push("{\n");
pkfragStr.push("float Xcoord = gl_FragCoord.x+0.5;\n");
pkfragStr.push("if(Xcoord>0.0) gl_FragColor = vec4(pickcolor,1.0);\n");
pkfragStr.push("if(Xcoord>1.0) gl_FragColor = vec4(n,1.0);\n");
pkfragStr.push("if(Xcoord>2.0){");	
pkfragStr.push("vec3 rgb=fract((gl_FragCoord.z/gl_FragCoord.w) * vec3(65536.0, 256.0, 1.0));\n");
pkfragStr.push("gl_FragColor=vec4(rgb-rgb.rrg*vec3(0.0,0.00390625,0.00390625),1.0);\n");
pkfragStr.push("}");
//x tex coord
pkfragStr.push("if(Xcoord>3.0){");	
pkfragStr.push("vec3 rgb=fract(UVCoord.x * vec3(65536.0, 256.0, 1.0));\n");
pkfragStr.push("gl_FragColor=vec4(rgb-rgb.rrg*vec3(0.0,0.00390625,0.00390625),1.0);\n");
pkfragStr.push("}");
//y tex coord
pkfragStr.push("if(Xcoord>4.0){");	
pkfragStr.push("vec3 rgb=fract(UVCoord.y * vec3(65536.0, 256.0, 1.0));\n");
pkfragStr.push("gl_FragColor=vec4(rgb-rgb.rrg*vec3(0.0,0.00390625,0.00390625),1.0);\n");
pkfragStr.push("}");
pkfragStr.push("}\n");
GLGE.Object.prototype.pkfragStr=pkfragStr.join("");


/**
* Gets the objects draw type
*/
GLGE.Object.prototype.getDrawType=function(){
	return this.drawType;
}
/**
* Sets the objects draw type
* @param {GLGE.number} value the draw type of this object
*/
GLGE.Object.prototype.setDrawType=function(value){
	this.drawType=value;
	return this;
}

/**
* Gets the objects draw point size
*/
GLGE.Object.prototype.getPointSize=function(){
	return this.pointSize;
}
/**
* Sets the objects draw points size
* @param {GLGE.number} value the point size to render
*/
GLGE.Object.prototype.setPointSize=function(value){
	this.pointSize=parseFloat(value);
	return this;
}


/**
* Gets the objects skeleton
* @returns GLGE.Group
*/
GLGE.Object.prototype.getSkeleton=function(){
	return this.skeleton;
}
/**
* Sets the objects skeleton
* @param {GLGE.Group} value the skeleton group to set
*/
GLGE.Object.prototype.setSkeleton=function(value){
	this.skeleton=value;
	this.bones=value.getNames();
	return this;
}

GLGE.Object.prototype.getBoundingVolume=function(){
	var multimaterials=this.multimaterials;
	this.boundingVolume=new GLGE.BoundingVolume(0,0,0,0,0,0);
	for(var i=0;i<multimaterials.length;i++){
		this.boundingVolume.addBoundingVolume(multimaterials[i].lods[0].mesh.getBoundingVolume());
	}
	this.boundingVolume.applyMatrixScale(this.getModelMatrix());

	return this.boundingVolume;
}

/**
* Sets the Z Transparency of this object
* @param {boolean} value Does this object need blending?
*/
GLGE.Object.prototype.setZtransparent=function(value){
	this.zTrans=value;
	return this;
}
/**
* Gets the z transparency
* @returns boolean
*/
GLGE.Object.prototype.isZtransparent=function(){
	return this.zTrans;
}

/**
* Adds a new instance of this object
* @param {GLGE.ObjectInstance} value the instance to add
*/
GLGE.Object.prototype.addInstance=function(value){
	this.instances.push(value);
	return this;
}

/**
* Removes an instance of this object
* @param {GLGE.ObjectInstance} value the instance to remove
*/
GLGE.Object.prototype.removeInstance=function(value){
	for(var i=0; i<this.instances;i++){
		if(this.instance==value) this.instances.splice(i);
	}
}

/**
* Sets the material associated with the object
* @param GLGE.Material
*/
GLGE.Object.prototype.setMaterial=function(material,idx){
	if(typeof material=="string")  material=GLGE.Assets.get(material);
	if(!idx) idx=0;
	if(!this.multimaterials[idx]) this.multimaterials[idx]=new GLGE.MultiMaterial();
	if(this.multimaterials[idx].getMaterial()!=material){
		this.multimaterials[idx].setMaterial(material);
		this.updateProgram();
	}
	return this;
}
/**
* Gets the material associated with the object
* @returns GLGE.Material
*/
GLGE.Object.prototype.getMaterial=function(idx){
	if(!idx) idx=0;
	if(this.multimaterials[idx]) {
		return this.multimaterials[idx].getMaterial();
	}else{
		return false;
	}
}
/**
* Sets the mesh associated with the object
* @param GLGE.Mesh
*/
GLGE.Object.prototype.setMesh=function(mesh,idx){
	if(typeof mesh=="string")  mesh=GLGE.Assets.get(mesh);
	if(!idx) idx=0;
	if(!this.multimaterials[idx]) this.multimaterials.push(new GLGE.MultiMaterial());
	this.multimaterials[idx].setMesh(mesh);
	return this;
}
/**
* Gets the mesh associated with the object
* @returns GLGE.Mesh
*/
GLGE.Object.prototype.getMesh=function(idx){
	if(!idx) idx=0;
	if(this.multimaterials[idx]) {
		this.multimaterials[idx].getMesh();
	}else{
		return false;
	}
}
/**
* Initiallize all the GL stuff needed to render to screen
* @private
*/
GLGE.Object.prototype.GLInit=function(gl){
	this.gl=gl;
}
/**
* Cleans up all the GL stuff we sets
* @private
*/
GLGE.Object.prototype.GLDestory=function(gl){
}
/**
* Updates the GL shader program for the object
* @private
*/
GLGE.Object.prototype.updateProgram=function(){
	for(var i=0; i<this.multimaterials.length;i++){
		this.multimaterials[i].GLShaderProgram=null;
	}
}
/**
* Adds another material to this object
* @returns GLGE.Material
*/
GLGE.Object.prototype.addMultiMaterial=function(multimaterial){
	if(typeof multimaterial=="string")  multimaterial=GLGE.Assets.get(multimaterial);
	this.multimaterials.push(multimaterial);
}
/**
* gets all of the objects materials and meshes
* @returns array of GLGE.MultiMaterial objects
*/
GLGE.Object.prototype.getMultiMaterials=function(){
	return this.multimaterials;
}
/**
* Creates the shader program for the object
* @private
*/
GLGE.Object.prototype.GLGenerateShader=function(gl){
	//create the programs strings
	//Vertex Shader
	var UV=joints1=joints2=false;
	var lights=gl.lights;
	var vertexStr=[];
	var tangent=false;
	if(!this.mesh.normals) this.mesh.calcNormals();
	for(var i=0;i<this.mesh.buffers.length;i++){
		if(this.mesh.buffers[i].name=="tangent") tangent=true;
		if(this.mesh.buffers[i].size>1){
			vertexStr.push("attribute vec"+this.mesh.buffers[i].size+" "+this.mesh.buffers[i].name+";\n");
		}else{
			vertexStr.push("attribute float "+this.mesh.buffers[i].name+";\n");
		}
		if(this.mesh.buffers[i].name=="UV") UV=true;
		if(this.mesh.buffers[i].name=="joints1") joints1=this.mesh.buffers[i];
		if(this.mesh.buffers[i].name=="joints2") joints2=this.mesh.buffers[i];
	}
	vertexStr.push("uniform mat4 worldView;\n");
	vertexStr.push("uniform mat4 projection;\n");  
	vertexStr.push("uniform mat4 view;\n");  
	vertexStr.push("uniform mat4 worldInverseTranspose;\n");
	vertexStr.push("uniform mat4 envMat;\n");

	for(var i=0; i<lights.length;i++){
			vertexStr.push("uniform vec3 lightpos"+i+";\n");
			vertexStr.push("uniform vec3 lightdir"+i+";\n");
			vertexStr.push("uniform mat4 lightmat"+i+";\n");
			vertexStr.push("varying vec4 spotcoord"+i+";\n");
	}
	
	vertexStr.push("varying vec3 eyevec;\n"); 
	for(var i=0; i<lights.length;i++){
			vertexStr.push("varying vec3 lightvec"+i+";\n"); 
			vertexStr.push("varying vec3 tlightvec"+i+";\n"); 
			vertexStr.push("varying float lightdist"+i+";\n"); 
	}
	
	if(this.mesh.joints && this.mesh.joints.length>0){
		vertexStr.push("uniform mat4 jointMat["+(this.mesh.joints.length)+"];\n"); 
		vertexStr.push("uniform mat4 jointNMat["+(this.mesh.joints.length)+"];\n"); 
	}
	
	if(this.material) vertexStr.push(this.material.getVertexVarying(vertexStr));
    
	vertexStr.push("varying vec3 n;\n");  
	vertexStr.push("varying vec3 b;\n");  
	vertexStr.push("varying vec3 t;\n");  
	
	vertexStr.push("varying vec4 UVCoord;\n");
	vertexStr.push("varying vec3 OBJCoord;\n");
	vertexStr.push("varying vec3 tang;\n");
	vertexStr.push("varying vec3 teyevec;\n");
	
	vertexStr.push("void main(void)\n");
	vertexStr.push("{\n");
	if(UV) vertexStr.push("UVCoord=UV;\n");
	vertexStr.push("OBJCoord = position;\n");
	vertexStr.push("vec4 pos = vec4(0.0, 0.0, 0.0, 1.0);\n");
	vertexStr.push("vec4 norm = vec4(0.0, 0.0, 0.0, 1.0);\n");
	vertexStr.push("vec4 tang4 = vec4(0.0, 0.0, 0.0, 1.0);\n");
	
	if(joints1){
		if(joints1.size==1){
			vertexStr.push("pos += jointMat[int(joints1)]*vec4(position,1.0)*weights1;\n");
			vertexStr.push("norm += jointNMat[int(joints1)]*vec4(normal,1.0)*weights1;\n");  
			if(tangent) vertexStr.push("tang4 +=  jointNMat[int(joints1)]*vec4(tangent,1.0)*weights1;\n");
		}else{
			for(var i=0;i<joints1.size;i++){
				vertexStr.push("pos += jointMat[int(joints1["+i+"])]*vec4(position,1.0)*weights1["+i+"];\n");
				vertexStr.push("norm += jointNMat[int(joints1["+i+"])]*vec4(normal,1.0)*weights1["+i+"];\n");  
				if(tangent) vertexStr.push("tang4 +=  jointNMat[int(joints1["+i+"])]*vec4(tangent,1.0)*weights1["+i+"];\n");
			}
		}
		if(joints2){
			if(joints2.size==1){
				vertexStr.push("pos += jointMat[int(joints2)]*vec4(position,1.0)*weights2;\n");
				vertexStr.push("norm += jointNMat[int(joints2)]*vec4(normal,1.0)*weights2;\n");  
				if(tangent) vertexStr.push("tang4 +=  jointNMat[int(joints2)]*vec4(tangent,1.0)*weights2;\n");
			}else{
				for(var i=0;i<joints2.size;i++){
					vertexStr.push("pos += jointMat[int(joints2["+i+"])]*vec4(position,1.0)*weights2["+i+"];\n");
					vertexStr.push("norm += jointNMat[int(joints2["+i+"])]*vec4(normal,1.0)*weights2["+i+"];\n");  
					if(tangent) vertexStr.push("tang4 +=  jointNMat[int(joints2["+i+"])]*vec4(tangent,1.0)*weights2["+i+"];\n");
				}
			}
		}
		
		for(var i=0; i<lights.length;i++){
			vertexStr.push("spotcoord"+i+"=lightmat"+i+"*vec4(pos.xyz,1.0);\n");
		}
		vertexStr.push("pos = worldView * vec4(pos.xyz, 1.0);\n");
		vertexStr.push("norm = worldInverseTranspose * vec4(norm.xyz, 1.0);\n");
		if(tangent) vertexStr.push("tang = (worldInverseTranspose*vec4(tang4.xyz,1.0)).xyz;\n");
	}else{	
		for(var i=0; i<lights.length;i++){
			vertexStr.push("spotcoord"+i+"=lightmat"+i+"*vec4(position,1.0);\n");
		}
		vertexStr.push("pos = worldView * vec4(position, 1.0);\n");
		vertexStr.push("norm = worldInverseTranspose * vec4(normal, 1.0);\n");  
		if(tangent) vertexStr.push("tang = (worldInverseTranspose*vec4(tangent,1.0)).xyz;\n");
	}
    
	vertexStr.push("gl_Position = projection * pos;\n");
	vertexStr.push("gl_PointSize="+(this.pointSize.toFixed(5))+";\n");
	
	vertexStr.push("eyevec = -pos.xyz;\n");
	
	vertexStr.push("t = normalize(tang);");
	vertexStr.push("n = normalize(norm.rgb);");
	vertexStr.push("b = normalize(cross(n,t));");
	if(tangent){
		vertexStr.push("teyevec.x = dot(eyevec, t);");
		vertexStr.push("teyevec.y = dot(eyevec, b);");
		vertexStr.push("teyevec.z = dot(eyevec, n);");
	}else{
		vertexStr.push("teyevec = eyevec;");
	}
	
	for(var i=0; i<lights.length;i++){			
			if(lights[i].getType()==GLGE.L_DIR){
				vertexStr.push("vec3 tmplightvec"+i+" = -lightdir"+i+";\n");
			}else{
				vertexStr.push("vec3 tmplightvec"+i+" = -(lightpos"+i+"-pos.xyz);\n");
			}
			//tan space stuff
			if(tangent){
				vertexStr.push("tlightvec"+i+".x = dot(tmplightvec"+i+", t);");
				vertexStr.push("tlightvec"+i+".y = dot(tmplightvec"+i+", b);");
				vertexStr.push("tlightvec"+i+".z = dot(tmplightvec"+i+", n);");
				
			}else{
				vertexStr.push("tlightvec"+i+" = tmplightvec"+i+";");
			}
			vertexStr.push("lightvec"+i+" = tmplightvec"+i+";");

			
			vertexStr.push("lightdist"+i+" = length(lightpos"+i+".xyz-pos.xyz);\n");
	}
	if(this.material) vertexStr.push(this.material.getLayerCoords(vertexStr));
	vertexStr.push("}\n");
	
	vertexStr=vertexStr.join("");

	//Fragment Shader
	if(!this.material){
		var fragStr=[];
		fragStr.push("void main(void)\n");
		fragStr.push("{\n");
		fragStr.push("gl_FragColor = vec4(1.0,1.0,1.0,1.0);\n");
		fragStr.push("}\n");
		fragStr=fragStr.join("");
	}
	else
	{
		fragStr=this.material.getFragmentShader(lights);
	}
	
	this.GLFragmentShaderNormal=GLGE.getGLShader(gl,gl.FRAGMENT_SHADER,this.nfragStr);
	this.GLFragmentShaderShadow=GLGE.getGLShader(gl,gl.FRAGMENT_SHADER,this.shfragStr);
	this.GLFragmentShaderPick=GLGE.getGLShader(gl,gl.FRAGMENT_SHADER,this.pkfragStr);
	this.GLFragmentShader=GLGE.getGLShader(gl,gl.FRAGMENT_SHADER,fragStr);
	this.GLVertexShader=GLGE.getGLShader(gl,gl.VERTEX_SHADER,vertexStr);

	this.GLShaderProgramPick=GLGE.getGLProgram(gl,this.GLVertexShader,this.GLFragmentShaderPick);
	this.GLShaderProgramShadow=GLGE.getGLProgram(gl,this.GLVertexShader,this.GLFragmentShaderShadow);
	this.GLShaderProgramNormal=GLGE.getGLProgram(gl,this.GLVertexShader,this.GLFragmentShaderNormal);
	this.GLShaderProgram=GLGE.getGLProgram(gl,this.GLVertexShader,this.GLFragmentShader);
}
/**
* creates shader programs;
* @param multimaterial the multimaterial object to create the shader programs for
* @private
*/
GLGE.Object.prototype.createShaders=function(multimaterial){
	if(this.gl){
		this.mesh=multimaterial.mesh;
		this.material=multimaterial.material;
		this.GLGenerateShader(this.gl);
		multimaterial.GLShaderProgramPick=this.GLShaderProgramPick;
		multimaterial.GLShaderProgramShadow=this.GLShaderProgramShadow;
		multimaterial.GLShaderProgram=this.GLShaderProgram;
	}
}
/**
* Sets the shader program uniforms ready for rendering
* @private
*/
GLGE.Object.prototype.GLUniforms=function(gl,renderType,pickindex){
	var program;
	switch(renderType){
		case GLGE.RENDER_DEFAULT:
			program=this.GLShaderProgram;
			break;
		case GLGE.RENDER_SHADOW:
			program=this.GLShaderProgramShadow;
			break;
		case GLGE.RENDER_NORMAL:
			program=this.GLShaderProgramNormal;
			break;
		case GLGE.RENDER_PICK:
			program=this.GLShaderProgramPick;
			var b = pickindex >> 16 & 0xFF; 
			var g = pickindex >> 8 & 0xFF; 
			var r = pickindex & 0xFF;
			gl.uniform3f(GLGE.getUniformLocation(gl,program, "pickcolor"), r/255,g/255,b/255);
			break;
	}
	
	
	if(!program.caches) program.caches={};
	if(!program.glarrays) program.glarrays={};

	if(program.caches.far!=gl.scene.camera.far){
		gl.uniform1f(GLGE.getUniformLocation(gl,program, "far"), gl.scene.camera.far);
		program.caches.far=gl.scene.camera.far;
	}
	if(renderType==GLGE.RENDER_DEFAULT){
		if(program.caches.ambientColor!=gl.scene.ambientColor){
			gl.uniform3f(GLGE.getUniformLocation(gl,program, "amb"), gl.scene.ambientColor.r,gl.scene.ambientColor.g,gl.scene.ambientColor.b);
			program.caches.ambientColor=gl.scene.ambientColor;
		}
		if(program.caches.fogFar!=gl.scene.fogFar){
			gl.uniform1f(GLGE.getUniformLocation(gl,program, "fogfar"), gl.scene.fogFar);
			program.caches.fogFar=gl.scene.fogFar;
		}
		if(program.caches.fogNear!=gl.scene.fogNear){
			gl.uniform1f(GLGE.getUniformLocation(gl,program, "fognear"), gl.scene.fogNear);
			program.caches.fogNear=gl.scene.fogNear;
		}
		if(program.caches.fogType!=gl.scene.fogType){
			gl.uniform1i(GLGE.getUniformLocation(gl,program, "fogtype"), gl.scene.fogType);
			program.caches.fogType=gl.scene.fogType;
		}
		if(program.caches.fogType!=gl.scene.fogcolor){
			gl.uniform3f(GLGE.getUniformLocation(gl,program, "fogcolor"), gl.scene.fogColor.r,gl.scene.fogColor.g,gl.scene.fogColor.b);
			program.caches.fogcolor=gl.scene.fogcolor;
		}
	}

			
	
	var cameraMatrix=gl.scene.camera.getViewMatrix();
	var modelMatrix=this.getModelMatrix();
	if(!program.caches.mvMatrix) program.caches.mvMatrix={cameraMatrix:null,modelMatrix:null};
	var mvCache=program.caches.mvMatrix;
	
	if(mvCache.camerMatrix!=cameraMatrix || mvCache.modelMatrix!=modelMatrix){
		try{
		//generate and set the modelView matrix
		if(!this.caches.mvMatrix) this.caches.mvMatrix=GLGE.mulMat4(cameraMatrix,modelMatrix);
		mvMatrix=this.caches.mvMatrix;
					
		var mvUniform = GLGE.getUniformLocation(gl,program, "worldView");
		if(!program.glarrays.mvMatrix) program.glarrays.mvMatrix=new Float32Array(mvMatrix);
			else GLGE.mat4gl(mvMatrix,program.glarrays.mvMatrix);
		gl.uniformMatrix4fv(mvUniform, true, program.glarrays.mvMatrix);

	    
		//invCamera matrix
		if(!this.caches.envMat){
			var envMat = GLGE.inverseMat4(mvMatrix);
			envMat[3]=0;
			envMat[7]=0;
			envMat[11]=0;
			this.caches.envMat = envMat;
		}
		envMat=this.caches.envMat;
		var icUniform = GLGE.getUniformLocation(gl,program, "envMat");
		
		if(!program.glarrays.envMat) program.glarrays.envMat=new Float32Array(envMat);
			else GLGE.mat4gl(envMat,program.glarrays.envMat);	
		gl.uniformMatrix4fv(icUniform, true, program.glarrays.envMat);
	    
		//normalising matrix
		if(!this.caches.normalMatrix){
			var normalMatrix = GLGE.inverseMat4(mvMatrix);
			this.caches.normalMatrix = normalMatrix;
		}
		normalMatrix=this.caches.normalMatrix;
		var nUniform = GLGE.getUniformLocation(gl,program, "worldInverseTranspose");
		
		if(!program.glarrays.normalMatrix) program.glarrays.normalMatrix=new Float32Array(normalMatrix);
			else GLGE.mat4gl(normalMatrix,program.glarrays.normalMatrix);	
		gl.uniformMatrix4fv(nUniform, false, program.glarrays.normalMatrix);
		
		var cUniform = GLGE.getUniformLocation(gl,program, "view");
		if(!program.glarrays.cameraMatrix) program.glarrays.cameraMatrix=new Float32Array(cameraMatrix);
			else GLGE.mat4gl(cameraMatrix,program.glarrays.cameraMatrix);	
		gl.uniformMatrix4fv(cUniform, true, program.glarrays.cameraMatrix);
		
		mvCache.camerMatrix=cameraMatrix;
		mvCache.modelMatrix!=modelMatrix;
		}catch(e){}
	}
	
	try{
	var pUniform = GLGE.getUniformLocation(gl,program, "projection");
	if(!program.glarrays.pMatrix) program.glarrays.pMatrix=new Float32Array(gl.scene.camera.getProjectionMatrix());
			else GLGE.mat4gl(gl.scene.camera.getProjectionMatrix(),program.glarrays.pMatrix);	
	gl.uniformMatrix4fv(pUniform, true, program.glarrays.pMatrix);
	}catch(e){}
	
	//light
	//dont' need lighting for picking
	if(renderType==GLGE.RENDER_DEFAULT){
		var pos,lpos;
		var lights=gl.lights
		if(!program.caches.lights) program.caches.lights=[];
		if(!program.glarrays.lights) program.glarrays.lights=[];
		if(!this.caches.lights) this.caches.lights=[];
		var lightCache=program.caches.lights;
		for(var i=0; i<lights.length;i++){
			if(!lightCache[i]) lightCache[i]={modelMatrix:null,cameraMatrix:null};
			if(lightCache[i].modelMatrix!=modelMatrix || lightCache[i].cameraMatrix!=cameraMatrix){
				if(!this.caches.lights[i])this.caches.lights[i]={};
				
				if(!this.caches.lights[i].pos) this.caches.lights[i].pos=GLGE.mulMat4Vec4(GLGE.mulMat4(cameraMatrix,lights[i].getModelMatrix()),[0,0,0,1]);
				pos=this.caches.lights[i].pos;
				gl.uniform3f(GLGE.getUniformLocation(gl,program, "lightpos"+i), pos[0],pos[1],pos[2]);		
				
				
				if(!this.caches.lights[i].lpos) this.caches.lights[i].lpos=GLGE.mulMat4Vec4(GLGE.mulMat4(cameraMatrix,lights[i].getModelMatrix()),[0,0,1,1]);
				lpos=this.caches.lights[i].lpos;
				gl.uniform3f(GLGE.getUniformLocation(gl,program, "lightdir"+i),lpos[0]-pos[0],lpos[1]-pos[1],lpos[2]-pos[2]);
				
				if(lights[i].s_cache){
					try{
					var lightmat=GLGE.mulMat4(lights[i].s_cache.smatrix,modelMatrix);
					if(!program.glarrays.lights[i]) program.glarrays.lights[i]=new Float32Array(lightmat);
						else GLGE.mat4gl(lightmat,program.glarrays.lights[i]);
					gl.uniformMatrix4fv(GLGE.getUniformLocation(gl,program, "lightmat"+i), true,program.glarrays.lights[i]);
					lightCache[i].modelMatrix=modelMatrix;
					lightCache[i].cameraMatrix=cameraMatrix;
					}catch(e){}
				}else{
					lightCache[i].modelMatrix=modelMatrix;
					lightCache[i].cameraMatrix=cameraMatrix;
				}
			}
		}
	}
	
	if(this.mesh.joints){
		if(!program.caches.joints) program.caches.joints=[];
		if(!program.glarrays.joints) program.glarrays.joints=[];
		if(!program.glarrays.jointsinv) program.glarrays.jointsinv=[];
		var jointCache=program.caches.joints;
			var ident=GLGE.identMatrix();
			for(i=0;i<this.mesh.joints.length;i++){
			if(!jointCache[i]) jointCache[i]={modelMatrix:null,invBind:null};
			if(typeof this.mesh.joints[i]=="string"){
				if(this.bones){
					var modelMatrix=this.bones[this.mesh.joints[i]].getModelMatrix();
				}
			}else{
				var modelMatrix=this.mesh.joints[i].getModelMatrix();
			}
			var invBind=this.mesh.invBind[i];
			if(jointCache[i].modelMatrix!=modelMatrix || jointCache[i].invBind!=invBind){
				try{
						var jointmat=GLGE.mulMat4(modelMatrix,invBind);
						if(!program.glarrays.joints[i]) program.glarrays.joints[i]=new Float32Array(jointmat);
							else GLGE.mat4gl(jointmat,program.glarrays.joints[i]);		
						if(!program.glarrays.jointsinv[i]) program.glarrays.jointsinv[i]=new Float32Array(GLGE.inverseMat4(jointmat));
							else GLGE.mat4gl(GLGE.inverseMat4(jointmat),program.glarrays.jointsinv[i]);						
						gl.uniformMatrix4fv(GLGE.getUniformLocation(gl,program, "jointMat["+i+"]"), true,program.glarrays.joints[i]);
						gl.uniformMatrix4fv(GLGE.getUniformLocation(gl,program, "jointNMat["+i+"]"), false,program.glarrays.jointsinv[i]);
						jointCache[i].modelMatrix=modelMatrix;
						jointCache[i].invBind=invBind;
				}catch(e){}
			}
		}
	}

    
	if(this.material && renderType==GLGE.RENDER_DEFAULT) this.material.textureUniforms(gl,program,lights,this);
}
/**
* Renders the object to the screen
* @private
*/
GLGE.Object.prototype.GLRender=function(gl,renderType,pickindex){
	if(!this.gl) this.GLInit(gl);
	
	//if look at is set then look
	if(this.lookAt) this.Lookat(this.lookAt);
 
	//animate this object
	if(renderType==GLGE.RENDER_DEFAULT){
		if(this.animation) this.animate();
	}
	this.caches={};
	for(var n=0;n<this.instances.length;n++){
		this.instances[n].caches={};
	}
	
	//get pixel size of object
	//TODO: only do this when it's needed!!!!
	var camerapos=gl.scene.camera.getPosition();
	var modelpos=this.getPosition();
	var dist=GLGE.lengthVec3([camerapos.x-modelpos.x,camerapos.y-modelpos.y,camerapos.z-modelpos.z]);
	dist=GLGE.mulMat4Vec4(gl.scene.camera.getProjectionMatrix(),[this.getBoundingVolume().getSphereRadius(),0,-dist,1]);
	var pixelsize=dist[0]/dist[3]*gl.scene.renderer.canvas.width;

	for(var i=0; i<this.multimaterials.length;i++){
		var lod=this.multimaterials[i].getLOD(pixelsize);

		if(lod.mesh && lod.mesh.loaded){
			if(renderType==GLGE.RENDER_NULL){
				if(lod.material) lod.material.registerPasses(gl,this);
				break;
			}
			if(!this.multimaterials[i].GLShaderProgram){
				this.createShaders(lod);
			}else{
				this.GLShaderProgramPick=lod.GLShaderProgramPick;
				this.GLShaderProgramShadow=lod.GLShaderProgramShadow;
				this.GLShaderProgram=lod.GLShaderProgram;
			}
			this.mesh=lod.mesh;
			this.material=lod.material;
			
			var drawType;
			switch(this.drawType){
				case GLGE.DRAW_LINES:
					drawType=gl.LINES;
					break;
				case GLGE.DRAW_POINTS:
					drawType=gl.POINTS;
					break;
				case GLGE.DRAW_LINELOOPS:
					drawType=gl.LINE_LOOP;
					break;
				case GLGE.DRAW_LINESTRIPS:
					drawType=gl.LINE_STRIP;
					break;
				default:
					drawType=gl.TRIANGLES;
					break;
			}
 
			switch(renderType){
				case  GLGE.RENDER_DEFAULT:
					gl.useProgram(this.GLShaderProgram);
					this.mesh.GLAttributes(gl,this.GLShaderProgram);
					break;
				case  GLGE.RENDER_SHADOW:
					gl.useProgram(this.GLShaderProgramShadow);
					this.mesh.GLAttributes(gl,this.GLShaderProgramShadow);
					break;
				case  GLGE.RENDER_NORMAL:
					gl.useProgram(this.GLShaderProgramNormal);
					this.mesh.GLAttributes(gl,this.GLShaderProgramNormal);
					break;
				case  GLGE.RENDER_PICK:
					gl.useProgram(this.GLShaderProgramPick);
					this.mesh.GLAttributes(gl,this.GLShaderProgramPick);
					drawType=gl.TRIANGLES;
					break;
			}
			//render the object
			this.GLUniforms(gl,renderType,pickindex);
			if(this.mesh.GLfaces){
				gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.mesh.GLfaces);
				gl.drawElements(drawType, this.mesh.GLfaces.numItems, gl.UNSIGNED_SHORT, 0);
			}else{
				gl.drawArrays(drawType, 0, this.mesh.positions.length/3);
			}
				
			var matrix=this.matrix;
			var caches=this.caches;
			for(var n=0;n<this.instances.length;n++){
				this.matrix=this.instances[n].getModelMatrix();
				this.caches=this.instances[n].caches;
				this.GLUniforms(gl,renderType,pickindex);
				if(this.mesh.GLfaces){
					gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.mesh.GLfaces);
					gl.drawElements(drawType, this.mesh.GLfaces.numItems, gl.UNSIGNED_SHORT, 0);
				}else{
					gl.drawArrays(drawType, 0, this.mesh.positions.length/3);
				}
			}

			this.matrix=matrix;
			this.caches=caches;
		}
	}
}



/**
* @class Creates a new mesh
* @see GLGE.Object
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
* @augments GLGE.Events
*/
GLGE.Mesh=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.GLbuffers=[];
	this.buffers=[];
	this.UV=[];
	this.boneWeights=[];
	this.setBuffers=[];
	this.faces={};
}
GLGE.augment(GLGE.QuickNotation,GLGE.Mesh);
GLGE.augment(GLGE.JSONLoader,GLGE.Mesh);
GLGE.augment(GLGE.Events,GLGE.Mesh);
GLGE.Mesh.prototype.gl=null;
GLGE.Mesh.prototype.className="Mesh";
GLGE.Mesh.prototype.GLbuffers=null;
GLGE.Mesh.prototype.buffers=null;
GLGE.Mesh.prototype.setBuffers=null;
GLGE.Mesh.prototype.GLfaces=null;
GLGE.Mesh.prototype.faces=null;
GLGE.Mesh.prototype.UV=null;
GLGE.Mesh.prototype.joints=null;
GLGE.Mesh.prototype.invBind=null;
GLGE.Mesh.prototype.loaded=false;
/**
 * @name GLGE.Mesh#shaderupdate
 * @event fired when the shader needs updating
 * @param {object} data
 */

/**
* Gets the bounding volume for the mesh
* @returns {GLGE.BoundingVolume} 
*/
GLGE.Mesh.prototype.getBoundingVolume=function(){
	if(!this.boundingVolume){
		var minX,maxX,minY,maxY,minZ,maxZ;
		for(var i=0;i<this.buffers.length;i++){
			if(this.buffers[i].name=="position") var positions=this.buffers[i].data;
		}
		for(var i=0;i<positions.length;i=i+3){
			if(i==0){
				minX=maxX=positions[i];
				minY=maxY=positions[i+1];
				minZ=maxZ=positions[i+2];
			}else{
				minX=Math.min(minX,positions[i]);
				maxX=Math.max(maxX,positions[i]);
				minY=Math.min(minY,positions[i+1]);
				maxY=Math.max(maxY,positions[i+1]);
				minZ=Math.min(minZ,positions[i+2]);
				maxZ=Math.max(maxZ,positions[i+2]);
			}
		}
		this.boundingVolume=new GLGE.BoundingVolume(minX,maxX,minY,maxY,minZ,maxZ);
	}
	return this.boundingVolume;
}
/**
* Sets the joints
* @param {string[]} jsArray set joint objects
*/
GLGE.Mesh.prototype.setJoints=function(jsArray){
	this.joints=jsArray;
	this.fireEvent("shaderupdate",{});
	return this;
}
/**
* Sets the inverse bind matrix for each joint
* @param {GLGE.Matrix[]} jsArray set joint names
*/
GLGE.Mesh.prototype.setInvBindMatrix=function(jsArray){
	this.invBind=jsArray;
	this.fireEvent("shaderupdate",{});
	return this;
}
/**
* Sets the joint channels for each vertex 
* @param {Number[]} jsArray The 1 dimentional array of bones
* @param {Number} num the number of chanels in this mesh
*/
GLGE.Mesh.prototype.setVertexJoints=function(jsArray,num){
	if(num<4){
		this.setBuffer("joints1",jsArray,num);
	}else{
		var jsArray1=[];
		var jsArray2=[];
		for(var i=0;i<jsArray.length;i++){
			if(i%num<4){
				jsArray1.push(jsArray[i]);
			}else{
				jsArray2.push(jsArray[i]);
			}
		}
		this.setBuffer("joints1",jsArray1,4);
		this.setBuffer("joints2",jsArray2,num%4);
	}
	this.fireEvent("shaderupdate",{});
	return this;
}
/**
* Sets the joint weights on each vertex
* @param {Number[]} jsArray The 1 dimentional array of weights
* @param {Number} num the number of chanels in this mesh
*/
GLGE.Mesh.prototype.setVertexWeights=function(jsArray,num){
	//normalize the weights!
	for(var i=0;i<jsArray.length;i=i+parseInt(num)){
		var total=0;
		for(var n=0;n<num;n++){
			total+=parseFloat(jsArray[i+n]);
		}
		for(var n=0;n<num;n++){
			jsArray[i+n]=jsArray[i+n]/total;
		}
	}

	if(num<4){
		this.setBuffer("weights1",jsArray,num);
	}else{
		var jsArray1=[];
		var jsArray2=[];
		for(var i=0;i<jsArray.length;i++){
			if(i%num<4){
				jsArray1.push(jsArray[i]);
			}else{
				jsArray2.push(jsArray[i]);
			}
		}
		this.setBuffer("weights1",jsArray1,4);
		this.setBuffer("weights2",jsArray2,num%4);
	}
	this.fireEvent("shaderupdate",{});
	return this;
}
/**
* Set the UV coord for the first UV layer
* @param {Number[]} jsArray the UV coords in a 1 dimentional array
*/
GLGE.Mesh.prototype.setUV=function(jsArray){
	var idx=0;
	for(var i=0; i<jsArray.length;i=i+2){
		this.UV[idx]=jsArray[i];
		this.UV[idx+1]=jsArray[i+1];
		if(!this.UV[idx+2]) this.UV[idx+2]=0;
		if(!this.UV[idx+3]) this.UV[idx+3]=0;
		idx=idx+4;
	}
	this.setBuffer("UV",this.UV,4);
	return this;
}
/**
* Set the UV coord for the second UV layer
* @param {Number[]} jsArray the UV coords in a 1 dimentional array
*/
GLGE.Mesh.prototype.setUV2=function(jsArray){
	var idx=0;
	for(var i=0; i<jsArray.length;i=i+2){
		if(!this.UV[idx]) this.UV[idx]=0;
		if(!this.UV[idx+1]) this.UV[idx+1]=0;
		this.UV[idx+2]=jsArray[i];
		this.UV[idx+3]=jsArray[i+1];
		idx=idx+4;
	}
	this.setBuffer("UV",this.UV,4);
	return this;
}
/**
* Sets the positions of the verticies
* @param {Number[]} jsArray The 1 dimentional array of positions
*/
GLGE.Mesh.prototype.setPositions=function(jsArray){
	this.loaded=true;
	this.positions=jsArray;
	this.setBuffer("position",jsArray,3);
	return this;
}
/**
* Sets the normals of the verticies
* @param {Number[]} jsArray The 1 dimentional array of normals
*/
GLGE.Mesh.prototype.setNormals=function(jsArray){
	this.normals=jsArray;
	this.setBuffer("normal",jsArray,3);
	return this;
}
/**
* Sets a buffer for the
* @param {String} boneName The name of the bone
* @param {Number[]} jsArray The 1 dimentional array of weights
* @private
*/
GLGE.Mesh.prototype.setBuffer=function(bufferName,jsArray,size){
	//make sure all jsarray items are floats
	for(var i=0;i<jsArray.length;i++) jsArray[i]=parseFloat(jsArray[i]);
	var buffer;
	for(var i=0;i<this.buffers.length;i++){
		if(this.buffers[i].name==bufferName) buffer=i;
	}
	if(!buffer){
		this.buffers.push({name:bufferName,data:jsArray,size:size,GL:false});
	}
        else 
	{
		this.buffers[buffer]={name:bufferName,data:jsArray,size:size,GL:false};
	}
	return this;
}
/**
* Sets the faces for this mesh
* @param {Number[]} jsArray The 1 dimentional array of normals
*/
GLGE.Mesh.prototype.setFaces=function(jsArray){
	this.faces={data:jsArray,GL:false};	
	//if at this point calculate normals if we haven't got them yet
	if(!this.normals) this.calcNormals();
	
	//add a tangent buffer
	for(var i=0;i<this.buffers.length;i++){
		if(this.buffers[i].name=="position") var position=this.buffers[i].data;
		if(this.buffers[i].name=="UV") var uv=this.buffers[i].data;
		if(this.buffers[i].name=="normal") var normal=this.buffers[i].data;
	}
	

	if(position && uv){
		var tangentArray=[];
		var data={};
		var ref;
		for(var i=0;i<this.faces.data.length;i=i+3){
			var p1=[position[(parseInt(this.faces.data[i]))*3],position[(parseInt(this.faces.data[i]))*3+1],position[(parseInt(this.faces.data[i]))*3+2]];
			var p2=[position[(parseInt(this.faces.data[i+1]))*3],position[(parseInt(this.faces.data[i+1]))*3+1],position[(parseInt(this.faces.data[i+1]))*3+2]];
			var p3=[position[(parseInt(this.faces.data[i+2]))*3],position[(parseInt(this.faces.data[i+2]))*3+1],position[(parseInt(this.faces.data[i+2]))*3+2]];
			
			var n1=[normal[(parseInt(this.faces.data[i]))*3],normal[(parseInt(this.faces.data[i]))*3+1],normal[(parseInt(this.faces.data[i]))*3+2]];
			var n2=[normal[(parseInt(this.faces.data[i+1]))*3],normal[(parseInt(this.faces.data[i+1]))*3+1],normal[(parseInt(this.faces.data[i+1]))*3+2]];
			var n3=[normal[(parseInt(this.faces.data[i+2]))*3],normal[(parseInt(this.faces.data[i+2]))*3+1],normal[(parseInt(this.faces.data[i+2]))*3+2]];
			
			var p21=[p2[0]-p1[0],p2[1]-p1[1],p2[2]-p1[2]];
			var p31=[p3[0]-p1[0],p3[1]-p1[1],p3[2]-p1[2]];
			var uv21=[uv[(parseInt(this.faces.data[i+1]))*4]-uv[(parseInt(this.faces.data[i]))*4],uv[(parseInt(this.faces.data[i+1]))*4+1]-uv[(parseInt(this.faces.data[i]))*4+1]];
			var uv31=[uv[(parseInt(this.faces.data[i+2]))*4]-uv[(parseInt(this.faces.data[i]))*4],uv[(parseInt(this.faces.data[i+2]))*4+1]-uv[(parseInt(this.faces.data[i]))*4+1]];
			

   
			var tangent=GLGE.toUnitVec3([p21[0]*uv31[1]-p31[0]*uv21[01],
								p21[1]*uv31[1]-p31[1]*uv21[1],
								p21[2]*uv31[1]-p31[2]*uv21[1]]);		
								
			var cp = uv21[1] * uv31[0] - uv21[0] * uv31[1];
			if ( cp != 0.0 ) tangent=GLGE.toUnitVec3(GLGE.scaleVec3(tangent,1/cp));

			if(data[[p1[0],p1[1],p1[2],n1[0],n1[1],n1[2]].join(",")]){
				tang=data[[p1[0],p1[1],p1[2],n1[0],n1[1],n1[2]].join(",")];
				tang.vec=GLGE.scaleVec3(GLGE.addVec3(GLGE.scaleVec3(tang.vec,tang.weight),tangent),1/(tang.weight));
				tang.weight++;
			}else{
				data[[p1[0],p1[1],p1[2],n1[0],n1[1],n1[2]].join(",")]={vec:tangent,weight:1};
			}
			if(data[[p2[0],p2[1],p2[2],n2[0],n2[1],n2[2]].join(",")]){
				tang=data[[p2[0],p2[1],p2[2],n2[0],n2[1],n2[2]].join(",")];
				tang.vec=GLGE.scaleVec3(GLGE.addVec3(GLGE.scaleVec3(tang.vec,tang.weight),tangent),1/(tang.weight+1));
				tang.weight++;
			}else{
				data[[p2[0],p2[1],p2[2],n2[0],n2[1],n2[2]].join(",")]={vec:tangent,weight:1};
			}
			if(data[[p3[0],p3[1],p3[2],n3[0],n3[1],n3[2]].join(",")]){
				tang=data[[p3[0],p3[1],p3[2],n3[0],n3[1],n3[2]].join(",")];
				tang.vec=GLGE.scaleVec3(GLGE.addVec3(GLGE.scaleVec3(tang.vec,tang.weight),tangent),1/(tang.weight+1));
				tang.weight++;
			}else{
				data[[p3[0],p3[1],p3[2],n3[0],n3[1],n3[2]].join(",")]={vec:tangent,weight:1};
			}
		}
		for(var i=0;i<position.length/3;i++){
			var p1=[position[i*3],position[i*3+1],position[i*3+2]];
			var n1=[normal[i*3],normal[i*3+1],normal[i*3+2]];
			t=data[[p1[0],p1[1],p1[2],n1[0],n1[1],n1[2]].join(",")].vec;
			if(t){
				tangentArray[i*3]=t[0];
				tangentArray[i*3+1]=t[1];
				tangentArray[i*3+2]=t[2];
			}
		}
		this.setBuffer("tangent",tangentArray,3);
	}
	return this;
}
/**
* Sets the faces for this mesh
* @param {Number[]} jsArray The 1 dimentional array of normals
* @private
*/
GLGE.Mesh.prototype.GLSetFaceBuffer=function(gl){
	if(!this.GLfaces) this.GLfaces = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.GLfaces);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(this.faces.data), gl.STATIC_DRAW);
	this.GLfaces.itemSize = 1;
	this.GLfaces.numItems = this.faces.data.length;
}
/**
* Sets up a GL Buffer
* @param {WebGLContext} gl The context being drawn on
* @param {String} bufferName The name of the buffer to create
* @param {Number[]}  jsArray The data to add to the buffer
* @param {Number}  size Size of a single element within the array
* @private
*/
GLGE.Mesh.prototype.GLSetBuffer=function(gl,bufferName,jsArray,size){
	if(!this.GLbuffers[bufferName]) this.GLbuffers[bufferName] = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, this.GLbuffers[bufferName]);
	gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(jsArray), gl.STATIC_DRAW);
	this.GLbuffers[bufferName].itemSize = size;
	this.GLbuffers[bufferName].numItems = jsArray.length/size;
};
/**
* Calculates the normals for this mesh
* @private
*/
GLGE.Mesh.prototype.calcNormals=function(){
	var normals=[];
	var positions=this.positions;
	var faces=this.faces.data;
	if(!faces){
		faces=[];
		for(var i=0;i<positions.length/3;i++) faces[i]=i;
	}
	for(var i=0;i<faces.length;i=i+3){
		var v1=[positions[faces[i]*3],positions[faces[i]*3+1],positions[faces[i]*3+2]];
		var v2=[positions[faces[i+1]*3],positions[faces[i+1]*3+1],positions[faces[i+1]*3+2]];
		var v3=[positions[faces[i+2]*3],positions[faces[i+2]*3+1],positions[faces[i+2]*3+2]];
		var vec1=GLGE.subVec3(v2,v1);
		var vec2=GLGE.subVec3(v3,v1);
		var norm=GLGE.toUnitVec3(GLGE.crossVec3(vec1,vec2));
		if(normals[faces[i]]==undefined) normals[faces[i]]=[];
		normals[faces[i]].push(norm);
		if(normals[faces[i+1]]==undefined) normals[faces[i+1]]=[];
		normals[faces[i+1]].push(norm);
		if(normals[faces[i+2]]==undefined) normals[faces[i+2]]=[];
		normals[faces[i+2]].push(norm);
	}
	var norms=[];
	for(i=0;i<normals.length;i++){
		var x=0,y=0,z=0;
		if(normals[i]!=undefined){
			for(var j=0;j<normals[i].length;j++){
				x+=normals[i][j][0];
				y+=normals[i][j][1];
				z+=normals[i][j][2];
			}
			x/=normals[i].length;
			y/=normals[i].length;
			z/=normals[i].length;
			norms[i*3]=x;
			norms[i*3+1]=y;
			norms[i*3+2]=z;
		}
	}
	this.setNormals(norms);
}
/**
* Sets the Attributes for this mesh
* @param {WebGLContext} gl The context being drawn on
* @private
*/
GLGE.Mesh.prototype.GLAttributes=function(gl,shaderProgram){
	//if at this point we have no normals set then calculate them
	if(!this.normals) this.calcNormals();

	//disable all the attribute initially arrays - do I really need this?
	for(var i=0; i<8; i++) gl.disableVertexAttribArray(i);
	//check if the faces have been updated
	if(!this.faces.GL && this.faces.data && this.faces.data.length>0){
		this.GLSetFaceBuffer(gl);
		this.faces.GL=true;
	}
	//loop though the buffers
	for(i=0; i<this.buffers.length;i++){
		if(!this.buffers[i].GL){
			this.GLSetBuffer(gl,this.buffers[i].name,this.buffers[i].data,this.buffers[i].size);
			this.buffers[i].GL=true;
		}
		attribslot=GLGE.getAttribLocation(gl,shaderProgram, this.buffers[i].name);
		if(attribslot>-1){
			gl.bindBuffer(gl.ARRAY_BUFFER, this.GLbuffers[this.buffers[i].name]);
			gl.enableVertexAttribArray(attribslot);
			gl.vertexAttribPointer(attribslot, this.GLbuffers[this.buffers[i].name].itemSize, gl.FLOAT, false, 0, 0);
		}
	}
}



/**
* @class Creates a new light source to be added to a scene
* @property {Boolean} diffuse Dose this light source effect diffuse shading
* @property {Boolean} specular Dose this light source effect specular shading
* @augments GLGE.Animatable
* @augments GLGE.Placeable
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.Light=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.color={r:1,g:1,b:1};
}
GLGE.augment(GLGE.Placeable,GLGE.Light);
GLGE.augment(GLGE.Animatable,GLGE.Light);
GLGE.augment(GLGE.QuickNotation,GLGE.Light);
GLGE.augment(GLGE.JSONLoader,GLGE.Light);
GLGE.Light.prototype.className="Light";
/**
* @constant 
* @description Enumeration for an point light source
*/
GLGE.L_POINT=1;
/**
* @constant 
* @description Enumeration for an directional light source
*/
GLGE.L_DIR=2;
/**
* @constant 
* @description Enumeration for an spot light source
*/
GLGE.L_SPOT=3;

GLGE.Light.prototype.constantAttenuation=1;
GLGE.Light.prototype.linearAttenuation=0.002;
GLGE.Light.prototype.quadraticAttenuation=0.0008;
GLGE.Light.prototype.spotCosCutOff=0.95;
GLGE.Light.prototype.spotPMatrix=null;
GLGE.Light.prototype.spotExponent=10;
GLGE.Light.prototype.color=null; 
GLGE.Light.prototype.diffuse=true; 
GLGE.Light.prototype.specular=true; 
GLGE.Light.prototype.samples=0; 
GLGE.Light.prototype.softness=0.01; 
GLGE.Light.prototype.type=GLGE.L_POINT;
GLGE.Light.prototype.frameBuffer=null;
GLGE.Light.prototype.renderBuffer=null;
GLGE.Light.prototype.texture=null;
GLGE.Light.prototype.bufferHeight=256;
GLGE.Light.prototype.bufferWidth=256;
GLGE.Light.prototype.shadowBias=2.0;
GLGE.Light.prototype.castShadows=false;
/**
* Gets the spot lights projection matrix
* @returns the lights spot projection matrix
* @private
*/
GLGE.Light.prototype.getPMatrix=function(){
	if(!this.spotPMatrix){
		var far;
		if(this.scene && this.scene.camera) far=this.scene.camera.far;
			else far=1000;
		this.spotPMatrix=GLGE.makePerspective(Math.acos(this.spotCosCutOff)/3.14159*360, 1.0, 0.1, far);
	}
	return this.spotPMatrix;
}
/**
* Sets the shadow casting flag
* @param {number} value should cast shadows?
*/
GLGE.Light.prototype.setCastShadows=function(value){
	this.castShadows=value;
	return this;
}
/**
* Gets the shadow casting flag
* @returns {number} true if casts shadows
*/
GLGE.Light.prototype.getCastShadows=function(){
	return this.castShadows;
	return this;
}
/**
* Sets the shadow bias
* @param {number} value The shadow bias
*/
GLGE.Light.prototype.setShadowBias=function(value){
	this.shadowBias=value;
	return this;
}
/**
* Gets the shadow bias
* @returns {number} The shadow buffer bias
*/
GLGE.Light.prototype.getShadowBias=function(){
	return this.shadowBias;
}

/**
* Sets the number of samples for this shadow
* @param {number} value The number of samples to perform
*/
GLGE.Light.prototype.setShadowSamples=function(value){
	this.samples=value;
	return this;
}
/**
* Gets the number of samples for this shadow
* @returns {number} The number of samples
*/
GLGE.Light.prototype.getShadowSamples=function(){
	return this.samples;
}
/**
* Sets the shadow softness
* @param {number} value The number of samples to perform
*/
GLGE.Light.prototype.setShadowSoftness=function(value){
	this.softness=value;
	return this;
}
/**
* Gets the shadow softness
* @returns {number} The softness of the shadows
*/
GLGE.Light.prototype.getShadowSamples=function(){
	return this.softness;
}
/**
* Sets the shadow buffer width
* @param {number} value The shadow buffer width
*/
GLGE.Light.prototype.setBufferWidth=function(value){
	this.bufferWidth=value;
	return this;
}
/**
* Gets the shadow buffer width
* @returns {number} The shadow buffer width
*/
GLGE.Light.prototype.getBufferHeight=function(){
	return this.bufferHeight;
}
/**
* Sets the shadow buffer width
* @param {number} value The shadow buffer width
*/
GLGE.Light.prototype.setBufferHeight=function(value){
	this.bufferHeight=value;
	return this;
}
/**
* Gets the shadow buffer width
* @returns {number} The shadow buffer width
*/
GLGE.Light.prototype.getBufferWidth=function(){
	return this.bufferWidth;
}
/**
* Sets the spot light cut off
* @param {number} value The cos of the angle to limit
*/
GLGE.Light.prototype.setSpotCosCutOff=function(value){
	this.spotPMatrix=null;
	this.spotCosCutOff=value;
	return this;
}
/**
* Gets the spot light cut off
* @returns {number} The cos of the limiting angle 
*/
GLGE.Light.prototype.getSpotCosCutOff=function(){
	return this.spotCosCutOff;
}
/**
* Sets the spot light exponent
* @param {number} value The spot lights exponent
*/
GLGE.Light.prototype.setSpotExponent=function(value){
	this.spotExponent=value;
	return this;
}
/**
* Gets the spot light exponent
* @returns {number} The exponent of the spot light
*/
GLGE.Light.prototype.getSpotExponent=function(){
	return this.spotExponent;
}
/**
* Sets the light sources Attenuation
* @returns {Object} The components of the light sources attenuation
*/
GLGE.Light.prototype.getAttenuation=function(constant,linear,quadratic){
	var attenuation={};
	attenuation.constant=this.constantAttenuation;
	attenuation.linear=this.linearAttenuation;
	attenuation.quadratic=this.quadraticAttenuation;
	return attenuation;
}
/**
* Sets the light sources Attenuation
* @param {Number} constant The constant part of the attenuation
* @param {Number} linear The linear part of the attenuation
* @param {Number} quadratic The quadratic part of the attenuation
*/
GLGE.Light.prototype.setAttenuation=function(constant,linear,quadratic){
	this.constantAttenuation=constant;
	this.linearAttenuation=linear;
	this.quadraticAttenuation=quadratic;
	return this;
}
/**
* Sets the light sources constant attenuation
* @param {Number} value The constant part of the attenuation
*/
GLGE.Light.prototype.setAttenuationConstant=function(value){
	this.constantAttenuation=value;
	return this;
}
/**
* Sets the light sources linear attenuation
* @param {Number} value The linear part of the attenuation
*/
GLGE.Light.prototype.setAttenuationLinear=function(value){
	this.linearAttenuation=value;
	return this;
}
/**
* Sets the light sources quadratic attenuation
* @param {Number} value The quadratic part of the attenuation
*/
GLGE.Light.prototype.setAttenuationQuadratic=function(value){
	this.quadraticAttenuation=value;
	return this;
}

/**
* Sets the color of the light source
* @param {string} color The color of the light
*/
GLGE.Light.prototype.setColor=function(color){
	color=GLGE.colorParse(color);
	this.color={r:color.r,g:color.g,b:color.b};
	return this;
}
/**
* Sets the red color of the light source
* @param {Number} value The new red level 0-1
*/
GLGE.Light.prototype.setColorR=function(value){
	this.color.r=value;
	return this;
}
/**
* Sets the green color of the light source
* @param {Number} value The new green level 0-1
*/
GLGE.Light.prototype.setColorG=function(value){
	this.color.g=value;
	return this;
}
/**
* Sets the blue color of the light source
* @param {Number} value The new blue level 0-1
*/
GLGE.Light.prototype.setColorB=function(value){
	this.color.b=value;
	return this;
}
/**
* Gets the current color of the light source
* @return {[r,g,b]} The current position
*/
GLGE.Light.prototype.getColor=function(){
	return this.color;
}
/**
* Gets the type of the light
* @return {Number} The type of the light source eg GLGE.L_POINT
*/
GLGE.Light.prototype.getType=function(){
	return this.type;
}
/**
* Sets the type of the light
* @param {Number} type The type of the light source eg GLGE.L_POINT
*/
GLGE.Light.prototype.setType=function(type){
	this.type=type;
	return this;
}
/**
* init for the rendering
* @private
*/
GLGE.Light.prototype.GLInit=function(gl){	
	this.gl=gl;
	if(this.type==GLGE.L_SPOT && !this.texture){
		this.createSpotBuffer(gl);
	}
}
/**
* Sets up the WebGL needed to render the depth map for this light source. Only used for spot lights which produce shadows
* @private
*/
GLGE.Light.prototype.createSpotBuffer=function(gl){
    this.frameBuffer = gl.createFramebuffer();
    this.renderBuffer = gl.createRenderbuffer();
    this.texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, this.texture);

    try {
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.bufferWidth, this.bufferHeight, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    } catch (e) {
        var tex = new Uint8Array(this.bufferWidth * this.bufferHeight * 4);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, this.bufferWidth, this.bufferHeight, 0, gl.RGBA, gl.UNSIGNED_BYTE, tex);
    }
    
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.frameBuffer);
    gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderBuffer);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, this.bufferWidth, this.bufferHeight);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    
    gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.texture, 0);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, this.renderBuffer);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
}

/**
* @constant 
* @description Enumeration for a perspective camera
*/
GLGE.C_PERSPECTIVE=1;
/**
* @constant 
* @description Enumeration for a orthographic camera
*/
GLGE.C_ORTHO=2;

/**
* @class Creates a new camera object
* @augments GLGE.Animatable
* @augments GLGE.Placeable
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.Camera=function(uid){
	GLGE.Assets.registerAsset(this,uid);
};
GLGE.augment(GLGE.Placeable,GLGE.Camera);
GLGE.augment(GLGE.Animatable,GLGE.Camera);
GLGE.augment(GLGE.QuickNotation,GLGE.Camera);
GLGE.augment(GLGE.JSONLoader,GLGE.Camera);
GLGE.Camera.prototype.className="Camera";
GLGE.Camera.prototype.fovy=35;
GLGE.Camera.prototype.aspect=1.0;
GLGE.Camera.prototype.near=0.1;
GLGE.Camera.prototype.far=1000.0;
GLGE.Camera.prototype.orthoscale=5;
GLGE.Camera.prototype.type=GLGE.C_PERSPECTIVE;
GLGE.Camera.prototype.pMatrix=null;

/**
* Method gets the orthographic scale for the camers
* @return {Matrix} Returns the orthographic scale
*/
GLGE.Camera.prototype.getOrthoScale=function(){
	if(this.type==GLGE.C_ORTHO) {
		return this.orthoscale
	}else{
		GLGE.error("You may only get a scale for a orthographic camera");
		return 1;
	}
};
/**
* Method sets the orthographic scale for the camers
* @param {number} scale The new orthographic scale
*/
GLGE.Camera.prototype.setOrthoScale=function(scale){
	if(this.type==GLGE.C_ORTHO) {
		this.orthoscale=scale;
		this.pMatrix=null;
	}
	else
	{
		GLGE.error("You may only set a scale for a orthographic camera");
	}
	return this;
};

/**
* Method gets the far drawing distance
* @return {Matrix} Returns the cameras far draw distance
*/
GLGE.Camera.prototype.getFar=function(){
	return this.far;
};
/**
* Method sets the far draw distance of the camera
* @param {number} distance The far draw distance
*/
GLGE.Camera.prototype.setFar=function(distance){
	this.far=distance;
	return this;
};

/**
* Method gets the near drawing distance
* @return {Matrix} Returns the cameras near draw distance
*/
GLGE.Camera.prototype.getNear=function(){
	return this.near;
};
/**
* Method sets the near draw distance of the camera
* @param {number} distance The near draw distance
*/
GLGE.Camera.prototype.setNear=function(distance){
	this.near=distance;
	return this;
};

/**
* Method gets the current camera type
* @return {Matrix} Returns the camera type
*/
GLGE.Camera.prototype.getType=function(){
	return this.type
};
/**
* Method sets the type of camera GLGE.C_PERSPECTIVE or GLGE.C_ORTHO
* @param {number} type The type of this camera
*/
GLGE.Camera.prototype.setType=function(type){
	if(type==GLGE.C_PERSPECTIVE || type==GLGE.C_ORTHO){
		this.type=type;
		this.pMatrix=null;
	}else{
		GLGE.error("unsuported camera type");
	}
	return this;
};

/**
* Method gets the current yfov if the camera type is GLGE.C_PERSPECTIVE
* @return {Matrix} Returns the yfov
*/
GLGE.Camera.prototype.getFovY=function(){
	if(this.type==GLGE.C_PERSPECTIVE) {
		return this.fovy
	}else{
		GLGE.error("You may only get a yfov for a perspective camera");
		return 1;
	}
};
/**
* Method sets the yfov of the camera
* @param {number} yfov The new yfov of the camera
*/
GLGE.Camera.prototype.setFovY=function(fovy){
	if(this.type==GLGE.C_PERSPECTIVE) {
		this.fovy=fovy;
		this.ymax=null;
		this.pMatrix=null;
	}
	else
	{
		GLGE.error("You may only set a yfov for a perspective camera");
	}
	return this;
};

/**
* Method gets the current aspect if the camera type is GLGE.C_PERSPECTIVE
* @return {Matrix} Returns the yfov
*/
GLGE.Camera.prototype.getAspect=function(){
	if(this.type==GLGE.C_PERSPECTIVE || this.type==GLGE.C_ORTHO) {
		return this.aspect
	}
	else
	{
		GLGE.error("You may only set a aspect for a perspective or orthographic camera");
		return 1;
	}
};
/**
* Method sets the aspect of the camera
* @param {number} aspect The new projection matrix
*/
GLGE.Camera.prototype.setAspect=function(aspect){
	if(this.type==GLGE.C_PERSPECTIVE || this.type==GLGE.C_ORTHO) {
		this.aspect=aspect;
		this.pMatrix=null;
	}
	else
	{
		GLGE.error("You may only set a aspect for a perspective or orthographic camera");
	}
	return this;
};


/**
* Method gets the current projection matrix of this camera
* @return {Matrix} Returns the camera projection matrix
*/
GLGE.Camera.prototype.getProjectionMatrix=function(){
	if(!this.pMatrix){
		switch(this.type){
			case GLGE.C_PERSPECTIVE:
				this.pMatrix=GLGE.makePerspective(this.fovy, this.aspect, this.near, this.far);
				break;
			case GLGE.C_ORTHO:
				this.pMatrix=GLGE.makeOrtho(-this.orthoscale*this.aspect,this.orthoscale*this.aspect,-this.orthoscale,this.orthoscale, this.near, this.far);
				break;
		}
	}
	return this.pMatrix;
};
/**
* Method generates the projection matrix based on the 
* camera paramaters
* @param {Matrix} projection The new projection matrix
*/
GLGE.Camera.prototype.setProjectionMatrix=function(projection){
	this.pMatrix=projection;
	return this;
};
/**
* Method generates the cameras view matrix
* @return Returns the view matrix based on this camera
* @type Matrix
*/
GLGE.Camera.prototype.updateMatrix=function(){
	var position=this.getPosition();
	var vMatrix=GLGE.translateMatrix(position.x,position.y,position.z);
	vMatrix=GLGE.mulMat4(vMatrix,this.getRotMatrix());
	if(this.parent) vMatrix=GLGE.mulMat4(this.parent.getModelMatrix(),vMatrix);
	this.matrix=GLGE.inverseMat4(vMatrix);
};
/**
* Method generates the cameras view matrix
* @return Returns the view matrix based on this camera
* @type Matrix
*/
GLGE.Camera.prototype.getViewMatrix=function(){
	if(!this.matrix || !this.rotmatrix) this.updateMatrix();
	return this.matrix;
};



/**
* @constant 
* @description Enumeration for no fog
*/
GLGE.FOG_NONE=1;
/**
* @constant 
* @description Enumeration for linear fall off fog
*/
GLGE.FOG_LINEAR=2;
/**
* @constant 
* @description Enumeration for exponential fall off fog
*/
GLGE.FOG_QUADRATIC=3;

/**
* @class Scene class containing the camera, lights and objects
* @augments GLGE.Group
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.Scene=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.children=[];
	this.camera=new GLGE.Camera();
	this.backgroundColor={r:1,g:1,b:1,a:1};
	this.ambientColor={r:0,g:0,b:0};
	this.fogColor={r:0.5,g:0.5,b:0.5};
	this.passes=[];
}
GLGE.augment(GLGE.Group,GLGE.Scene);
GLGE.augment(GLGE.QuickNotation,GLGE.Scene);
GLGE.augment(GLGE.JSONLoader,GLGE.Scene);
GLGE.Scene.prototype.camera=null;
GLGE.Scene.prototype.className="Scene";
GLGE.Scene.prototype.renderer=null;
GLGE.Scene.prototype.backgroundColor=null;
GLGE.Scene.prototype.filter=null;
GLGE.Scene.prototype.fogColor=null;
GLGE.Scene.prototype.ambientColor=null;
GLGE.Scene.prototype.fogNear=10;
GLGE.Scene.prototype.fogFar=80;
GLGE.Scene.prototype.fogType=GLGE.FOG_NONE;
GLGE.Scene.prototype.passes=null;

/**
* Gets the fog falloff type
* @returns {number} the far falloff type
*/
GLGE.Scene.prototype.getFogType=function(){	
	return this.fogType;
}
/**
* Sets the scenes fog falloff type
* @param {number} type The fog falloff type FOG_NONE,FOG_LINEAR,FOG_QUADRATIC
*/
GLGE.Scene.prototype.setFogType=function(type){	
	this.fogType=type;
	return this;
}

/**
* Gets the far fog distance
* @returns {number} the far distance of the fog
*/
GLGE.Scene.prototype.getFogFar=function(){	
	return this.fogFar;
}
/**
* Sets the scenes fog far distance
* @param {number} dist The fog far distance
*/
GLGE.Scene.prototype.setFogFar=function(dist){	
	this.fogFar=dist;
	return this;
}

/**
* Gets the near fog distance
* @returns {number} the near distance of the fog
*/
GLGE.Scene.prototype.getFogNear=function(){	
	return this.fogNear;
}
/**
* Sets the scenes fog near distance
* @param {number} dist The fog near distance
*/
GLGE.Scene.prototype.setFogNear=function(dist){	
	this.fogNear=dist;
	return this;
}

/**
* Gets the fog color
* @returns {object} An assoiative array r,g,b
*/
GLGE.Scene.prototype.getFogColor=function(){	
	return this.fogColor;
}
/**
* Sets the scenes fog color
* @param {string} color The fog color
*/
GLGE.Scene.prototype.setFogColor=function(color){	
	color=GLGE.colorParse(color);
	this.fogColor={r:color.r,g:color.g,b:color.b};
	return this;
}

/**
* Gets the scenes background color
* @returns {object} An assoiative array r,g,b
*/
GLGE.Scene.prototype.getBackgroundColor=function(){	
	return this.backgroundColor;
}
/**
* Sets the scenes background color
* @param {string} color The backgorund color
*/
GLGE.Scene.prototype.setBackgroundColor=function(color){	
	color=GLGE.colorParse(color);
	this.backgroundColor={r:color.r,g:color.g,b:color.b,a:color.a};
	return this;
}
/**
* Gets the scenes ambient light
* @returns {object} An assoiative array r,g,b
*/
GLGE.Scene.prototype.getAmbientColor=function(){	
	return this.ambientColor;
}

/**
* Sets the scenes ambient light
* @param {string} color The ambient light color
*/
GLGE.Scene.prototype.setAmbientColor=function(color){	
	color=GLGE.colorParse(color);
	this.ambientColor={r:color.r,g:color.g,b:color.b};
	if(this.renderer){
		this.renderer.gl.clearColor(this.backgroundColor.r, this.backgroundColor.g, this.backgroundColor.b, 1.0);
	}
	return this;
}
/**
* Sets the scenes ambient light
* @param {number} value the red componenent of the ambient light 0-1
*/
GLGE.Scene.prototype.setAmbientColorR=function(value){	
	this.ambientColor.r=value;
	return this;
}
/**
* Sets the scenes ambient light
* @param {number} value the green componenent of the ambient light 0-1
*/
GLGE.Scene.prototype.setAmbientColorG=function(value){	
	this.ambientColor.g=value;
	return this;
}
/**
* Sets the scenes ambient light
* @param {number} value the blue componenent of the ambient light 0-1
*/
GLGE.Scene.prototype.setAmbientColorB=function(value){	
	this.ambientColor.b=value;
	return this;
}

/**
* Sets the active camera for this scene
* @property {GLGE.Camera} object The object to be added
*/
GLGE.Scene.prototype.setCamera=function(camera){	
	if(typeof camera=="string")  camera=GLGE.Assets.get(camera);
	this.camera=camera;
	return this;
}
/**
* Gets the scenes active camera
* @returns {GLGE.Camera} The current camera
*/
GLGE.Scene.prototype.getCamera=function(){	
	return this.camera;
}
/**
* used to initialize all the WebGL buffers etc need for this scene
* @private
*/
GLGE.Scene.prototype.GLInit=function(gl){
	this.gl=gl;
	gl.lights=this.getLights();
	//sets the camera aspect to same aspect as the canvas
	this.camera.setAspect(this.renderer.canvas.width/this.renderer.canvas.height);

	//this.createPickBuffer(gl);
	this.renderer.gl.clearColor(this.backgroundColor.r, this.backgroundColor.g, this.backgroundColor.b, 1.0);
	
	for(var i=0;i<this.children;i++){
		if(this.children[i].GLInit) children[i].GLInit(gl);
	}
}
/**
* used to clean up all the WebGL buffers etc need for this scene
* @private
*/
GLGE.Scene.prototype.GLDestroy=function(gl){
}
/**
* sort function
*/
GLGE.Scene.sortFunc=function(a,b){
	return a.zdepth-b.zdepth;
}
/**
* z sorts the objects
* @private
*/
GLGE.Scene.prototype.zSort=function(gl,objects){
	var cameraMatrix=gl.scene.camera.getViewMatrix();
	var transMatrix;
	for(var i=0;i<objects.length;i++){
		transMatrix=GLGE.mulMat4(cameraMatrix,objects[i].getModelMatrix());
		objects[i].zdepth=transMatrix[11];
	}
	objects.sort(GLGE.Scene.sortFunc);
	return objects;
}
/**
* sets the 2d filter to apply
* @param {GLGE.Filter2d} filter the filter to apply when rendering the scene
*/
GLGE.Scene.prototype.setFilter2d=function(value){
	this.filter=value;
	return this;
}
/**
* gets the 2d filter being applied apply
* @returns {GLGE.Filter2d}
*/
GLGE.Scene.prototype.getFilter2d=function(filter){
	return this.filter;
}
/**
* gets the scenes frame buffer
* @private
*/
GLGE.Scene.prototype.getFrameBuffer=function(gl){
	if(this.filter) return this.filter.getFrameBuffer(gl);
	return null;
}
/**
* culls objects from the scene
* @private
*/
GLGE.Scene.prototype.cull=function(renderobjects,camera,projection){
	return renderobjects;
}

/**
* renders the scene
* @private
*/
GLGE.Scene.prototype.render=function(gl){
	//if look at is set then look
	if(this.camera.lookAt) this.camera.Lookat(this.camera.lookAt);	
	
	this.animate();
	
	var lights=gl.lights;
	gl.scene=this;
	
	gl.disable(gl.BLEND);
	
	this.framebuffer=this.getFrameBuffer(gl);
	
	var renderObjects=this.getObjects();
	//shadow stuff
	for(var i=0; i<lights.length;i++){
		if(lights[i].castShadows){
			if(!lights[i].gl) lights[i].GLInit(gl);
			gl.bindFramebuffer(gl.FRAMEBUFFER, lights[i].frameBuffer);
			

			gl.viewport(0,0,lights[i].bufferWidth,lights[i].bufferHeight);
			gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
			var cameraMatrix=this.camera.matrix;
			var cameraPMatrix=this.camera.getProjectionMatrix();
			if(!lights[i].s_cache) lights[i].s_cache={};
			if(lights[i].s_cache.pmatrix!=lights[i].getPMatrix() || lights[i].s_cache.mvmatrix!=lights[i].getModelMatrix()){
				lights[i].s_cache.pmatrix=lights[i].getPMatrix();
				lights[i].s_cache.mvmatrix=lights[i].getModelMatrix();
				lights[i].s_cache.imvmatrix=GLGE.inverseMat4(lights[i].getModelMatrix());
				lights[i].s_cache.smatrix=GLGE.mulMat4(lights[i].getPMatrix(),lights[i].s_cache.imvmatrix);
			}
			this.camera.setProjectionMatrix(lights[i].s_cache.pmatrix);
			this.camera.matrix=lights[i].s_cache.imvmatrix;
			//draw shadows
			for(var n=0; n<renderObjects.length;n++){
				renderObjects[n].GLRender(gl, GLGE.RENDER_SHADOW,n);
			}
			gl.flush();
			this.camera.matrix=cameraMatrix;
			this.camera.setProjectionMatrix(cameraPMatrix);
			gl.bindFramebuffer(gl.FRAMEBUFFER, null);
		}
	}
	if(this.camera.animation) this.camera.animate();
	
	//null render pass to findout what else needs rendering
	this.getPasses(gl,renderObjects);	
	
	//first off render the passes
	var cameraMatrix=this.camera.matrix;
	var cameraPMatrix=this.camera.getProjectionMatrix();
	this.allowPasses=false;
	while(this.passes.length>0){
		var pass=this.passes.pop();
		gl.bindFramebuffer(gl.FRAMEBUFFER, pass.frameBuffer);
		this.camera.matrix=pass.cameraMatrix;
		this.camera.setProjectionMatrix(pass.projectionMatrix);
		this.renderPass(gl,renderObjects,pass.width,pass.height);
	}
	
	this.camera.matrix=cameraMatrix;
	this.camera.setProjectionMatrix(cameraPMatrix);
	

	gl.bindFramebuffer(gl.FRAMEBUFFER, this.framebuffer);
	this.renderPass(gl,renderObjects,this.renderer.canvas.width,this.renderer.canvas.height);	
	
	this.applyFilter(gl,renderObjects,null);
	
	this.allowPasses=true;

}
/**
* gets the passes needed to render this scene
* @private
*/
GLGE.Scene.prototype.getPasses=function(gl,renderObjects){
	for(var i=0; i<renderObjects.length;i++){
		renderObjects[i].GLRender(gl,GLGE.RENDER_NULL);
	}
}

/**
* renders the scene
* @private
*/
GLGE.Scene.prototype.renderPass=function(gl,renderObjects,width,height,type){
	if(!type) type=GLGE.RENDER_DEFAULT;
	
	gl.clearDepth(1.0);
	gl.depthFunc(gl.LEQUAL);
	gl.viewport(0,0,width,height);
	
	gl.clearColor(this.backgroundColor.r, this.backgroundColor.g, this.backgroundColor.b, this.backgroundColor.a);
	
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT | gl.STENCIL_BUFFER_BIT);
	var transObjects=[];
	gl.disable(gl.BLEND);
	for(var i=0; i<renderObjects.length;i++){
		if(!renderObjects[i].zTrans) renderObjects[i].GLRender(gl,type);
			else transObjects.push(renderObjects[i])
	}

	gl.enable(gl.BLEND);
	transObjects=this.zSort(gl,transObjects);
	for(var i=0; i<transObjects.length;i++){
		transObjects[i].GLRender(gl, type);
	}
}

GLGE.Scene.prototype.applyFilter=function(gl,renderObject,framebuffer){
	if(this.filter && this.filter.renderDepth){	
		gl.clearDepth(1.0);
		gl.depthFunc(gl.LEQUAL);
		gl.bindFramebuffer(gl.FRAMEBUFFER, this.filter.getDepthBuffer(gl));
		this.renderPass(gl,renderObject,this.filter.getDepthBufferWidth(), this.filter.getDepthBufferHeight(),GLGE.RENDER_SHADOW);	
	}
	
	if(this.filter && this.filter.renderNormal){	
		gl.clearDepth(1.0);
		gl.depthFunc(gl.LEQUAL);
		gl.bindFramebuffer(gl.FRAMEBUFFER, this.filter.getNormalBuffer(gl));
		this.renderPass(gl,renderObject,this.filter.getNormalBufferWidth(),this.filter.getNormalBufferHeight(),GLGE.RENDER_NORMAL);	
	}
	
	if(this.filter) this.filter.GLRender(gl,framebuffer);
}

/**
* Adds and additional render pass to the scene for RTT, reflections and refractions
* @private
*/
GLGE.Scene.prototype.addRenderPass=function(frameBuffer,cameraMatrix,projectionMatrix,width,height){
	if(this.allowPasses)	this.passes.push({frameBuffer:frameBuffer, cameraMatrix:cameraMatrix, projectionMatrix:projectionMatrix, height:height, width:width});
	return this;
}
/**
* Sets up the WebGL needed create a picking frame and render buffer
* @private
*/
/*GLGE.Scene.prototype.createPickBuffer=function(gl){
    this.framePickBuffer = gl.createFramebuffer();
    this.renderPickBufferD = gl.createRenderbuffer();
    this.renderPickBufferC = gl.createRenderbuffer();
    //this.pickTexture = gl.createTexture();
    //gl.bindTexture(gl.TEXTURE_2D, this.pickTexture);

    //TODO update when null is accepted
   /* try {
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 4, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, null);
    } catch (e) {
        var tex = new WebGLUnsignedByteArray(4*1*4);
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 4,1, 0, gl.RGBA, gl.UNSIGNED_BYTE, tex);
    }
    
    gl.bindFramebuffer(gl.FRAMEBUFFER, this.framePickBuffer);
    gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderPickBufferD);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16,4, 1);
    //gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.pickTexture, 0);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, this.renderPickBufferD);
    
    
    gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderPickBufferC);
    gl.renderbufferStorage(gl.RENDERBUFFER, gl.RGBA,4, 1);
    gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.RENDERBUFFER, this.renderPickBufferC);
    
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.bindTexture(gl.TEXTURE_2D, null);
}*/

/**
* ray query from origin in the given direction
* @param origin the source of the ray
* @param direction the direction of the ray
*/
GLGE.Scene.prototype.ray=function(origin,direction){

		var gl=this.renderer.gl;
		var origmatrix=this.camera.matrix;	
		var origpmatrix=this.camera.pMatrix;
		
		this.camera.matrix=GLGE.inverseMat4(GLGE.Mat4([direction[2], direction[1], direction[0], origin[0],
									direction[0], direction[2], direction[1], origin[1],
									direction[1], direction[0], direction[2], origin[2],
									0, 0, 0, 1]));

		if(!this.pickPMatrix)	this.pickPMatrix=GLGE.makeOrtho(-0.0001,0.0001,-0.0001,0.0001,this.camera.near,this.camera.far);
		this.camera.pMatrix=this.pickPMatrix;
		gl.viewport(0,0,8,1);
		gl.clear(gl.DEPTH_BUFFER_BIT);
		gl.disable(gl.BLEND);
		gl.scene=this;
		var objects=this.getObjects();
		for(var i=0; i<objects.length;i++){
			if(objects[i].pickable) objects[i].GLRender(gl,GLGE.RENDER_PICK,i+1);
		}
		gl.flush();

		var data = new Uint8Array(8 * 1 * 4);
		gl.readPixels(0, 0, 8, 1, gl.RGBA,gl.UNSIGNED_BYTE, data);
		
		
		var norm=[data[4]/255,data[5]/255,data[6]/255];
		var normalsize=Math.sqrt(norm[0]*norm[0]+norm[1]*norm[1]+norm[2]*norm[2])*0.5;
		norm=[norm[0]/normalsize-1,norm[1]/normalsize-1,norm[2]/normalsize-1];
		var obj=objects[data[0]+data[1]*256+data[2]*65536-1];
		
		var dist=(data[10]/255+0.00390625*data[9]/255+0.0000152587890625*data[8]/255)*this.camera.far;
		var tex=[];
		tex[0]=(data[14]/255+0.00390625*data[13]/255+0.0000152587890625*data[12]/255);
		tex[1]=(data[18]/255+0.00390625*data[17]/255+0.0000152587890625*data[16]/255);
		
				
		gl.bindFramebuffer(gl.FRAMEBUFFER, null);
		gl.viewport(0,0,this.renderer.canvas.width,this.renderer.canvas.height);
		
		//revert the view matrix
		this.camera.matrix=origmatrix;	
		this.camera.pMatrix=origpmatrix;
		return {object:obj,distance:dist,coord:[origin[0]-direction[0]*dist,origin[1]-direction[1]*dist,origin[2]-direction[2]*dist],normal:norm,texture:tex};
}

/**
* Picks and object from canvas coords
* @param x the canvas x coord to pick
* @param y the canvas y coord to pick
*/
GLGE.Scene.prototype.pick=function(x,y){
	if(!this.camera){
		GLGE.error("No camera set for picking");
		return false;
	}else if(this.camera.matrix && this.camera.pMatrix){
		xcoord =  -( ( ( 2 * x ) / this.renderer.canvas.width ) - 1 ) / this.camera.pMatrix[0];
		ycoord =( ( ( 2 * y ) / this.renderer.canvas.height ) - 1 ) / this.camera.pMatrix[5];
		zcoord =  1;
		var coord=[xcoord,ycoord,zcoord,0];
		coord=GLGE.mulMat4Vec4(GLGE.inverseMat4(this.camera.matrix),coord);
		var cameraPos=this.camera.getPosition();
		var origin=[cameraPos.x,cameraPos.y,cameraPos.z];
		return this.ray(origin,coord);
		
	}else{
		return false;
	}
	
}

/**
* @class Sets the scene to render
* @param {GLGE.Scene} scene The scene to be rendered
*/
GLGE.Renderer=function(canvas,error){
	this.canvas=canvas;
	try {
		this.gl = canvas.getContext("experimental-webgl",{alpha:true,depth:true,stencil:true,antialias:true,premultipliedAlpha:true});
	} catch(e) {}
	if(!this.gl) {
		if(!error){
			var div=document.createElement("div");
			div.setAttribute("style","position: absolute; top: 10px; left: 10px; font-family: sans-serif; font-size: 14px; padding: 10px;background-color: #fcffcb;color: #800; width: 200px; border:2px solid #f00");
			div.innerHTML="Cannot detect webgl, please download a <b><a href='http://www.khronos.org/webgl/wiki/Getting_a_WebGL_Implementation'>compatible browser</a></b>";
			document.getElementsByTagName("body")[0].appendChild(div);
			throw "cannot create webgl context";
		}else{
			error();
			throw "cannot create webgl context";
		}
	}
	//firefox is doing something here?
	try{
	this.gl.canvas=canvas;
	}catch(e){};
	//this.gl = WebGLDebugUtils.makeDebugContext(this.gl);
	//this.gl.setTracing(true);

	//chome compatibility
	//TODO: Remove this when chome is right
	if (!this.gl.getProgramParameter)
	{
		this.gl.getProgramParameter = this.gl.getProgrami
	}
	if (!this.gl.getShaderParameter)
	{
		this.gl.getShaderParameter = this.gl.getShaderi
	}
	// End of Chrome compatibility code
	
	this.gl.uniformMatrix4fvX=this.gl.uniformMatrix4fv
	this.gl.uniformMatrix4fv=function(uniform,transpose,array){
		if(!transpose){
			this.uniformMatrix4fvX(uniform,false,array);
		}else{
			GLGE.mat4gl(GLGE.transposeMat4(array),array);
			this.uniformMatrix4fvX(uniform,false,array);
		}
	}
	var gl=this.gl;
	
	/*this.gl.texImage2Dx=this.gl.texImage2D;
	this.gl.texImage2D=function(){
		if(arguments.length==9){
			gl.texImage2Dx(arguments[0], arguments[1], arguments[2],arguments[3],arguments[4],arguments[5],arguments[6],arguments[7],arguments[8]);
		}else{
			gl.texImage2Dx(arguments[0], arguments[1], arguments[5],false,false);
		}
	}*/

	
	//set up defaults
	this.gl.clearDepth(1.0);
	this.gl.clearStencil(0);
	this.gl.enable(this.gl.DEPTH_TEST);
    
	//this.gl.enable(this.gl.CULL_FACE);
    
	this.gl.depthFunc(this.gl.LEQUAL);
	this.gl.blendFuncSeparate(this.gl.SRC_ALPHA,this.gl.ONE_MINUS_SRC_ALPHA,this.gl.ZERO,this.gl.ONE);	
};
GLGE.Renderer.prototype.gl=null;
GLGE.Renderer.prototype.scene=null;
/**
* Gets the scene which is set to be rendered
* @returns the current render scene
*/
GLGE.Renderer.prototype.getScene=function(){
	return this.scene;
};
/**
* Sets the scene to render
* @param {GLGE.Scene} scene The scene to be rendered
*/
GLGE.Renderer.prototype.setScene=function(scene){
	scene.renderer=this;
	this.scene=scene;
	scene.GLInit(this.gl);
	return this;
};
/**
* Renders the current scene to the canvas
*/
GLGE.Renderer.prototype.render=function(){
	this.scene.render(this.gl);
	//if this is the first ever pass then render twice to fill shadow buffers
	if(!this.rendered){
		this.scene.render(this.gl);
		this.rendered=true;
	}
};


/**
* @class A texture to be included in a material
* @param {string} uid the unique id for this texture
* @see GLGE.Material
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.Texture=function(uid){
	GLGE.Assets.registerAsset(this,uid);
}
GLGE.augment(GLGE.QuickNotation,GLGE.Texture);
GLGE.augment(GLGE.JSONLoader,GLGE.Texture);
GLGE.Texture.prototype.className="Texture";
GLGE.Texture.prototype.image=null;
GLGE.Texture.prototype.glTexture=null;
GLGE.Texture.prototype.url=null;
/**
* Gets the textures used by the layer
* @return {string} The textures image url
*/
GLGE.Texture.prototype.getSrc=function(){
	return this.url;
};

/**
* Sets the textures image location
* @param {string} url the texture image url
*/
GLGE.Texture.prototype.setSrc=function(url){
	this.url=url;
	this.state=0;
	this.image=new Image();
	var texture=this;
	this.image.onload = function(){
		texture.state=1;
	}	
	this.image.src=url;	
	if(this.glTexture && this.gl){
		this.gl.deleteTexture(this.glTexture);
		this.glTexture=null;
	}
	return this;
};

/**
* Sets the textures image location
* @private
**/
GLGE.Texture.prototype.doTexture=function(gl){
	this.gl=gl;
	//create the texture if it's not already created
	if(!this.glTexture) this.glTexture=gl.createTexture();
	//if the image is loaded then set in the texture data
	if(this.state==1){
		gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE,this.image);
		gl.generateMipmap(gl.TEXTURE_2D);
		gl.bindTexture(gl.TEXTURE_2D, null);
		this.state=2;
	}
	gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.REPEAT);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.REPEAT);
	
	if(this.state==2) return true;
		else return false;
}

/**
* @class A canvase texture to be included in a material
* @param {string} uid the unique id for this texture
* @see GLGE.Material
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.TextureCanvas=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.canvas=document.createElement("canvas");
}
GLGE.augment(GLGE.QuickNotation,GLGE.TextureCanvas);
GLGE.augment(GLGE.JSONLoader,GLGE.TextureCanvas);
GLGE.TextureCanvas.prototype.className="TextureCanvas";
GLGE.TextureCanvas.prototype.glTexture=null;
/**
* Gets the canvas used by the texture
* @return {canvas} The textures image url
*/
GLGE.TextureCanvas.prototype.getCanvas=function(){
	return this.canvas;
};
/**
* Sets the canvas used by the texture
* @param {canvas} canvas The canvas to use
*/
GLGE.TextureCanvas.prototype.setCanvas=function(canvas){
	this.canvas=canvas;
	return this;
};
/**
* Sets the canvas height
* @param {number} value The canvas height
*/
GLGE.TextureCanvas.prototype.setHeight=function(value){
	this.canvas.height=value;
	return this;
};
/**
* Sets the canvas width
* @param {number} value The canvas width
*/
GLGE.TextureCanvas.prototype.setWidth=function(value){
	this.canvas.width=value;
	return this;
};

/**
* gets the canvas height
* @returns {number} The canvas height
*/
GLGE.TextureCanvas.prototype.getHeight=function(){
	return this.canvas.height;
};
/**
* gets the canvas width
* @returns {number} The canvas width
*/
GLGE.TextureCanvas.prototype.getWidth=function(){
	return this.canvas.width;
};

/**
* does the canvas texture GL stuff
* @private
**/
GLGE.TextureCanvas.prototype.doTexture=function(gl){
	this.gl=gl;
	//create the texture if it's not already created
	if(!this.glTexture){
		this.glTexture=gl.createTexture();
		gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
		this.updateCanvas(gl);
	}else{
		gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
		this.updateCanvas(gl);
	}

	
	return true;
}
/**
* Updates the canvas texture
* @private
*/
GLGE.TextureCanvas.prototype.updateCanvas=function(gl){
	var canvas = this.canvas;
	
	gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
	//TODO: fix this when minefield is upto spec
	try{gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas);}
	catch(e){gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, canvas,null);}
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.generateMipmap(gl.TEXTURE_2D);
}


/**
* @class A video texture to be included in a material
* @param {string} uid the unique id for this texture
* @see GLGE.Material
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.TextureVideo=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.video=document.createElement("video");
	this.video.style.display="none";
	this.video.setAttribute("loop","loop");
	this.video.autoplay=true;
	//looping isn't working in firefox so quick fix!
	this.video.addEventListener("ended", function() { this.play(); }, true); 
	//video needs to be part of page to work for some reason :-s
	document.getElementsByTagName("body")[0].appendChild(this.video);
	//used to get webkit working
	this.canvas=document.createElement("canvas");
	this.ctx=this.canvas.getContext("2d");
	
}
GLGE.augment(GLGE.QuickNotation,GLGE.TextureVideo);
GLGE.augment(GLGE.JSONLoader,GLGE.TextureVideo);
GLGE.TextureVideo.prototype.className="TextureVideo";
GLGE.TextureVideo.prototype.glTexture=null;
/**
* Gets the canvas used by the texture
* @return {video} The textures image url
*/
GLGE.TextureVideo.prototype.getVideo=function(){
	return this.video;
};
/**
* Sets the video used by the texture
* @param {video} canvas The canvas to use
*/
GLGE.TextureVideo.prototype.setVideo=function(video){
	this.video=video;
	return this;
};

/**
* Sets the source used for the video
* @param {string} src The URL of the video
*/
GLGE.TextureVideo.prototype.setSrc=function(src){
	this.video.src=src;
	return this;
};
/**
* gets the source used for the video
* @returns {string} The URL of the video
*/
GLGE.TextureVideo.prototype.getSrc=function(src){
	return this.video.src;
};

/**
* does the canvas texture GL stuff
* @private
**/
GLGE.TextureVideo.prototype.doTexture=function(gl){
	this.gl=gl;
	//create the texture if it's not already created
	if(!this.glTexture){
		this.glTexture=gl.createTexture();
		gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
		this.updateTexture(gl);
	}else{
		gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
		this.updateTexture(gl);
	}

	
	return true;
}
/**
* Updates the canvas texture
* @private
*/
GLGE.TextureVideo.prototype.updateTexture=function(gl){
	var video = this.video;
	gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
	//TODO: fix this when minefield is upto spec
	if(video.readyState>0){
	if(video.height<=0){
		video.style.display="";
		video.height=video.offsetHeight;
		video.width=video.offsetWidth;
		video.style.display="none";
	}
	this.canvas.height=video.height;
	this.canvas.width=video.width;
	this.ctx.drawImage(video, 0, 0);
	try{gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.canvas);}
	catch(e){gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.canvas,null);}
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.generateMipmap(gl.TEXTURE_2D);
	
	/*
	use when video is working in webkit
	try{gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, video);}
	catch(e){gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, video,null);}
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
	gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
	gl.generateMipmap(gl.TEXTURE_2D);
	*/
	}
}



/**
* @class A reflection texture will reflect in a plane for a specified transform
* @param {string} uid the unique id for this texture
* @see GLGE.Material
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.TextureCamera=function(uid){
	GLGE.Assets.registerAsset(this,uid);
}
GLGE.augment(GLGE.QuickNotation,GLGE.TextureCamera);
GLGE.augment(GLGE.JSONLoader,GLGE.TextureCamera);
GLGE.TextureCamera.prototype.className="Texture";
GLGE.TextureCamera.prototype.texture=null;
GLGE.TextureCamera.prototype.glTexture=null;
GLGE.TextureCamera.prototype.object=null;
GLGE.TextureCamera.prototype.camera=null;
GLGE.TextureCamera.prototype.bufferHeight=0;
GLGE.TextureCamera.prototype.bufferWidth=0;
GLGE.TextureCamera.prototype.mirrorAxis=GLGE.NONE;
GLGE.TextureCamera.prototype.clipAxis=GLGE.NONE;

/**
* sets the RTT  render buffer width
* @param {number} buffer width
**/
GLGE.TextureCamera.prototype.setBufferWidth=function(width){
	this.bufferWidth=width;
	this.update=true;
	return this;
}
/**
* gets the RTT  render buffer width
* @returns the width
**/
GLGE.TextureCamera.prototype.getBufferWidth=function(){
	return this.bufferWidth;
}

/**
* sets the RTT  render buffer height
* @param {number} buffer height
**/
GLGE.TextureCamera.prototype.setBufferHeight=function(height){
	this.bufferHeight=height;
	this.update=true;
	return this;
}
/**
* gets the RTT  render buffer height
* @returns the height
**/
GLGE.TextureCamera.prototype.getBufferHeight=function(){
	return this.bufferHeight;
}

/**
* sets the RTT  clip axis
* @param {number} the axis
**/
GLGE.TextureCamera.prototype.setClipAxis=function(camera){
	this.clipAxis=camera;
	return this;
}
/**
* gets the RTT clip axis
* @returns the axis
**/
GLGE.TextureCamera.prototype.getClipAxis=function(){
	return this.clipAxis;
}

/**
* sets the RTT  mirror axis
* @param {number} the axis
**/
GLGE.TextureCamera.prototype.setMirrorAxis=function(camera){
	this.mirrorAxis=camera;
	return this;
}
/**
* gets the RTT mirror axis
* @returns the axis
**/
GLGE.TextureCamera.prototype.getMirrorAxis=function(){
	return this.mirrorAxis;
}

/**
* sets the RTT camera to use
* @param {GLGE.Camera} the source camera
**/
GLGE.TextureCamera.prototype.setCamera=function(camera){
	this.camera=camera;
	return this;
}
/**
* gets the RTT source camera
* @returns {GLGE.Camera} the source camera
**/
GLGE.TextureCamera.prototype.getCamera=function(){
	return this.camera;
}

/**
* does what is needed to get the texture
* @private
**/
GLGE.TextureCamera.prototype.doTexture=function(gl,object){
	if(this.camera){
		this.gl=gl;
		var modelmatrix=object.getModelMatrix();
		var pmatrix=gl.scene.camera.getProjectionMatrix();
		var cameramatrix=this.camera.getViewMatrix();
		var matrix;
		
		if(this.mirrorAxis){
			switch(this.mirrorAxis){
				case GLGE.XAXIS:
					matrix=GLGE.mulMat4(GLGE.mulMat4(GLGE.mulMat4(cameramatrix,modelmatrix),GLGE.scaleMatrix(-1,1,1)),GLGE.inverseMat4(modelmatrix));
				break;
				case GLGE.YAXIS:
					matrix=GLGE.mulMat4(GLGE.mulMat4(GLGE.mulMat4(cameramatrix,modelmatrix),GLGE.scaleMatrix(1,-1,1)),GLGE.inverseMat4(modelmatrix));
				break;
				case GLGE.ZAXIS:
					matrix=GLGE.mulMat4(GLGE.mulMat4(GLGE.mulMat4(cameramatrix,modelmatrix),GLGE.scaleMatrix(1,1,-1)),GLGE.inverseMat4(modelmatrix));
				break;
			}
		}else{
			matrix=cameramatrix;
		}
		
		if(this.clipAxis){
			var clipplane
			switch(this.clipAxis){
				case GLGE.NEG_XAXIS:
					var dirnorm=GLGE.toUnitVec3([-modelmatrix[0],-modelmatrix[4],-modelmatrix[8]]);
					clipplane=[dirnorm[0],dirnorm[1],dirnorm[2],-GLGE.dotVec3([modelmatrix[3],modelmatrix[7],modelmatrix[11]],dirnorm)];
					break;
				case GLGE.POS_XAXIS:
					var dirnorm=GLGE.toUnitVec3([modelmatrix[0],modelmatrix[4],modelmatrix[8]]);
					clipplane=[dirnorm[0],dirnorm[1],dirnorm[2],-GLGE.dotVec3([modelmatrix[3],modelmatrix[7],modelmatrix[11]],dirnorm)];
					break;
				case GLGE.NEG_YAXIS:
					var dirnorm=GLGE.toUnitVec3([-modelmatrix[1],-modelmatrix[5],-modelmatrix[9]]);
					clipplane=[dirnorm[0],dirnorm[1],dirnorm[2],-GLGE.dotVec3([modelmatrix[3],modelmatrix[7],modelmatrix[11]],dirnorm)];
					break;
				case GLGE.POS_YAXIS:
					var dirnorm=GLGE.toUnitVec3([modelmatrix[1],modelmatrix[5],modelmatrix[9]]);
					clipplane=[dirnorm[0],dirnorm[1],dirnorm[2],-GLGE.dotVec3([modelmatrix[3],modelmatrix[7],modelmatrix[11]],dirnorm)];
					break;
				case GLGE.NEG_ZAXIS:
					var dirnorm=GLGE.toUnitVec3([-modelmatrix[2],-modelmatrix[6],-modelmatrix[10]]);
					clipplane=[dirnorm[0],dirnorm[1],dirnorm[2],-GLGE.dotVec3([modelmatrix[3],modelmatrix[7],modelmatrix[11]],dirnorm)+0.001];
					break;
				case GLGE.POS_ZAXIS:
					var dirnorm=GLGE.toUnitVec3([modelmatrix[2],modelmatrix[6],modelmatrix[10]]);
					clipplane=[dirnorm[0],dirnorm[1],dirnorm[2],-GLGE.dotVec3([modelmatrix[3],modelmatrix[7],modelmatrix[11]],dirnorm)+0.001];
					break;
			}
			
			var itmvp=GLGE.transposeMat4(GLGE.inverseMat4(GLGE.mulMat4(pmatrix,matrix)));

			clipplane=GLGE.mulMat4Vec4(itmvp,clipplane);
			clipplane=GLGE.scaleVec4(clipplane,pmatrix[10]);
			clipplane[3] -= 1;
			if(clipplane[2]<0) GLGE.scaleVec4(clipplane,-1);
			var suffix=[ 1,0,0,0,
					0,1,0,0,
					clipplane[0],clipplane[1],clipplane[2],clipplane[3],
					0,0,0,1];
			pmatrix=GLGE.mulMat4(suffix,pmatrix);
		}
		var height=(!this.bufferHeight ? gl.scene.renderer.canvas.height : this.bufferHeight);
		var width=(!this.bufferWidth ? gl.scene.renderer.canvas.width : this.bufferWidth);
	
		//create the texture if it's not already created
		if(!this.glTexture || this.update){
			this.createFrameBuffer(gl);
			gl.scene.addRenderPass(this.frameBuffer,matrix, gl.scene.camera.getProjectionMatrix(),width,height);
			gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
			this.update=false;
			return false;
		}else{	
			gl.bindTexture(gl.TEXTURE_2D, this.glTexture);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
			gl.scene.addRenderPass(this.frameBuffer,matrix, pmatrix,width,height);
			return true;
		}
	}else{
		return false;
	}
}
GLGE.TextureCamera.prototype.registerPasses=GLGE.TextureCamera.prototype.doTexture;
/**
* Creates the frame buffer for our texture
* @private
*/
GLGE.TextureCamera.prototype.createFrameBuffer=function(gl){
	var height=(!this.bufferHeight ? gl.scene.renderer.canvas.height : this.bufferHeight);
	var width=(!this.bufferWidth ? gl.scene.renderer.canvas.width : this.bufferWidth);
	
	if(!this.frameBuffer) this.frameBuffer = gl.createFramebuffer();
	if(!this.renderBuffer) this.renderBuffer = gl.createRenderbuffer();
	if(!this.glTexture) this.glTexture=gl.createTexture();
	gl.bindTexture(gl.TEXTURE_2D, this.glTexture);

	var tex = new Uint8Array(width*height*4);
	gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, width,height, 0, gl.RGBA, gl.UNSIGNED_BYTE, tex);
    
	gl.bindFramebuffer(gl.FRAMEBUFFER, this.frameBuffer);
    
	gl.bindRenderbuffer(gl.RENDERBUFFER, this.renderBuffer);
	//dpeth stencil doesn't seem to work in either webkit or mozilla so don't use for now - reflected particles will be messed up!
	//gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_STENCIL,width, height);
	//gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_STENCIL_ATTACHMENT, gl.RENDERBUFFER, this.renderBuffer);
	gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16,width, height);
	gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, this.renderBuffer);
    
	gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, this.glTexture, 0);	
    
	gl.bindRenderbuffer(gl.RENDERBUFFER, null);
	gl.bindFramebuffer(gl.FRAMEBUFFER, null);
	gl.bindTexture(gl.TEXTURE_2D, null);
}




/**
* @class A texture to be included in a material
* @param {string} uid the unique id for this texture
* @see GLGE.Material
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
*/
GLGE.TextureCube=function(uid){
	GLGE.Assets.registerAsset(this,uid);
}
GLGE.augment(GLGE.QuickNotation,GLGE.TextureCube);
GLGE.augment(GLGE.JSONLoader,GLGE.TextureCube);
GLGE.TextureCube.prototype.className="TextureCube";
GLGE.TextureCube.prototype.posX=null;
GLGE.TextureCube.prototype.negX=null;
GLGE.TextureCube.prototype.posY=null;
GLGE.TextureCube.prototype.negY=null;
GLGE.TextureCube.prototype.posZ=null;
GLGE.TextureCube.prototype.negZ=null;
GLGE.TextureCube.prototype.texture=null;
GLGE.TextureCube.prototype.glTexture=null;
GLGE.TextureCube.prototype.loadState=0;
/**
* Sets the url for a given image
* @param {string} url the texture image url
* @param {string} image the image element to load
*/
GLGE.TextureCube.prototype.setSrc=function(url,image,mask){
	this.url=url;
	this.state=0;
	this[image]=new Image();
	var texture=this;
	this[image].onload = function(){
		texture.loadState+=mask;
	}	
	this[image].src=url;	
	if(this.glTexture && this.gl) {
		this.gl.deleteTexture(this.glTexture);
		this.glTexture=null;
	}
	return this;
}

/**
* Sets the positive X cube image
* @param {string} url the texture image url
*/
GLGE.TextureCube.prototype.setSrcPosX=function(url){
	this.setSrc(url,"posX",1);
	return this;
};
/**
* Sets the negative X cube image
* @param {string} url the texture image url
*/
GLGE.TextureCube.prototype.setSrcNegX=function(url){
	this.setSrc(url,"negX",2);
	return this;
};
/**
* Sets the positive Y cube image
* @param {string} url the texture image url
*/
GLGE.TextureCube.prototype.setSrcPosY=function(url){
	this.setSrc(url,"posY",4);
	return this;
};
/**
* Sets the negative Y cube image
* @param {string} url the texture image url
*/
GLGE.TextureCube.prototype.setSrcNegY=function(url){
	this.setSrc(url,"negY",8);
	return this;
};
/**
* Sets the positive Z cube image
* @param {string} url the texture image url
*/
GLGE.TextureCube.prototype.setSrcPosZ=function(url){
	this.setSrc(url,"posZ",16);
	return this;
};
/**
* Sets the negative Z cube image
* @param {string} url the texture image url
*/
GLGE.TextureCube.prototype.setSrcNegZ=function(url){
	this.setSrc(url,"negZ",32);
	return this;
};

/**
* Sets the textures image location
* @private
**/
GLGE.TextureCube.prototype.doTexture=function(gl){
	this.gl=gl;
	//create the texture if it's not already created
	if(!this.glTexture) this.glTexture=gl.createTexture();
	//if the image is loaded then set in the texture data
	gl.bindTexture(gl.TEXTURE_CUBE_MAP, this.glTexture);
	if(this.loadState==63 && this.state==0){
		gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_X, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.posX);
		gl.texImage2D(gl.TEXTURE_CUBE_MAP_NEGATIVE_X, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.negX);
		gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_Y, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.posY);
		gl.texImage2D(gl.TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.negY);
		gl.texImage2D(gl.TEXTURE_CUBE_MAP_POSITIVE_Z, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.posZ);
		gl.texImage2D(gl.TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, this.negZ);
		gl.generateMipmap(gl.TEXTURE_CUBE_MAP);
		gl.bindTexture(gl.TEXTURE_CUBE_MAP, null);
		this.state=1;
	}
	gl.bindTexture(gl.TEXTURE_CUBE_MAP, this.glTexture);
	if(this.state==1) return true;
		else return false;
}


/**
* @class The material layer describes how to apply this layer to the material
* @see GLGE.Material
* @augments GLGE.Animatable
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
* @augments GLGE.Events
*/
GLGE.MaterialLayer=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.blendMode=GLGE.BL_MIX;
};
GLGE.augment(GLGE.Animatable,GLGE.MaterialLayer);
GLGE.augment(GLGE.QuickNotation,GLGE.MaterialLayer);
GLGE.augment(GLGE.JSONLoader,GLGE.MaterialLayer);
GLGE.augment(GLGE.Events,GLGE.MaterialLayer);
/**
 * @name GLGE.MaterialLayer#shaderupdated
 * @event Fires when a change will result in a change to the GLSL shader
 * @param {object} data
 */
 
GLGE.MaterialLayer.prototype.className="MaterialLayer";
GLGE.MaterialLayer.prototype.texture=null;
GLGE.MaterialLayer.prototype.blendMode=null;
GLGE.MaterialLayer.prototype.mapto=GLGE.M_COLOR;
GLGE.MaterialLayer.prototype.mapinput=GLGE.UV1;
GLGE.MaterialLayer.prototype.scaleX=1;
GLGE.MaterialLayer.prototype.offsetX=0;
GLGE.MaterialLayer.prototype.rotX=0;
GLGE.MaterialLayer.prototype.scaleY=1;
GLGE.MaterialLayer.prototype.offsetY=0;
GLGE.MaterialLayer.prototype.rotY=0;
GLGE.MaterialLayer.prototype.scaleZ=1;
GLGE.MaterialLayer.prototype.offsetZ=0;
GLGE.MaterialLayer.prototype.rotZ=0;
GLGE.MaterialLayer.prototype.dScaleX=0;
GLGE.MaterialLayer.prototype.dOffsetX=0;
GLGE.MaterialLayer.prototype.dRotX=0;
GLGE.MaterialLayer.prototype.dScaleY=0;
GLGE.MaterialLayer.prototype.dOffsetY=0;
GLGE.MaterialLayer.prototype.dRotY=0;
GLGE.MaterialLayer.prototype.dScaleZ=0;
GLGE.MaterialLayer.prototype.dOffsetZ=0;
GLGE.MaterialLayer.prototype.dRotZ=0;
GLGE.MaterialLayer.prototype.alpha=1;
GLGE.MaterialLayer.prototype.height=0.05;
GLGE.MaterialLayer.prototype.matrix=null;

/**
* Gets the textures used by the layer
* @return {GLGE.Texture} The current shininess of the material
*/
GLGE.MaterialLayer.prototype.getMatrix=function(){
	if(!this.matrix){
		var offset=this.getOffset();
		var scale=this.getScale();
		var rotation=this.getRotation();
		this.matrix=GLGE.mulMat4(GLGE.mulMat4(GLGE.translateMatrix(offset.x,offset.y,offset.z),GLGE.scaleMatrix(scale.x,scale.y,scale.z)),GLGE.rotateMatrix(rotation.x,rotation.y,rotation.z));
	}
	return this.matrix;
};
/**
* Sets the height for this layer, currently only used for parallax mapping
* @param {number} the height of this layer
*/
GLGE.MaterialLayer.prototype.setHeight=function(value){
	this.height=value;
	return this;
};
/**
* Gets the height for this layer, currently only used for parallax mapping
* @return {number} the height of this layer
*/
GLGE.MaterialLayer.prototype.getHeight=function(){
	return this.height;
};

/**
* Sets the textures alpha blending value
* @param {number} the alpha for this layer
*/
GLGE.MaterialLayer.prototype.setAlpha=function(value){
	this.alpha=value;
	return this;
};
/**
* Gets the textures alpha blending value
* @return {number} the alpha for this layer
*/
GLGE.MaterialLayer.prototype.getAlpha=function(){
	return this.alpha;
};

/**
* Sets the textures used by the layer
* @param {GLGE.Texture} value the teture to associate with this layer
*/
GLGE.MaterialLayer.prototype.setTexture=function(value){
	if(typeof value=="string")  value=GLGE.Assets.get(value);
	this.texture=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the textures used by the layer
* @return {GLGE.Texture} The current shininess of the material
*/
GLGE.MaterialLayer.prototype.getTexture=function(){
	return this.texture;
};
/**
* Sets the flag for how this layer maps to the material
* @param {Number} value the flags to set for this layer
*/
GLGE.MaterialLayer.prototype.setMapto=function(value){
	this.mapto=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the flag representing the way the layer maps to the material
* @return {Number} The flags currently set for this layer
*/
GLGE.MaterialLayer.prototype.getMapto=function(){
	return this.mapto;
};
/**
* Sets the texture coordinate system
* @param {Number} value the mapping to use
*/
GLGE.MaterialLayer.prototype.setMapinput=function(value){
	this.mapinput=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the texture coordinate system
* @return {Number} The current mapping
*/
GLGE.MaterialLayer.prototype.getMapinput=function(){
	return this.mapinput;
};

/**
* Gets the layers texture offset
* @return {object} the current offset
*/
GLGE.MaterialLayer.prototype.getOffset=function(){
	var offset={};
	offset.x=parseFloat(this.getOffsetX())+parseFloat(this.getDOffsetX());
	offset.y=parseFloat(this.getOffsetY())+parseFloat(this.getDOffsetY());
	offset.z=parseFloat(this.getOffsetZ())+parseFloat(this.getDOffsetZ());
	return offset;
};

/**
* Gets the layers texture rotation
* @return {object} the current rotation
*/
GLGE.MaterialLayer.prototype.getRotation=function(){
	var rotation={};
	rotation.x=parseFloat(this.getRotX())+parseFloat(this.getDRotX());
	rotation.y=parseFloat(this.getRotY())+parseFloat(this.getDRotY());
	rotation.z=parseFloat(this.getRotZ())+parseFloat(this.getDRotZ());
	return rotation;
};

/**
* Gets the layers texture scale
* @return {object} the current scale
*/
GLGE.MaterialLayer.prototype.getScale=function(){
	var scale={};
	scale.x=parseFloat(this.getScaleX())+parseFloat(this.getDScaleX());
	scale.y=parseFloat(this.getScaleY())+parseFloat(this.getDScaleY());
	scale.z=parseFloat(this.getScaleZ())+parseFloat(this.getDScaleZ());
	return scale;
};

/**
* Sets the layers texture X offset
* @param {Number} value the amount to offset the texture
*/
GLGE.MaterialLayer.prototype.setOffsetX=function(value){
	this.matrix=null;
	this.offsetX=value;
	return this;
};
/**
* Gets the layers texture X offset
* @return {Number} the current offset
*/
GLGE.MaterialLayer.prototype.getOffsetX=function(){
	return this.offsetX;
};
/**
* Sets the layers texture Y offset
* @param {Number} value the amount to offset the texture
*/
GLGE.MaterialLayer.prototype.setOffsetY=function(value){
	this.matrix=null;
	this.offsetY=value;
	return this;
};
/**
* Gets the layers texture Y offset
* @return {Number} the current offset
*/
GLGE.MaterialLayer.prototype.getOffsetY=function(){
	return this.offsetY;
};
/**
* Sets the layers texture Z offset
* @param {Number} value the amount to offset the texture
*/
GLGE.MaterialLayer.prototype.setOffsetZ=function(value){
	this.matrix=null;
	this.offsetZ=value;
	return this;
};
/**
* Gets the layers texture Z offset
* @return {Number} the current offset
*/
GLGE.MaterialLayer.prototype.getOffsetZ=function(){
	return this.offsetZ;
};
/**
* Sets the layers texture X displacment offset, useful for animation
* @param {Number} value the amount to offset the texture
*/
GLGE.MaterialLayer.prototype.setDOffsetX=function(value){
	this.matrix=null;
	this.dOffsetX=value;
	return this;
};
/**
* Gets the layers texture X displacment offset, useful for animation
* @return {Number} the current offset
*/
GLGE.MaterialLayer.prototype.getDOffsetX=function(){
	return this.dOffsetX;
};
/**
* Sets the layers texture Y displacment offset, useful for animation
* @param {Number} value the amount to offset the texture
*/
GLGE.MaterialLayer.prototype.setDOffsetY=function(value){
	this.matrix=null;
	this.dOffsetY=value;
	return this;
};
/**
* Gets the layers texture Y displacment offset, useful for animation
* @return {Number} the current offset
*/
GLGE.MaterialLayer.prototype.getDOffsetY=function(){
	return this.dOffsetY;
};
/**
* Sets the layers texture Z displacment offset, useful for animation
* @param {Number} value the amount to offset the texture
*/
GLGE.MaterialLayer.prototype.setDOffsetZ=function(value){
	this.matrix=null;
	this.dOffsetZ=value;
	return this;
};
/**
* Gets the layers texture X displacment offset, useful for animation
* @return {Number} the current offset
*/
GLGE.MaterialLayer.prototype.getDOffsetZ=function(){
	return this.dOffsetZ;
};
/**
* Sets the layers texture X scale
* @param {Number} value the amount to scale the texture
*/
GLGE.MaterialLayer.prototype.setScaleX=function(value){
	this.matrix=null;
	this.scaleX=value;
	return this;
};
/**
* Gets the layers texture X scale
* @return {Number} the current scale
*/
GLGE.MaterialLayer.prototype.getScaleX=function(){
	return this.scaleX;
};
/**
* Sets the layers texture Y scale
* @param {Number} value the amount to scale the texture
*/
GLGE.MaterialLayer.prototype.setScaleY=function(value){
	this.matrix=null;
	this.scaleY=value;
	return this;
};
/**
* Gets the layers texture Y scale
* @return {Number} the current scale
*/
GLGE.MaterialLayer.prototype.getScaleY=function(){
	return this.scaleY;
};
/**
* Sets the layers texture Z scale
* @param {Number} value the amount to scale the texture
*/
GLGE.MaterialLayer.prototype.setScaleZ=function(value){
	this.matrix=null;
	this.scaleZ=value;
	return this;
};
/**
* Gets the layers texture Z offset
* @return {Number} the current offset
*/
GLGE.MaterialLayer.prototype.getScaleZ=function(){
	return this.scaleZ;
};
/**
* Sets the layers texture X displacment scale, useful for animation
* @param {Number} value the amount to scale the texture
*/
GLGE.MaterialLayer.prototype.setDScaleX=function(value){
	this.matrix=null;
	this.dScaleX=value;
	return this;
};
/**
* Gets the layers texture X displacment scale, useful for animation
* @return {Number} the current scale
*/
GLGE.MaterialLayer.prototype.getDScaleX=function(){
	return this.dScaleX;
};
/**
* Sets the layers texture Y displacment scale, useful for animation
* @param {Number} value the amount to scale the texture
*/
GLGE.MaterialLayer.prototype.setDScaleY=function(value){
	this.matrix=null;
	this.dScaleY=value;
	return this;
};
/**
* Gets the layers texture Y displacment scale, useful for animation
* @return {Number} the current scale
*/
GLGE.MaterialLayer.prototype.getDScaleY=function(){
	return this.dScaleY;
};
/**
* Sets the layers texture Z displacment scale, useful for animation
* @param {Number} value the amount to scale the texture
*/
GLGE.MaterialLayer.prototype.setDScaleZ=function(value){
	this.matrix=null;
	this.dScaleZ=value;
	return this;
};
/**
* Gets the layers texture X displacment scale, useful for animation
* @return {Number} the current scale
*/
GLGE.MaterialLayer.prototype.getDScaleZ=function(){
	return this.dScaleZ;
};


/**
* Sets the layers texture X Rotation
* @param {Number} value the amount to roate the texture
*/
GLGE.MaterialLayer.prototype.setRotX=function(value){
	this.matrix=null;
	this.rotX=value;
	return this;
};
/**
* Gets the layers texture X rotate
* @return {Number} the current rotate
*/
GLGE.MaterialLayer.prototype.getRotX=function(){
	return this.rotX;
};
/**
* Sets the layers texture Y rotate
* @param {Number} value the amount to rotate the texture
*/
GLGE.MaterialLayer.prototype.setRotY=function(value){
	this.matrix=null;
	this.rotY=value;
	return this;
};
/**
* Gets the layers texture Y rotate
* @return {Number} the current rotate
*/
GLGE.MaterialLayer.prototype.getRotY=function(){
	return this.rotY;
};
/**
* Sets the layers texture Z rotate
* @param {Number} value the amount to rotate the texture
*/
GLGE.MaterialLayer.prototype.setRotZ=function(value){
	this.matrix=null;
	this.rotZ=value;
	return this;
};
/**
* Gets the layers texture Z rotate
* @return {Number} the current rotate
*/
GLGE.MaterialLayer.prototype.getRotZ=function(){
	return this.rotZ;
};
/**
* Sets the layers texture X displacment rotation, useful for animation
* @param {Number} value the amount to rotation the texture
*/
GLGE.MaterialLayer.prototype.setDRotX=function(value){
	this.matrix=null;
	this.dRotX=value;
	return this;
};
/**
* Gets the layers texture X displacment rotation, useful for animation
* @return {Number} the current rotation
*/
GLGE.MaterialLayer.prototype.getDRotX=function(){
	return this.dRotX;
};
/**
* Sets the layers texture Y displacment rotation, useful for animation
* @param {Number} value the amount to rotaion the texture
*/
GLGE.MaterialLayer.prototype.setDRotY=function(value){
	this.matrix=null;
	this.dRotY=value;
	return this;
};
/**
* Gets the layers texture Y displacment rotation, useful for animation
* @return {Number} the current rotation
*/
GLGE.MaterialLayer.prototype.getDRotY=function(){
	return this.dRotY;
};
/**
* Sets the layers texture Z displacment rotation, useful for animation
* @param {Number} value the amount to rotation the texture
*/
GLGE.MaterialLayer.prototype.setDRotZ=function(value){
	this.matrix=null;
	this.dRotZ=value;
	return this;
};
/**
* Gets the layers texture X displacment rotation, useful for animation
* @return {Number} the current rotation
*/
GLGE.MaterialLayer.prototype.getDRotZ=function(){
	return this.dRotZ;
};

/**
* Sets the layers blending mode
* @param {Number} value the blend mode for the layer
*/
GLGE.MaterialLayer.prototype.setBlendMode=function(value){
	this.blendMode=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the layers tblending mode
* @return {Number} the blend mode for the layer
*/
GLGE.MaterialLayer.prototype.getBlendMode=function(){
	return this.blendMode;
};




/**
* @class The Material class creates materials to be applied to objects in the graphics engine
* @see GLGE.Object
* @augments GLGE.Animatable
* @augments GLGE.QuickNotation
* @augments GLGE.JSONLoader
* @augments GLGE.Events
*/
GLGE.Material=function(uid){
	GLGE.Assets.registerAsset(this,uid);
	this.layers=[];
	this.layerlisteners=[];
	this.textures=[];
	this.lights=[];
	this.color={r:1,g:1,b:1,a:1};
	this.specColor={r:1,g:1,b:1};
	this.reflect=0.8;
	this.shine=10;
	this.specular=1;
	this.emit=0;
	this.alpha=1;
};
GLGE.augment(GLGE.Animatable,GLGE.Material);
GLGE.augment(GLGE.QuickNotation,GLGE.Material);
GLGE.augment(GLGE.JSONLoader,GLGE.Material);
GLGE.augment(GLGE.Events,GLGE.Material);


/**
 * @name GLGE.Material#shaderupdate
 * @event fires when the shader for this material needs updating
 * @param {object} data
 */

/**
* @constant 
* @description Flag for material colour
*/
GLGE.M_COLOR=1; 
/**
* @constant 
* @description Flag for material normal
*/
GLGE.M_NOR=2;
/**
* @constant 
* @description Flag for material alpha
*/
GLGE.M_ALPHA=4; 
/**
* @constant 
* @description Flag for material specular color
*/
GLGE.M_SPECCOLOR=8; 
/**
* @constant 
* @description Flag for material specular cvalue
*/
GLGE.M_SPECULAR=16;
/**
* @constant 
* @description Flag for material shineiness
*/
GLGE.M_SHINE=32; 
/**
* @constant 
* @description Flag for material reflectivity
*/
GLGE.M_REFLECT=64;
/**
* @constant 
* @description Flag for material emision
*/
GLGE.M_EMIT=128;
/**
* @constant 
* @description Flag for material alpha
*/
GLGE.M_ALPHA=256;
/**
* @constant 
* @description Flag for masking with textures red value
*/
GLGE.M_MSKR=512;
/**
* @constant 
* @description Flag for masking with textures green value
*/
GLGE.M_MSKG=1024;
/**
* @constant 
* @description Flag for masking with textures blue value
*/
GLGE.M_MSKB=2048;
/**
* @constant 
* @description Flag for masking with textures alpha value
*/
GLGE.M_MSKA=4096;
/**
* @constant 
* @description Flag for mapping of the height in parallax mapping
*/
GLGE.M_HEIGHT=8192;

/**
* @constant 
* @description Enumeration for first UV layer
*/
GLGE.UV1=0;
/**
* @constant 
* @description Enumeration for second UV layer
*/
GLGE.UV2=1;
/**
* @constant 
* @description Enumeration for normal texture coords
*/
GLGE.MAP_NORM=3;

/**
* @constant 
* @description Enumeration for object texture coords
*/
GLGE.MAP_OBJ=4;

/**
* @constant 
* @description Enumeration for reflection coords
*/
GLGE.MAP_REF=5;

/**
* @constant 
* @description Enumeration for environment coords
*/
GLGE.MAP_ENV=6;

/**
* @constant 
* @description Enumeration for view coords
*/
GLGE.MAP_VIEW=7;

/**
* @constant 
* @description Enumeration for mix blending mode
*/
GLGE.BL_MIX=0;

/**
* @constant 
* @description Enumeration for mix blending mode
*/
GLGE.BL_MUL=1;
	
GLGE.Material.prototype.layers=null;
GLGE.Material.prototype.className="Material";
GLGE.Material.prototype.textures=null;
GLGE.Material.prototype.color=null;
GLGE.Material.prototype.specColor=null;
GLGE.Material.prototype.specular=null;
GLGE.Material.prototype.emit=null;
GLGE.Material.prototype.shine=null;
GLGE.Material.prototype.reflect=null;
GLGE.Material.prototype.lights=null;
GLGE.Material.prototype.alpha=null;
GLGE.Material.prototype.shadow=true;
/**
* Sets the flag indicateing the material should or shouldn't recieve shadows
* @param {boolean} value The recieving shadow flag
*/
GLGE.Material.prototype.setShadow=function(value){
	this.shadow=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* gets the show flag
* @returns {boolean} The shadow flag
*/
GLGE.Material.prototype.getShadow=function(value){
	return this.shadow;
};
/**
* Sets the base colour of the material
* @param {string} color The colour of the material
*/
GLGE.Material.prototype.setColor=function(color){
	if(!color.r){
		color=GLGE.colorParse(color);
	}
	this.color={r:color.r,g:color.g,b:color.b};
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Sets the red base colour of the material
* @param {Number} r The new red level 0-1
*/
GLGE.Material.prototype.setColorR=function(value){
	this.color.r=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Sets the green base colour of the material
* @param {Number} g The new green level 0-1
*/
GLGE.Material.prototype.setColorG=function(value){
	this.color.g=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Sets the blue base colour of the material
* @param {Number} b The new blue level 0-1
*/
GLGE.Material.prototype.setColorB=function(value){
	this.color.b=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the current base color of the material
* @return {[r,g,b]} The current base color
*/
GLGE.Material.prototype.getColor=function(){
	return this.color;
};
/**
* Sets the base specular colour of the material
* @param {string} color The new specular colour
*/
GLGE.Material.prototype.setSpecularColor=function(color){
	if(!color.r){
		color=GLGE.colorParse(color);
	}
	this.specColor={r:color.r,g:color.g,b:color.b};
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the current base specular color of the material
* @return {[r,g,b]} The current base specular color
*/
GLGE.Material.prototype.getSpecularColor=function(){
	return this.specColor;
};
/**
* Sets the alpha of the material
* @param {Number} value how much alpha
*/
GLGE.Material.prototype.setAlpha=function(value){
	this.alpha=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the alpha of the material
* @return {Number} The current alpha of the material
*/
GLGE.Material.prototype.getAlpha=function(){
	return this.alpha;
};
/**
* Sets the specular of the material
* @param {Number} value how much specular
*/
GLGE.Material.prototype.setSpecular=function(value){
	this.specular=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the specular of the material
* @return {Number} The current specular of the material
*/
GLGE.Material.prototype.getSpecular=function(){
	return this.specular;
};
/**
* Sets the shininess of the material
* @param {Number} value how much shine
*/
GLGE.Material.prototype.setShininess=function(value){
	this.shine=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the shininess of the material
* @return {Number} The current shininess of the material
*/
GLGE.Material.prototype.getShininess=function(){
	return this.shine;
};
/**
* Sets how much the material should emit
* @param {Number} value how much to emit (0-1)
*/
GLGE.Material.prototype.setEmit=function(value){
	this.emit=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the amount this material emits
* @return {Number} The emit value for the material
*/
GLGE.Material.prototype.getEmit=function(){
	return this.emit;
};
/**
* Sets reflectivity of the material
* @param {Number} value how much to reflect (0-1)
*/
GLGE.Material.prototype.setReflectivity=function(value){
	this.reflect=value;
	this.fireEvent("shaderupdate",{});
	return this;
};
/**
* Gets the materials reflectivity
* @return {Number} The reflectivity of the material
*/
GLGE.Material.prototype.getReflectivity=function(){
	return this.reflect;
};

/**
* Add a new layer to the material
* @param {MaterialLayer} layer The material layer to add to the material
*/
GLGE.Material.prototype.addMaterialLayer=function(layer){
	if(typeof layer=="string")  layer=GLGE.Assets.get(layer);
	this.layers.push(layer);
	var material=this;
	var listener=function(event){
		material.fireEvent("shaderupdate",{});
	};
	this.layerlisteners.push(listener);
	layer.addEventListener("shaderupdate",listener);
	this.fireEvent("shaderupdate",{});
	return this;
};

/**
* Removes a layer from the material
* @param {MaterialLayer} layer The material layer to remove
*/
GLGE.Material.prototype.removeMaterialLayer=function(layer){
	var idx=this.layers.indexOf(layer);
	if(idx>=0){
		this.layers.splice(idx,1);
		layer.removeEventListener("shaderupdate",this.layerlisteners[idx]);
		this.layerlisteners.splice(idx,1);
		this.fireEvent("shaderupdate",{});
	}
	return this;
};

/**
* Gets all the materials layers
* @returns {GLGE.MaterialLayer[]} all of the layers contained within this material
*/
GLGE.Material.prototype.getLayers=function(){
	return this.layers;
};
/**
* Generate the code required to calculate the texture coords for each layer
* @private
*/
GLGE.Material.prototype.getLayerCoords=function(){
		var shader=[];
		shader.push("vec4 texturePos;\n"); 
		for(i=0; i<this.layers.length;i++){
			shader.push("textureCoords"+i+"=vec3(0.0,0.0,0.0);\n"); 
			
			if(this.layers[i].mapinput==GLGE.UV1 || this.layers[i].mapinput==GLGE.UV2){
				shader.push("texturePos=vec4(vec2(UVCoord["+(this.layers[i].mapinput*2)+"],(1.0-UVCoord["+(this.layers[i].mapinput*2+1)+"])),1.0,1.0);\n");
			}
			
			if(this.layers[i].mapinput==GLGE.MAP_NORM){
				shader.push("texturePos=vec4(normalize(n.xyz),1.0);\n");
			}
			if(this.layers[i].mapinput==GLGE.MAP_OBJ){
				shader.push("texturePos=vec4(normalize(OBJCoord.xyz),1.0);\n");
			}
			
			if(this.layers[i].mapinput==GLGE.MAP_REF){
				//will need to do in fragment to take the normal maps into account!
				shader.push("texturePos=vec4(reflect(normalize(eyevec.xyz),normalize(n.xyz)),1.0);\n");
			}
			

			
			if(this.layers[i].mapinput==GLGE.MAP_ENV){
				//will need to do in fragment to take the normal maps into account!
				shader.push("texturePos=envMat * vec4(reflect(normalize(eyevec.xyz),normalize(n.xyz)),1.0);\n");
			}
			
			shader.push("textureCoords"+i+"=(layer"+i+"Matrix * texturePos).xyz;\n");			
			
		}
		
		return shader.join("");
}
/**
* Generate the fragment shader program for this material
* @private
*/
GLGE.Material.prototype.getVertexVarying=function(){
	var shader=[];
	for(i=0; i<this.layers.length;i++){
		shader.push("uniform mat4 layer"+i+"Matrix;\n");  
		shader.push("varying vec3 textureCoords"+i+";\n"); 
	}
	return shader.join("");
}

GLGE.Material.prototype.registerPasses=function(gl,object){
	for(var i=0; i<this.textures.length;i++){
		if(this.textures[i].registerPasses) this.textures[i].registerPasses(gl,object);
	}
}

/**
* Generate the fragment shader program for this material
* @private
*/
GLGE.Material.prototype.getFragmentShader=function(lights){
	var shader="#ifdef GL_ES\nprecision mediump float;\n#endif\n";
	var tangent=false;
	for(var i=0; i<lights.length;i++){
		if(lights[i].type==GLGE.L_POINT || lights[i].type==GLGE.L_SPOT || lights[i].type==GLGE.L_DIR){
			shader=shader+"varying vec3 lightvec"+i+";\n"; 
			shader=shader+"varying vec3 tlightvec"+i+";\n"; 
			shader=shader+"varying vec3 lightpos"+i+";\n"; 
			shader=shader+"varying vec3 tlightdir"+i+";\n"; 
			shader=shader+"varying float lightdist"+i+";\n";  
			shader=shader+"varying vec2 spotCoords"+i+";\n"; 
		}
	}
	shader=shader+"varying vec3 n;\n";  
	shader=shader+"varying vec3 b;\n";  
	shader=shader+"varying vec3 t;\n";  
	shader=shader+"varying vec4 UVCoord;\n";
	shader=shader+"varying vec3 eyevec;\n"; 
	shader=shader+"varying vec3 OBJCoord;\n";
	shader=shader+"varying vec3 teyevec;\n";

	//texture uniforms
	for(var i=0; i<this.textures.length;i++){
		if(this.textures[i].className=="Texture") shader=shader+"uniform sampler2D TEXTURE"+i+";\n";
		if(this.textures[i].className=="TextureCanvas") shader=shader+"uniform sampler2D TEXTURE"+i+";\n";
		if(this.textures[i].className=="TextureVideo") shader=shader+"uniform sampler2D TEXTURE"+i+";\n";
		if(this.textures[i].className=="TextureCube") shader=shader+"uniform samplerCube TEXTURE"+i+";\n";
	}
	
	var cnt=0;
	var shadowlights=[];
	var num;
	for(var i=0; i<lights.length;i++){
			shader=shader+"uniform vec3 lightcolor"+i+";\n";  
			shader=shader+"uniform vec3 lightAttenuation"+i+";\n";  
			shader=shader+"uniform float spotCosCutOff"+i+";\n";  
			shader=shader+"uniform float spotExp"+i+";\n";  
			shader=shader+"uniform vec3 lightdir"+i+";\n";  
			shader=shader+"uniform mat4 lightmat"+i+";\n";
			shader=shader+"uniform float shadowbias"+i+";\n"; 
			shader=shader+"uniform int shadowsamples"+i+";\n";  
			shader=shader+"uniform float shadowsoftness"+i+";\n";  
			shader=shader+"uniform bool castshadows"+i+";\n";  
			shader=shader+"varying vec4 spotcoord"+i+";\n";  
			if(lights[i].getCastShadows() && this.shadow){
				num=this.textures.length+(cnt++);
				shader=shader+"uniform sampler2D TEXTURE"+num+";\n";
				shadowlights[i]=num;
			}
	}
	for(i=0; i<this.layers.length;i++){		
		shader=shader+"varying vec3 textureCoords"+i+";\n";
		shader=shader+"uniform float layeralpha"+i+";\n";
		if((this.layers[i].mapto & GLGE.M_HEIGHT) == GLGE.M_HEIGHT){
			shader=shader+"uniform float layerheight"+i+";\n";
		}
	}
	
	shader=shader+"uniform vec4 baseColor;\n";
	shader=shader+"uniform vec3 specColor;\n";
	shader=shader+"uniform float shine;\n";
	shader=shader+"uniform float specular;\n";
	shader=shader+"uniform float reflective;\n";
	shader=shader+"uniform float emit;\n";
	shader=shader+"uniform float alpha;\n";
	shader=shader+"uniform vec3 amb;\n";
	shader=shader+"uniform float fognear;\n";
	shader=shader+"uniform float fogfar;\n";
	shader=shader+"uniform int fogtype;\n";
	shader=shader+"uniform vec3 fogcolor;\n";
	shader=shader+"uniform float far;\n";
	shader=shader+"uniform mat4 worldInverseTranspose;\n"; 
	shader=shader+"uniform mat4 projection;\n"; 
    
	shader=shader+"void main(void)\n";
	shader=shader+"{\n";
	shader=shader+"float att;\n"; 
	shader=shader+"int texture;\n"; 
	shader=shader+"float mask=1.0;\n";
	shader=shader+"float spec=specular;\n"; 
	shader=shader+"vec3 specC=specColor;\n"; 
	shader=shader+"vec4 view;\n"; 
	shader=shader+"vec3 textureCoords=vec3(0.0,0.0,0.0);\n"; 
	shader=shader+"float ref=reflective;\n";
	shader=shader+"float sh=shine;\n"; 
	shader=shader+"float em=emit;\n"; 
	shader=shader+"float al=alpha;\n"; 
	shader=shader+"vec4 normalmap= vec4(n,0.0);\n"
	shader=shader+"vec4 color = baseColor;"; //set the initial color
	shader=shader+"float pheight=0.0;\n"
	shader=shader+"vec3 textureHeight=vec3(0.0,0.0,0.0);\n";
	for(i=0; i<this.layers.length;i++){
		shader=shader+"textureCoords=textureCoords"+i+"+textureHeight;\n";
		shader=shader+"mask=layeralpha"+i+"*mask;\n";
		
		if(this.layers[i].mapinput==GLGE.MAP_VIEW){
			//will need to do in fragment to take the normal maps into account!
			shader=shader+"view=projection * vec4(-eyevec,1.0);\n";
			shader=shader+"textureCoords=view.xyz/view.w*0.5+0.5;\n";
			shader=shader+"textureCoords=textureCoords+textureHeight;\n";
		}
			
		if(this.layers[i].getTexture().className=="Texture" || this.layers[i].getTexture().className=="TextureCanvas"  || this.layers[i].getTexture().className=="TextureVideo" ){
			var txcoord="xy";
			var sampletype="2D";
		}else{
			var txcoord="xyz";
			var sampletype="Cube";
		}
		
		if((this.layers[i].mapto & GLGE.M_COLOR) == GLGE.M_COLOR){			
			if(this.layers[i].blendMode==GLGE.BL_MUL){
				shader=shader+"color = color*(1.0-mask) + color*texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+")*mask;\n";
			}
			else 
			{
				shader=shader+"color = color*(1.0-mask) + texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+")*mask;\n";
			}
		}        
		
		if((this.layers[i].mapto & GLGE.M_HEIGHT) == GLGE.M_HEIGHT){
			//do paralax stuff
			shader=shader+"pheight = texture2D(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").x;\n";
			shader=shader+"textureHeight =vec3((layerheight"+i+"* (pheight-0.5)  * normalize(teyevec).xy*vec2(1.0,-1.0)),0.0);\n";
		}
		if((this.layers[i].mapto & GLGE.M_SPECCOLOR) == GLGE.M_SPECCOLOR){
			shader=shader+"specC = specC*(1.0-mask) + texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").rgb*mask;\n";
		}
		if((this.layers[i].mapto & GLGE.M_MSKR) == GLGE.M_MSKR){
			shader=shader+"mask = texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").r;\n";
		}
		if((this.layers[i].mapto & GLGE.M_MSKG) == GLGE.M_MSKG){
			shader=shader+"mask = texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").g;\n";
		}
		if((this.layers[i].mapto & GLGE.M_MSKB) == GLGE.M_MSKB){
			shader=shader+"mask = texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").b;\n";
		}
		if((this.layers[i].mapto & GLGE.M_MSKA) == GLGE.M_MSKA){
			shader=shader+"mask = texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").a;\n";
		}
		if((this.layers[i].mapto & GLGE.M_SPECULAR) == GLGE.M_SPECULAR){
			shader=shader+"spec = spec*(1.0-mask) + texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").r*mask;\n";
		}
		if((this.layers[i].mapto & GLGE.M_REFLECT) == GLGE.M_REFLECT){
			shader=shader+"ref = ref*(1.0-mask) + texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").g*mask;\n";
		}
		if((this.layers[i].mapto & GLGE.M_SHINE) == GLGE.M_SHINE){
			shader=shader+"sh = sh*(1.0-mask) + texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").b*mask*255.0;\n";
		}
		if((this.layers[i].mapto & GLGE.M_EMIT) == GLGE.M_EMIT){
			shader=shader+"em = em*(1.0-mask) + texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").r*mask;\n";
		}
		if((this.layers[i].mapto & GLGE.M_NOR) == GLGE.M_NOR){
			shader=shader+"normalmap = normalmap*(1.0-mask) + texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+")*mask;\n";
			tangent=true;
		}
		if((this.layers[i].mapto & GLGE.M_ALPHA) == GLGE.M_ALPHA){
			shader=shader+"al = al*(1.0-mask) + texture"+sampletype+"(TEXTURE"+this.layers[i].texture.idx+", textureCoords."+txcoord+").a*mask;\n";
		}
	}		
	if(tangent){
		shader=shader+"vec3 normal = normalize(normalmap.rgb)*2.0-1.0;\n";
	}else{
		shader=shader+"vec3 normal = normalize(n);\n";
	}

	shader=shader+"vec3 lightvalue=amb;\n"; 
	shader=shader+"vec3 specvalue=vec3(0.0,0.0,0.0);\n"; 
	shader=shader+"float dotN,spotEffect;";
	shader=shader+"vec3 lightvec=vec3(0.0,0.0,0.0);";
	shader=shader+"vec3 viewvec=vec3(0.0,0.0,0.0);";
	shader=shader+"float spotmul=0.0;";
	shader=shader+"float spotsampleX=0.0;";
	shader=shader+"float spotsampleY=0.0;";
	shader=shader+"float totalweight=0.0;";
	shader=shader+"int cnt=0;";
	shader=shader+"vec2 spotoffset=vec2(0.0,0.0);";
	for(var i=0; i<lights.length;i++){
	
		if(tangent){
			shader=shader+"lightvec=tlightvec"+i+"*vec3(-1.0,-1.0,1.0);\n";  
			shader=shader+"normal.z=(normal.z+1.0)/2.0;\n";  
			shader=shader+"viewvec=teyevec*vec3(-1.0,-1.0,1.0);\n";  
		}else{
			shader=shader+"lightvec=lightvec"+i+";\n";  
			shader=shader+"viewvec=eyevec;\n"; 
		}
		
		if(lights[i].type==GLGE.L_POINT){ 
			shader=shader+"dotN=max(dot(normal,normalize(-lightvec)),0.0);\n";       
			shader=shader+"if(dotN>0.0){\n";
			shader=shader+"att = 1.0 / (lightAttenuation"+i+"[0] + lightAttenuation"+i+"[1] * lightdist"+i+" + lightAttenuation"+i+"[2] * lightdist"+i+" * lightdist"+i+");\n";
			if(lights[i].diffuse){
				shader=shader+"lightvalue += att * dotN * lightcolor"+i+";\n";
			}
			if(lights[i].specular){
				shader=shader+"specvalue += att * specC * lightcolor"+i+" * spec  * pow(max(dot(reflect(normalize(lightvec), normal),normalize(viewvec)),0.0), sh);\n";
			}
			
			shader=shader+"}\n";
			
			
		}
		shader=shader+"spotEffect = 0.0;\n";
		if(lights[i].type==GLGE.L_SPOT){
			shader=shader+"spotEffect = dot(normalize(lightdir"+i+"), normalize(-lightvec"+i+"));";	
			shader=shader+"if (spotEffect > spotCosCutOff"+i+") {\n";		
			shader=shader+"spotEffect = pow(spotEffect, spotExp"+i+");";
			//spot shadow stuff
			if(lights[i].getCastShadows() && this.shadow){
				shader=shader+"if(castshadows"+i+"){\n";
				shader=shader+"vec4 dist=texture2D(TEXTURE"+shadowlights[i]+", (((spotcoord"+i+".xy)/spotcoord"+i+".w)+1.0)/2.0);\n";
				shader=shader+"float depth = dot(dist, vec4(0.000000059604644775390625,0.0000152587890625,0.00390625,1.0))*10000.0;\n";
				shader=shader+"spotmul=0.0;\n";
				shader=shader+"totalweight=0.0;\n";
				shader=shader+"if((depth+shadowbias"+i+"-length(lightvec"+i+"))<0.0) {spotmul=1.0; totalweight=1.0;}\n";
				shader=shader+"if(shadowsamples"+i+">0){\n";
					shader=shader+"for(cnt=0; cnt<4; cnt++){;\n";
						shader=shader+"spotsampleX=-0.707106781;spotsampleY=-0.707106781;\n"; 
						shader=shader+"if(cnt==0 || cnt==3) spotsampleX=0.707106781;\n"; 
						shader=shader+"if(cnt==1 || cnt==3) spotsampleY=0.707106781;\n"; 
						shader=shader+"spotoffset=vec2(spotsampleX,spotsampleY)*0.5;\n";
						shader=shader+"dist=texture2D(TEXTURE"+shadowlights[i]+", (((spotcoord"+i+".xy)/spotcoord"+i+".w)+1.0)/2.0+spotoffset*shadowsoftness"+i+");\n";
						shader=shader+"depth = dot(dist, vec4(0.000000059604644775390625,0.0000152587890625,0.00390625,1.0))*100.0;\n";
						shader=shader+"if((depth+shadowbias"+i+"-length(lightvec"+i+"))<0.0){\n";
						shader=shader+"spotmul+=length(spotoffset);\n";
						shader=shader+"}\n";
						shader=shader+"totalweight+=length(spotoffset);\n";
					shader=shader+"};\n";
				shader=shader+"};\n";
				shader=shader+"if(totalweight!=spotmul){\n";
					shader=shader+"spotmul=0.0;\n";
					shader=shader+"totalweight=0.0;\n";
					shader=shader+"for(cnt=0; cnt<shadowsamples"+i+"*2; cnt++){;\n";
						shader=shader+"spotsampleX=(fract(sin(dot(spotcoord"+i+".xy + vec2(float(cnt)),vec2(12.9898,78.233))) * 43758.5453)-0.5)*2.0;\n"; //generate random number
						shader=shader+"spotsampleY=(fract(sin(dot(spotcoord"+i+".yz + vec2(float(cnt)),vec2(12.9898,78.233))) * 43758.5453)-0.5)*2.0;\n"; //generate random number
						shader=shader+"spotoffset=vec2(spotsampleX,spotsampleY);\n";
						shader=shader+"dist=texture2D(TEXTURE"+shadowlights[i]+", (((spotcoord"+i+".xy)/spotcoord"+i+".w)+1.0)/2.0+spotoffset*shadowsoftness"+i+");\n";
						shader=shader+"depth = dot(dist, vec4(0.000000059604644775390625,0.0000152587890625,0.00390625,1.0))*100.0;\n";
						shader=shader+"if((depth+shadowbias"+i+"-length(lightvec"+i+"))<0.0){\n";
						shader=shader+"spotmul+=length(spotoffset);\n";
						shader=shader+"}\n";
						shader=shader+"totalweight+=length(spotoffset);\n";
					shader=shader+"};\n";
				shader=shader+"}\n";
				
				shader=shader+"if(totalweight>0.0) spotEffect=spotEffect*pow(1.0-spotmul/totalweight,3.0);\n";
				shader=shader+"}";
			}

			
			shader=shader+"dotN=max(dot(normal,normalize(-lightvec)),0.0);\n";       
			shader=shader+"if(dotN>0.0){\n";
			shader=shader+"att = spotEffect / (lightAttenuation"+i+"[0] + lightAttenuation"+i+"[1] * lightdist"+i+" + lightAttenuation"+i+"[2] * lightdist"+i+" * lightdist"+i+");\n";
			if(lights[i].diffuse){
				shader=shader+"lightvalue += att * dotN * lightcolor"+i+";\n";
			}
			if(lights[i].specular){
				shader=shader+"specvalue += att * specC * lightcolor"+i+" * spec  * pow(max(dot(reflect(normalize(lightvec), normal),normalize(viewvec)),0.0), sh);\n";
			}
			shader=shader+"}\n}\n";
		}
		if(lights[i].type==GLGE.L_DIR){
			shader=shader+"dotN=max(dot(normal,-normalize(lightvec)),0.0);\n";    
			shader=shader+"if(dotN>0.0){\n";			
			if(lights[i].diffuse){
				shader=shader+"lightvalue += dotN * lightcolor"+i+";\n";
			}
			if(lights[i].specular){
				shader=shader+"specvalue += specC * lightcolor"+i+" * spec  * pow(max(dot(reflect(normalize(lightvec), normal),normalize(viewvec)),0.0), sh);\n";
			}
			shader=shader+"}\n";
		}
	}
	shader=shader+"float fogfact=1.0;";
	shader=shader+"if(fogtype=="+GLGE.FOG_QUADRATIC+") fogfact=clamp(pow(max((fogfar - length(eyevec)) / (fogfar - fognear),0.0),2.0),0.0,1.0);\n";
	shader=shader+"if(fogtype=="+GLGE.FOG_LINEAR+") fogfact=clamp((fogfar - length(eyevec)) / (fogfar - fognear),0.0,1.0);\n";
	
	shader=shader+"lightvalue = (lightvalue)*ref;\n";
	shader=shader+"if(em>0.0){lightvalue=vec3(1.0,1.0,1.0);  fogfact=1.0;}\n";
	shader=shader+"gl_FragColor =vec4(specvalue.rgb+color.rgb*(em+1.0)*lightvalue.rgb,al)*fogfact+vec4(fogcolor,al)*(1.0-fogfact);\n";
	//shader=shader+"gl_FragColor =texture2D(TEXTURE"+shadowlights[0]+", (((spotcoord0.xy)/spotcoord"+i+".w)+1.0)/2.0+textureHeight);\n";

	shader=shader+"}\n";
	return shader;
};
/**
* Set the uniforms needed to render this material
* @private
*/
GLGE.Material.prototype.textureUniforms=function(gl,shaderProgram,lights,object){
	if(this.animation) this.animate();
	if(shaderProgram.caches.baseColor!=this.color){
		gl.uniform4f(GLGE.getUniformLocation(gl,shaderProgram, "baseColor"), this.color.r,this.color.g,this.color.b,this.color.a);
		shaderProgram.caches.baseColor=this.color;
	}
	if(shaderProgram.caches.specColor!=this.specColor){
		gl.uniform3f(GLGE.getUniformLocation(gl,shaderProgram, "specColor"), this.specColor.r,this.specColor.g,this.specColor.b);
		shaderProgram.caches.specColor=this.specColor;
	}
	if(shaderProgram.caches.specular!=this.specular){
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "specular"), this.specular);
		shaderProgram.caches.specular=this.specular;
	}
	if(shaderProgram.caches.shine!=this.shine){
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "shine"), this.shine);
		shaderProgram.caches.shine=this.shine;
	}
	if(shaderProgram.caches.reflect!=this.reflect){
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "reflective"), this.reflect);
		shaderProgram.caches.reflect=this.reflect;
	}
	if(shaderProgram.caches.emit!=this.emit){
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "emit"), this.emit);
		shaderProgram.caches.emit=this.emit;
	}
	if(shaderProgram.caches.alpha!=this.alpha){
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "alpha"), this.alpha);
		shaderProgram.caches.alpha=this.alpha;
	}
	
	var cnt=0;
	var num=0;
	for(var i=0; i<lights.length;i++){
		gl.uniform3f(GLGE.getUniformLocation(gl,shaderProgram, "lightcolor"+i), lights[i].color.r,lights[i].color.g,lights[i].color.b);
		gl.uniform3f(GLGE.getUniformLocation(gl,shaderProgram, "lightAttenuation"+i), lights[i].constantAttenuation,lights[i].linearAttenuation,lights[i].quadraticAttenuation);
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "spotCosCutOff"+i), lights[i].spotCosCutOff);
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "spotExp"+i), lights[i].spotExponent);
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "shadowbias"+i), lights[i].shadowBias);
		gl.uniform1i(GLGE.getUniformLocation(gl,shaderProgram, "castshadows"+i), lights[i].castShadows);
		gl.uniform1i(GLGE.getUniformLocation(gl,shaderProgram, "shadowsamples"+i), lights[i].samples);
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "shadowsoftness"+i), lights[i].softness);
		    
		//shadow code
		if(lights[i].getCastShadows() && this.shadow && this.emit==0) {
			num=this.textures.length+(cnt++);
			gl.activeTexture(gl["TEXTURE"+num]);
			gl.bindTexture(gl.TEXTURE_2D, lights[i].texture);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
			gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
			gl.generateMipmap(gl.TEXTURE_2D);
		    
			gl.uniform1i(GLGE.getUniformLocation(gl,shaderProgram, "TEXTURE"+num), num);
		}
	
			
	}
	
	if(!shaderProgram.glarrays.layermat) shaderProgram.glarrays.layermat=[];
	

			
	var scale,offset;
	for(i=0; i<this.layers.length;i++){
		if(this.layers[i].animation) this.layers[i].animate();
		scale=this.layers[i].getScale();
		offset=this.layers[i].getOffset();		
		if(!shaderProgram.glarrays.layermat[i]) shaderProgram.glarrays.layermat[i]=new Float32Array(this.layers[i].getMatrix());
			else GLGE.mat4gl(this.layers[i].getMatrix(),shaderProgram.glarrays.layermat[i]);	
		
		try{gl.uniformMatrix4fv(GLGE.getUniformLocation(gl,shaderProgram, "layer"+i+"Matrix"), true, shaderProgram.glarrays.layermat[i]);}catch(e){}
		
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "layeralpha"+i), this.layers[i].getAlpha());
		gl.uniform1f(GLGE.getUniformLocation(gl,shaderProgram, "layerheight"+i), this.layers[i].getHeight());
	}
    
	for(var i=0; i<this.textures.length;i++){
		
			gl.activeTexture(gl["TEXTURE"+i]);
			if(this.textures[i].doTexture(gl,object)){
			}
			gl.uniform1i(GLGE.getUniformLocation(gl,shaderProgram, "TEXTURE"+i), i);
	}	
	
};
/**
* Adds a new texture to this material
* @param {String} image URL of the image to be used by the texture
* @return {Number} index of the new texture
*/
GLGE.Material.prototype.addTexture=function(texture){	
	if(typeof texture=="string")  texture=GLGE.Assets.get(texture);
	this.textures.push(texture);
	texture.idx=this.textures.length-1;
	this.fireEvent("shaderupdate",{});
	return this;
};
GLGE.Material.prototype.addTextureCube=GLGE.Material.prototype.addTexture;
GLGE.Material.prototype.addTextureCamera=GLGE.Material.prototype.addTexture;
GLGE.Material.prototype.addTextureCanvas=GLGE.Material.prototype.addTexture;
GLGE.Material.prototype.addTextureVideo=GLGE.Material.prototype.addTexture;




/**
* Closure Export
*/
function closure_export(){
if(GLGE.Message){
	GLGE["Message"]=GLGE.Message;
	GLGE.Message["parseMessage"]=GLGE.Message.parseMessage;
}

if(GLGE.Document){
	GLGE["Document"]=GLGE.Document
	GLGE.Document.prototype["getElementById"]=GLGE.Document.prototype.getElementById;
	GLGE.Document.prototype["getElement"]=GLGE.Document.prototype.getElement;
	GLGE.Document.prototype["load"]=GLGE.Document.prototype.load;
	GLGE.Document.prototype["loadDocument"]=GLGE.Document.prototype.loadDocument;
	GLGE.Document.prototype["onLoad"]=GLGE.Document.prototype.onLoad;
	GLGE.Document.prototype["addLoadListener"]=GLGE.Document.prototype.addLoadListener;
	GLGE.Document.prototype["removeLoadListener"]=GLGE.Document.prototype.removeLoadListener;
}


if(GLGE.Placeable){
	GLGE["Placeable"]=GLGE.Placeable;
	GLGE.Placeable.prototype["getRoot"]=GLGE.Placeable.prototype.getRoot;
	GLGE.Placeable.prototype["getRef"]=GLGE.Placeable.prototype.getRef;
	GLGE.Placeable.prototype["setId"]=GLGE.Placeable.prototype.setId;
	GLGE.Placeable.prototype["getId"]=GLGE.Placeable.prototype.getId;
	GLGE.Placeable.prototype["getLookat"]=GLGE.Placeable.prototype.getLookat;
	GLGE.Placeable.prototype["setLookat"]=GLGE.Placeable.prototype.setLookat;
	GLGE.Placeable.prototype["Lookat"]=GLGE.Placeable.prototype.Lookat;
	GLGE.Placeable.prototype["getRotOrder"]=GLGE.Placeable.prototype.getRotOrder;
	GLGE.Placeable.prototype["setRotOrder"]=GLGE.Placeable.prototype.setRotOrder;
	GLGE.Placeable.prototype["getRotMatrix"]=GLGE.Placeable.prototype.getRotMatrix;
	GLGE.Placeable.prototype["setRotMatrix"]=GLGE.Placeable.prototype.setRotMatrix;
	GLGE.Placeable.prototype["setLocX"]=GLGE.Placeable.prototype.setLocX;
	GLGE.Placeable.prototype["setLocY"]=GLGE.Placeable.prototype.setLocY;
	GLGE.Placeable.prototype["setLocZ"]=GLGE.Placeable.prototype.setLocZ;
	GLGE.Placeable.prototype["setLoc"]=GLGE.Placeable.prototype.setLoc;
	GLGE.Placeable.prototype["setDLocX"]=GLGE.Placeable.prototype.setDLocX;
	GLGE.Placeable.prototype["setDLocY"]=GLGE.Placeable.prototype.setDLocY;
	GLGE.Placeable.prototype["setDLocZ"]=GLGE.Placeable.prototype.setDLocZ;
	GLGE.Placeable.prototype["setDLoc"]=GLGE.Placeable.prototype.setDLoc;
	GLGE.Placeable.prototype["setQuatX"]=GLGE.Placeable.prototype.setQuatX;
	GLGE.Placeable.prototype["setQuatY"]=GLGE.Placeable.prototype.setQuatY;
	GLGE.Placeable.prototype["setQuatZ"]=GLGE.Placeable.prototype.setQuatZ;
	GLGE.Placeable.prototype["setQuatW"]=GLGE.Placeable.prototype.setQuatW;
	GLGE.Placeable.prototype["setQuat"]=GLGE.Placeable.prototype.setQuat;
	GLGE.Placeable.prototype["setRotX"]=GLGE.Placeable.prototype.setRotX;
	GLGE.Placeable.prototype["setRotY"]=GLGE.Placeable.prototype.setRotY;
	GLGE.Placeable.prototype["setRotZ"]=GLGE.Placeable.prototype.setRotZ;
	GLGE.Placeable.prototype["setRot"]=GLGE.Placeable.prototype.setRot;
	GLGE.Placeable.prototype["setDRotX"]=GLGE.Placeable.prototype.setDRotX;
	GLGE.Placeable.prototype["setDRotY"]=GLGE.Placeable.prototype.setDRotY;
	GLGE.Placeable.prototype["setDRotZ"]=GLGE.Placeable.prototype.setDRotZ;
	GLGE.Placeable.prototype["setDRot"]=GLGE.Placeable.prototype.setDRot;
	GLGE.Placeable.prototype["setScaleX"]=GLGE.Placeable.prototype.setScaleX;
	GLGE.Placeable.prototype["setScaleY"]=GLGE.Placeable.prototype.setScaleY;
	GLGE.Placeable.prototype["setScaleZ"]=GLGE.Placeable.prototype.setScaleZ;
	GLGE.Placeable.prototype["setScale"]=GLGE.Placeable.prototype.setScale;
	GLGE.Placeable.prototype["setDScaleX"]=GLGE.Placeable.prototype.setDScaleX;
	GLGE.Placeable.prototype["setDScaleY"]=GLGE.Placeable.prototype.setDScaleY;
	GLGE.Placeable.prototype["setDScaleZ"]=GLGE.Placeable.prototype.setDScaleZ;
	GLGE.Placeable.prototype["setDScale"]=GLGE.Placeable.prototype.setDScale;
	GLGE.Placeable.prototype["getLocX"]=GLGE.Placeable.prototype.getLocX;
	GLGE.Placeable.prototype["getLocY"]=GLGE.Placeable.prototype.getLocY;
	GLGE.Placeable.prototype["getLocZ"]=GLGE.Placeable.prototype.getLocZ;
	GLGE.Placeable.prototype["getDLocX"]=GLGE.Placeable.prototype.getDLocX;
	GLGE.Placeable.prototype["getDLocY"]=GLGE.Placeable.prototype.getDLocY;
	GLGE.Placeable.prototype["getDLocZ"]=GLGE.Placeable.prototype.getDLocZ;
	GLGE.Placeable.prototype["getQuatX"]=GLGE.Placeable.prototype.getQuatX;
	GLGE.Placeable.prototype["getQuatY"]=GLGE.Placeable.prototype.getQuatY;
	GLGE.Placeable.prototype["getQuatZ"]=GLGE.Placeable.prototype.getQuatZ;
	GLGE.Placeable.prototype["getQuatW"]=GLGE.Placeable.prototype.getQuatW;
	GLGE.Placeable.prototype["getRotX"]=GLGE.Placeable.prototype.getRotX;
	GLGE.Placeable.prototype["getRotY"]=GLGE.Placeable.prototype.getRotY;
	GLGE.Placeable.prototype["getRotZ"]=GLGE.Placeable.prototype.getRotZ;
	GLGE.Placeable.prototype["getDRotX"]=GLGE.Placeable.prototype.getDRotX;
	GLGE.Placeable.prototype["getDRotY"]=GLGE.Placeable.prototype.getDRotY;
	GLGE.Placeable.prototype["getDRotZ"]=GLGE.Placeable.prototype.getDRotZ;
	GLGE.Placeable.prototype["getScaleX"]=GLGE.Placeable.prototype.getScaleX;
	GLGE.Placeable.prototype["getScaleY"]=GLGE.Placeable.prototype.getScaleY;
	GLGE.Placeable.prototype["getScaleZ"]=GLGE.Placeable.prototype.getScaleZ;
	GLGE.Placeable.prototype["getDScaleX"]=GLGE.Placeable.prototype.getDScaleX;
	GLGE.Placeable.prototype["getDScaleY"]=GLGE.Placeable.prototype.getDScaleY;
	GLGE.Placeable.prototype["getDScaleZ"]=GLGE.Placeable.prototype.getDScaleZ;
	GLGE.Placeable.prototype["getPosition"]=GLGE.Placeable.prototype.getPosition;
	GLGE.Placeable.prototype["getRotation"]=GLGE.Placeable.prototype.getRotation;
	GLGE.Placeable.prototype["getScale"]=GLGE.Placeable.prototype.getScale;
	GLGE.Placeable.prototype["getModelMatrix"]=GLGE.Placeable.prototype.getModelMatrix;
}

if(GLGE.Animatable){
	GLGE["Animatable"]=GLGE.Animatable;
	GLGE.Animatable.prototype["animationStart"]=GLGE.Animatable.prototype.animationStart;
	GLGE.Animatable.prototype["animate"]=GLGE.Animatable.prototype.animate;
	GLGE.Animatable.prototype["setAnimation"]=GLGE.Animatable.prototype.setAnimation;
	GLGE.Animatable.prototype["getAnimation"]=GLGE.Animatable.prototype.getAnimation;
	GLGE.Animatable.prototype["setFrameRate"]=GLGE.Animatable.prototype.setFrameRate;
	GLGE.Animatable.prototype["getFrameRate"]=GLGE.Animatable.prototype.getFrameRate;
	GLGE.Animatable.prototype["setLoop"]=GLGE.Animatable.prototype.setLoop;
	GLGE.Animatable.prototype["getLoop"]=GLGE.Animatable.prototype.getLoop;
	GLGE.Animatable.prototype["isLooping"]=GLGE.Animatable.prototype.isLooping;
	GLGE.Animatable.prototype["setPaused"]=GLGE.Animatable.prototype.setPaused;
	GLGE.Animatable.prototype["getPaused"]=GLGE.Animatable.prototype.getPaused;
	GLGE.Animatable.prototype["togglePaused"]=GLGE.Animatable.prototype.togglePaused;
}

if(GLGE.BezTriple){
	GLGE["BezTriple"]=GLGE.BezTriple;
	GLGE.BezTriple.prototype["className"]=GLGE.BezTriple.prototype.className;
	GLGE.BezTriple.prototype["setX1"]=GLGE.BezTriple.prototype.setX1;
	GLGE.BezTriple.prototype["setY1"]=GLGE.BezTriple.prototype.setY1;
	GLGE.BezTriple.prototype["setX2"]=GLGE.BezTriple.prototype.setX2;
	GLGE.BezTriple.prototype["setY2"]=GLGE.BezTriple.prototype.setY2;
	GLGE.BezTriple.prototype["setX3"]=GLGE.BezTriple.prototype.setX3;
	GLGE.BezTriple.prototype["setY4"]=GLGE.BezTriple.prototype.setY4;
}

if(GLGE.LinearPoint){
	GLGE["LinearPoint"]=GLGE.LinearPoint;
	GLGE.LinearPoint.prototype["className"]=GLGE.LinearPoint.prototype.className;
	GLGE.LinearPoint.prototype["setX"]=GLGE.LinearPoint.prototype.setX;
	GLGE.LinearPoint.prototype["setY"]=GLGE.LinearPoint.prototype.setY;
}

if(GLGE.StepPoint){
	GLGE["StepPoint"]=GLGE.StepPoint;
	//missing here?
}

if(GLGE.AnimationCurve){
	GLGE["AnimationCurve"]=GLGE.AnimationCurve;
	GLGE.AnimationCurve.prototype["className"]=GLGE.AnimationCurve.prototype.className;
	GLGE.AnimationCurve.prototype["addPoint"]=GLGE.AnimationCurve.prototype.addPoint;
	GLGE.AnimationCurve.prototype["getValue"]=GLGE.AnimationCurve.prototype.getValue;
}

if(GLGE.AnimationVector){
	GLGE["AnimationVector"]=GLGE.AnimationVector;
	GLGE.AnimationVector.prototype["addCurve"]=GLGE.AnimationVector.prototype.addCurve;
	GLGE.AnimationVector.prototype["removeCurve"]=GLGE.AnimationVector.prototype.removeCurve;
	GLGE.AnimationVector.prototype["setFrames"]=GLGE.AnimationVector.prototype.setFrames;
	GLGE.AnimationVector.prototype["getFrames"]=GLGE.AnimationVector.prototype.getFrames;
}
if(GLGE.augment){
	GLGE["augment"]=GLGE.augment;
}

GLGE["G_NODE"]=GLGE.G_NODE;
GLGE["G_ROOT"]=GLGE.G_ROOT;

if(GLGE.Group){
	GLGE["Group"]=GLGE.Group;
	GLGE.Group.prototype["children"]=GLGE.Group.prototype.children;
	GLGE.Group.prototype["className"]=GLGE.Group.prototype.className;
	GLGE.Group.prototype["type"]=GLGE.Group.prototype.type;
	GLGE.Group.prototype["getObjects"]=GLGE.Group.prototype.getObjects;
	GLGE.Group.prototype["getLights"]=GLGE.Group.prototype.getLights;
	GLGE.Group.prototype["addChild"]=GLGE.Group.prototype.addChild;
	GLGE.Group.prototype["addObject"]=GLGE.Group.prototype.addObject;
	GLGE.Group.prototype["addGroup"]=GLGE.Group.prototype.addGroup;
	GLGE.Group.prototype["addText"]=GLGE.Group.prototype.addText;
	GLGE.Group.prototype["addSkeleton"]=GLGE.Group.prototype.addSkeleton;
	GLGE.Group.prototype["addLight"]=GLGE.Group.prototype.addLight;
	GLGE.Group.prototype["addCamera"]=GLGE.Group.prototype.addCamera;
	GLGE.Group.prototype["removeChild"]=GLGE.Group.prototype.removeChild;
	GLGE.Group.prototype["getChildren"]=GLGE.Group.prototype.getChildren;
}

if(GLGE.Text){
	GLGE["Text"]=GLGE.Text;
	GLGE.Text.prototype["className"]=GLGE.Text.prototype.className;
	GLGE.Text.prototype["getPickType"]=GLGE.Text.prototype.getPickType;
	GLGE.Text.prototype["setPickType"]=GLGE.Text.prototype.setPickType;
	GLGE.Text.prototype["getFont"]=GLGE.Text.prototype.getFont;
	GLGE.Text.prototype["setFont"]=GLGE.Text.prototype.setFont;
	GLGE.Text.prototype["getSize"]=GLGE.Text.prototype.getSize;
	GLGE.Text.prototype["setSize"]=GLGE.Text.prototype.setSize;
	GLGE.Text.prototype["getText"]=GLGE.Text.prototype.getText;
	GLGE.Text.prototype["setText"]=GLGE.Text.prototype.setText;
	GLGE.Text.prototype["setColor"]=GLGE.Text.prototype.setColor;
	GLGE.Text.prototype["setColorR"]=GLGE.Text.prototype.setColorR;
	GLGE.Text.prototype["setColorG"]=GLGE.Text.prototype.setColorG;
	GLGE.Text.prototype["setColorB"]=GLGE.Text.prototype.setColorB;
	GLGE.Text.prototype["getColor"]=GLGE.Text.prototype.getColor;
	GLGE.Text.prototype["setZtransparent"]=GLGE.Text.prototype.setZtransparent;
	GLGE.Text.prototype["isZtransparent"]=GLGE.Text.prototype.isZtransparent;
}

if(GLGE.MultiMaterial){
	GLGE["MultiMaterial"]=GLGE.MultiMaterial;
	GLGE.MultiMaterial.prototype["className"]=GLGE.MultiMaterial.prototype.className;
	GLGE.MultiMaterial.prototype["setMesh"]=GLGE.MultiMaterial.prototype.setMesh;
	GLGE.MultiMaterial.prototype["getMesh"]=GLGE.MultiMaterial.prototype.getMesh;
	GLGE.MultiMaterial.prototype["setMaterial"]=GLGE.MultiMaterial.prototype.setMaterial;
	GLGE.MultiMaterial.prototype["getMaterial"]=GLGE.MultiMaterial.prototype.getMaterial;
}

if(GLGE.Object instanceof Object){
	GLGE["Object"]=GLGE.Object;
	GLGE.Object.prototype["className"]=GLGE.Object.prototype.className;
	GLGE.Object.prototype["setZtransparent"]=GLGE.Object.prototype.setZtransparent;
	GLGE.Object.prototype["isZtransparent"]=GLGE.Object.prototype.isZtransparent;
	GLGE.Object.prototype["setSkeleton"]=GLGE.Object.prototype.setSkeleton;
	GLGE.Object.prototype["getSkeleton"]=GLGE.Object.prototype.getSkeleton;
	GLGE.Object.prototype["setMaterial"]=GLGE.Object.prototype.setMaterial;
	GLGE.Object.prototype["getMaterial"]=GLGE.Object.prototype.getMaterial;
	GLGE.Object.prototype["setMesh"]=GLGE.Object.prototype.setMesh;
	GLGE.Object.prototype["getMesh"]=GLGE.Object.prototype.getMesh;
	GLGE.Object.prototype["addMultiMaterial"]=GLGE.Object.prototype.addMultiMaterial;
	GLGE.Object.prototype["getMultiMaterials"]=GLGE.Object.prototype.getMultiMaterials;
}

if(GLGE.Mesh){
	GLGE["Mesh"]=GLGE.Mesh;
	GLGE.Mesh.prototype["className"]=GLGE.Mesh.prototype.className;
	GLGE.Mesh.prototype["setJoints"]=GLGE.Mesh.prototype.setJoints;
	GLGE.Mesh.prototype["setInvBindMatrix"]=GLGE.Mesh.prototype.setInvBindMatrix;
	GLGE.Mesh.prototype["setVertexJoints"]=GLGE.Mesh.prototype.setVertexJoints;
	GLGE.Mesh.prototype["setVertexWeights"]=GLGE.Mesh.prototype.setVertexWeights;
	GLGE.Mesh.prototype["setUV"]=GLGE.Mesh.prototype.setUV;
	GLGE.Mesh.prototype["setUV2"]=GLGE.Mesh.prototype.setUV2;
	GLGE.Mesh.prototype["setPositions"]=GLGE.Mesh.prototype.setPositions;
	GLGE.Mesh.prototype["setNormals"]=GLGE.Mesh.prototype.setNormals;
	GLGE.Mesh.prototype["setBuffer"]=GLGE.Mesh.prototype.setBuffer;
	GLGE.Mesh.prototype["setFaces"]=GLGE.Mesh.prototype.setFaces;
	GLGE.Mesh.prototype["addObject"]=GLGE.Mesh.prototype.addObject;
	GLGE.Mesh.prototype["removeObject"]=GLGE.Mesh.prototype.removeObject;
}

GLGE["L_POINT"]=GLGE.L_POINT;
GLGE["L_DIR"]=GLGE.L_DIR;
GLGE["L_SPOT"]=GLGE.L_SPOT;

if(GLGE.Light){
	GLGE["Light"]=GLGE.Light;
	GLGE.Light.prototype["className"]=GLGE.Light.prototype.className;
	GLGE.Light.prototype["getPMatrix"]=GLGE.Light.prototype.getPMatrix;
	GLGE.Light.prototype["setCastShadows"]=GLGE.Light.prototype.setCastShadows;
	GLGE.Light.prototype["getCastShadows"]=GLGE.Light.prototype.getCastShadows;
	GLGE.Light.prototype["setShadowBias"]=GLGE.Light.prototype.setShadowBias;
	GLGE.Light.prototype["getShadowBias"]=GLGE.Light.prototype.getShadowBias;
	GLGE.Light.prototype["setBufferWidth"]=GLGE.Light.prototype.setBufferWidth;
	GLGE.Light.prototype["getBufferHeight"]=GLGE.Light.prototype.getBufferHeight;
	GLGE.Light.prototype["setBufferHeight"]=GLGE.Light.prototype.setBufferHeight;
	GLGE.Light.prototype["getBufferWidth"]=GLGE.Light.prototype.getBufferWidth;
	GLGE.Light.prototype["setSpotCosCutOff"]=GLGE.Light.prototype.setSpotCosCutOff;
	GLGE.Light.prototype["getSpotCosCutOff"]=GLGE.Light.prototype.getSpotCosCutOff;
	GLGE.Light.prototype["setSpotExponent"]=GLGE.Light.prototype.setSpotExponent;
	GLGE.Light.prototype["getSpotExponent"]=GLGE.Light.prototype.getSpotExponent;
	GLGE.Light.prototype["getAttenuation"]=GLGE.Light.prototype.getAttenuation;
	GLGE.Light.prototype["setAttenuation"]=GLGE.Light.prototype.setAttenuation;
	GLGE.Light.prototype["setAttenuationConstant"]=GLGE.Light.prototype.setAttenuationConstant;
	GLGE.Light.prototype["setAttenuationLinear"]=GLGE.Light.prototype.setAttenuationLinear;
	GLGE.Light.prototype["setAttenuationQuadratic"]=GLGE.Light.prototype.setAttenuationQuadratic;
	GLGE.Light.prototype["setColor"]=GLGE.Light.prototype.setColor;
	GLGE.Light.prototype["setColorR"]=GLGE.Light.prototype.setColorR;
	GLGE.Light.prototype["setColorG"]=GLGE.Light.prototype.setColorG;
	GLGE.Light.prototype["setColorB"]=GLGE.Light.prototype.setColorB;
	GLGE.Light.prototype["getColor"]=GLGE.Light.prototype.getColor;
	GLGE.Light.prototype["getType"]=GLGE.Light.prototype.getType;
	GLGE.Light.prototype["setType"]=GLGE.Light.prototype.setType;
}

GLGE["C_PERSPECTIVE"]=GLGE.C_PERSPECTIVE;
GLGE["C_ORTHO"]=GLGE.C_ORTHO;

if(GLGE.Camera){
	GLGE["Camera"]=GLGE.Camera;
	GLGE.Camera.prototype["className"]=GLGE.Camera.prototype.className;
	GLGE.Camera.prototype["getOrthoScale"]=GLGE.Camera.prototype.getOrthoScale;
	GLGE.Camera.prototype["setOrthoScale"]=GLGE.Camera.prototype.setOrthoScale;
	GLGE.Camera.prototype["getFar"]=GLGE.Camera.prototype.getFar;
	GLGE.Camera.prototype["setFar"]=GLGE.Camera.prototype.setFar;
	GLGE.Camera.prototype["getNear"]=GLGE.Camera.prototype.getNear;
	GLGE.Camera.prototype["setNear"]=GLGE.Camera.prototype.setNear;
	GLGE.Camera.prototype["getType"]=GLGE.Camera.prototype.getType;
	GLGE.Camera.prototype["setType"]=GLGE.Camera.prototype.setType;
	GLGE.Camera.prototype["getFovY"]=GLGE.Camera.prototype.getFovY;
	GLGE.Camera.prototype["setFovY"]=GLGE.Camera.prototype.setFovY;
	GLGE.Camera.prototype["getAspect"]=GLGE.Camera.prototype.getAspect;
	GLGE.Camera.prototype["setAspect"]=GLGE.Camera.prototype.setAspect;
	GLGE.Camera.prototype["getProjectionMatrix"]=GLGE.Camera.prototype.getProjectionMatrix;
	GLGE.Camera.prototype["setProjectionMatrix"]=GLGE.Camera.prototype.setProjectionMatrix;
	GLGE.Camera.prototype["getViewMatrix"]=GLGE.Camera.prototype.getViewMatrix;
}

GLGE["FOG_NONE"]=GLGE.FOG_NONE;
GLGE["FOG_LINEAR"]=GLGE.FOG_LINEAR
GLGE["FOG_QUADRATIC"]=GLGE.FOG_QUADRATIC;

if(GLGE.Scene){
	GLGE["Scene"]=GLGE.Scene;
	GLGE.Scene.prototype["className"]=GLGE.Scene.prototype.className;
	GLGE.Scene.prototype["getFogType"]=GLGE.Scene.prototype.getFogType;
	GLGE.Scene.prototype["setFogType"]=GLGE.Scene.prototype.setFogType;
	GLGE.Scene.prototype["getFogFar"]=GLGE.Scene.prototype.getFogFar;
	GLGE.Scene.prototype["setFogFar"]=GLGE.Scene.prototype.setFogFar;
	GLGE.Scene.prototype["getFogNear"]=GLGE.Scene.prototype.getFogNear;
	GLGE.Scene.prototype["setFogNear"]=GLGE.Scene.prototype.setFogNear;
	GLGE.Scene.prototype["getFogColor"]=GLGE.Scene.prototype.getFogColor;
	GLGE.Scene.prototype["setFogColor"]=GLGE.Scene.prototype.setFogColor;
	GLGE.Scene.prototype["getBackgroundColor"]=GLGE.Scene.prototype.getBackgroundColor;
	GLGE.Scene.prototype["setBackgroundColor"]=GLGE.Scene.prototype.setBackgroundColor;
	GLGE.Scene.prototype["getAmbientColor"]=GLGE.Scene.prototype.getAmbientColor;
	GLGE.Scene.prototype["setAmbientColor"]=GLGE.Scene.prototype.setAmbientColor;
	GLGE.Scene.prototype["setAmbientColorR"]=GLGE.Scene.prototype.setAmbientColorR;
	GLGE.Scene.prototype["setAmbientColorG"]=GLGE.Scene.prototype.setAmbientColorG;
	GLGE.Scene.prototype["setAmbientColorB"]=GLGE.Scene.prototype.setAmbientColorB;
	GLGE.Scene.prototype["setCamera"]=GLGE.Scene.prototype.setCamera;
	GLGE.Scene.prototype["getCamera"]=GLGE.Scene.prototype.getCamera;
	GLGE.Scene.prototype["render"]=GLGE.Scene.prototype.render;
	GLGE.Scene.prototype["ray"]=GLGE.Scene.prototype.ray;
	GLGE.Scene.prototype["pick"]=GLGE.Scene.prototype.pick;
}

if(GLGE.Renderer){
	GLGE["Renderer"]=GLGE.Renderer;
	GLGE.Renderer.prototype["getScene"]=GLGE.Renderer.prototype.getScene;
	GLGE.Renderer.prototype["setScene"]=GLGE.Renderer.prototype.setScene;
	GLGE.Renderer.prototype["render"]=GLGE.Renderer.prototype.render;
}

if(GLGE.Texture){
	GLGE["Texture"]=GLGE.Texture;
	GLGE.Texture.prototype["className"]=GLGE.Texture.prototype.className;
	GLGE.Texture.prototype["getSrc"]=GLGE.Texture.prototype.getSrc;
	GLGE.Texture.prototype["setSrc"]=GLGE.Texture.prototype.setSrc;
}

if(GLGE.MaterialLayer){
	GLGE["MaterialLayer"]=GLGE.MaterialLayer;
	GLGE.MaterialLayer.prototype["className"]=GLGE.MaterialLayer.prototype.className;
	GLGE.MaterialLayer.prototype["getMatrix"]=GLGE.MaterialLayer.prototype.getMatrix;
	GLGE.MaterialLayer.prototype["setTexture"]=GLGE.MaterialLayer.prototype.setTexture;
	GLGE.MaterialLayer.prototype["getTexture"]=GLGE.MaterialLayer.prototype.getTexture;
	GLGE.MaterialLayer.prototype["setMapto"]=GLGE.MaterialLayer.prototype.setMapto;
	GLGE.MaterialLayer.prototype["getMapto"]=GLGE.MaterialLayer.prototype.getMapto;
	GLGE.MaterialLayer.prototype["setMapinput"]=GLGE.MaterialLayer.prototype.setMapinput;
	GLGE.MaterialLayer.prototype["getMapinput"]=GLGE.MaterialLayer.prototype.getMapinput;
	GLGE.MaterialLayer.prototype["getOffset"]=GLGE.MaterialLayer.prototype.getOffset;
	GLGE.MaterialLayer.prototype["getRotation"]=GLGE.MaterialLayer.prototype.getRotation;
	GLGE.MaterialLayer.prototype["getScale"]=GLGE.MaterialLayer.prototype.getScale;
	GLGE.MaterialLayer.prototype["setOffsetX"]=GLGE.MaterialLayer.prototype.setOffsetX;
	GLGE.MaterialLayer.prototype["getOffsetX"]=GLGE.MaterialLayer.prototype.getOffsetX;
	GLGE.MaterialLayer.prototype["setOffsetY"]=GLGE.MaterialLayer.prototype.setOffsetY;
	GLGE.MaterialLayer.prototype["getOffsetY"]=GLGE.MaterialLayer.prototype.getOffsetY;
	GLGE.MaterialLayer.prototype["setOffsetZ"]=GLGE.MaterialLayer.prototype.setOffsetZ;
	GLGE.MaterialLayer.prototype["getOffsetZ"]=GLGE.MaterialLayer.prototype.getOffsetZ;
	GLGE.MaterialLayer.prototype["setDOffsetX"]=GLGE.MaterialLayer.prototype.setDOffsetX;
	GLGE.MaterialLayer.prototype["getDOffsetX"]=GLGE.MaterialLayer.prototype.getDOffsetX;
	GLGE.MaterialLayer.prototype["setDOffsetY"]=GLGE.MaterialLayer.prototype.setDOffsetY;
	GLGE.MaterialLayer.prototype["getDOffsetY"]=GLGE.MaterialLayer.prototype.getDOffsetY;
	GLGE.MaterialLayer.prototype["setDOffsetZ"]=GLGE.MaterialLayer.prototype.setDOffsetZ;
	GLGE.MaterialLayer.prototype["getDOffsetZ"]=GLGE.MaterialLayer.prototype.getDOffsetZ;
	GLGE.MaterialLayer.prototype["setScaleX"]=GLGE.MaterialLayer.prototype.setScaleX;
	GLGE.MaterialLayer.prototype["getScaleX"]=GLGE.MaterialLayer.prototype.getScaleX;
	GLGE.MaterialLayer.prototype["setScaleY"]=GLGE.MaterialLayer.prototype.setScaleY;
	GLGE.MaterialLayer.prototype["getScaleY"]=GLGE.MaterialLayer.prototype.getScaleY;
	GLGE.MaterialLayer.prototype["setScaleZ"]=GLGE.MaterialLayer.prototype.setScaleZ;
	GLGE.MaterialLayer.prototype["getScaleZ"]=GLGE.MaterialLayer.prototype.getScaleZ;
	GLGE.MaterialLayer.prototype["setDScaleX"]=GLGE.MaterialLayer.prototype.setDScaleX;
	GLGE.MaterialLayer.prototype["getDScaleX"]=GLGE.MaterialLayer.prototype.getDScaleX;
	GLGE.MaterialLayer.prototype["setDScaleY"]=GLGE.MaterialLayer.prototype.setDScaleY;
	GLGE.MaterialLayer.prototype["getDScaleY"]=GLGE.MaterialLayer.prototype.getDScaleY;
	GLGE.MaterialLayer.prototype["setDScaleZ"]=GLGE.MaterialLayer.prototype.setDScaleZ;
	GLGE.MaterialLayer.prototype["getDScaleZ"]=GLGE.MaterialLayer.prototype.getDScaleZ;
	GLGE.MaterialLayer.prototype["setRotX"]=GLGE.MaterialLayer.prototype.setRotX;
	GLGE.MaterialLayer.prototype["getRotX"]=GLGE.MaterialLayer.prototype.getRotX;
	GLGE.MaterialLayer.prototype["setRotY"]=GLGE.MaterialLayer.prototype.setRotY;
	GLGE.MaterialLayer.prototype["getRotY"]=GLGE.MaterialLayer.prototype.getRotY;
	GLGE.MaterialLayer.prototype["setRotZ"]=GLGE.MaterialLayer.prototype.setRotZ;
	GLGE.MaterialLayer.prototype["getRotZ"]=GLGE.MaterialLayer.prototype.getRotZ;
	GLGE.MaterialLayer.prototype["setDRotX"]=GLGE.MaterialLayer.prototype.setDRotX;
	GLGE.MaterialLayer.prototype["getDRotX"]=GLGE.MaterialLayer.prototype.getDRotX;
	GLGE.MaterialLayer.prototype["setDRotY"]=GLGE.MaterialLayer.prototype.setDRotY;
	GLGE.MaterialLayer.prototype["getDRotY"]=GLGE.MaterialLayer.prototype.getDRotY;
	GLGE.MaterialLayer.prototype["setDRotZ"]=GLGE.MaterialLayer.prototype.setDRotZ;
	GLGE.MaterialLayer.prototype["getDRotZ"]=GLGE.MaterialLayer.prototype.getDRotZ;
	GLGE.MaterialLayer.prototype["setBlendMode"]=GLGE.MaterialLayer.prototype.setBlendMode;
	GLGE.MaterialLayer.prototype["getBlendMode"]=GLGE.MaterialLayer.prototype.getBlendMode;
}

GLGE["M_COLOR"]=GLGE.M_COLOR;
GLGE["M_NOR"]=GLGE.M_NOR;
GLGE["M_ALPHA"]=GLGE.M_ALPHA;
GLGE["M_SPECCOLOR"]=GLGE.M_SPECCOLOR;
GLGE["M_SPECULAR"]=GLGE.M_SPECULAR;
GLGE["M_SHINE"]=GLGE.M_SHINE;
GLGE["M_REFLECT"]=GLGE.M_REFLECT;
GLGE["M_EMIT"]=GLGE.M_EMIT;
GLGE["M_ALPHA"]=GLGE.M_ALPHA;
GLGE["M_MSKR"]=GLGE.M_MSKR;
GLGE["M_MSKG"]=GLGE.M_MSKG;
GLGE["M_MSKB"]=GLGE.M_MSKB;
GLGE["M_MSKA"]=GLGE.M_MSKA;
GLGE["M_HEIGHT"]=GLGE.M_HEIGHT;
GLGE["UV1"]=GLGE.UV1;
GLGE["UV2"]=GLGE.UV2;
GLGE["MAP_NORM"]=GLGE.MAP_NORM;
GLGE["MAP_OBJ"]=GLGE.MAP_OBJ;
GLGE["BL_MIX"]=GLGE.BL_MIX;
GLGE["BL_MUL"]=GLGE.BL_MUL;

if(GLGE.Material){
	GLGE["Material"]=GLGE.Material;
	GLGE.Material.prototype["className"]=GLGE.Material.prototype.className;
	GLGE.Material.prototype["setShadow"]=GLGE.Material.prototype.setShadow;
	GLGE.Material.prototype["getShadow"]=GLGE.Material.prototype.getShadow;
	GLGE.Material.prototype["setColor"]=GLGE.Material.prototype.setColor;
	GLGE.Material.prototype["setColorR"]=GLGE.Material.prototype.setColorR;
	GLGE.Material.prototype["setColorG"]=GLGE.Material.prototype.setColorG;
	GLGE.Material.prototype["setColorB"]=GLGE.Material.prototype.setColorB;
	GLGE.Material.prototype["getColor"]=GLGE.Material.prototype.getColor;
	GLGE.Material.prototype["setSpecularColor"]=GLGE.Material.prototype.setSpecularColor;
	GLGE.Material.prototype["getSpecularColor"]=GLGE.Material.prototype.getSpecularColor;
	GLGE.Material.prototype["setAlpha"]=GLGE.Material.prototype.setAlpha;
	GLGE.Material.prototype["getAlpha"]=GLGE.Material.prototype.getAlpha;
	GLGE.Material.prototype["setSpecular"]=GLGE.Material.prototype.setSpecular;
	GLGE.Material.prototype["getSpecular"]=GLGE.Material.prototype.getSpecular;
	GLGE.Material.prototype["setShininess"]=GLGE.Material.prototype.setShininess;
	GLGE.Material.prototype["getShininess"]=GLGE.Material.prototype.getShininess;
	GLGE.Material.prototype["setEmit"]=GLGE.Material.prototype.setEmit;
	GLGE.Material.prototype["getEmit"]=GLGE.Material.prototype.getEmit;
	GLGE.Material.prototype["setReflectivity"]=GLGE.Material.prototype.setReflectivity;
	GLGE.Material.prototype["getReflectivity"]=GLGE.Material.prototype.getReflectivity;
	GLGE.Material.prototype["addMaterialLayer"]=GLGE.Material.prototype.addMaterialLayer;
	GLGE.Material.prototype["getLayers"]=GLGE.Material.prototype.getLayers;
	GLGE.Material.prototype["addTexture"]=GLGE.Material.prototype.addTexture;
}
}
closure_export();


})(window["GLGE"]);


