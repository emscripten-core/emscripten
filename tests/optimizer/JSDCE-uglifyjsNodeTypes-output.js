var defun = (function() {
})();
var name = (function() {
})();
var object = (function() {
})();
var non_reserved = (function() {
})();
function func_1() {
}
function func_2() {
}
function func_3() {
}
function func_4() {
}
function func_5() {
}
function func_6() {
}
function func_7() {
}
function func_8() {
}
function func_9() {
}
function func_10() {
}
var quotedObject = {
 "var": func_1,
 "defun": func_2,
 "function": func_3,
 "name": func_4,
 "non_reserved": func_5
};
var unquotedObject = {
 "var": func_6,
 defun: func_7,
 "function": func_8,
 name: func_9,
 non_reserved: func_10
};
var recursiveObject = {
 object: {
  func: (function() {
  }),
  object: {
   func: (function() {
   })
  }
 }
};
