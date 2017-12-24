var defun = (function () { var a = 1; })();
var name = (function () { var a = 1; })();
var object = (function () { var a = 1; })();
var non_reserved = (function () { var a = 1; })();

function func_1() { var a = 1; }
function func_2() { var a = 1; }
function func_3() { var a = 1; }
function func_4() { var a = 1; }
function func_5() { var a = 1; }
function func_6() { var a = 1; }
function func_7() { var a = 1; }
function func_8() { var a = 1; }
function func_9() { var a = 1; }
function func_10() { var a = 1; }
function func_deleted() { var a = 1; }

var quotedObject = {
  "var": func_1,
  "defun": func_2,
  "function": func_3,
  "name": func_4,
  "non_reserved": func_5
};

var unquotedObject = {
  var: func_6,
  defun: func_7,
  function: func_8,
  name: func_9,
  non_reserved: func_10
};

var recursiveObject = {
  object: {
    func: function () {
      var a = 1;
    },
    object: {
      func: function () {
        var b = 1;
      }
    }
  }
};
