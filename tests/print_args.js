// Print command-line arguments to script.
// Used in test_sanity.py

var print = print || console.log;
var args = [];

if (typeof process !== 'undefined') {
  // Use process global in node
  args = process.argv.slice(2);
} else {
  // Use arguments object for d8 and jsc
  args = Array.prototype.slice.call(arguments);
}

args.forEach(function (val, index, array) {
  print(index + ': ' + val);
});

