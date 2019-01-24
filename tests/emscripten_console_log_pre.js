console.realLog = console.log;
console.realWarn = console.warn;
console.realError = console.error;

var logged = false;
var warned = false;

console.log = function(s) {
	console.realLog(s);
	if (s === 'Hello!') logged = true;
}

console.warn = function(s) {
	console.realWarn(s);
	if (s === 'Hello!' && logged) warned = true;
}

console.error = function(s) {
	console.realError(s);
	if (s === 'Hello!' && logged && warned) Module['testPassed'] = true;
}
