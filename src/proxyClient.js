
// proxy to/from worker

var worker = new Worker('{{{ filename }}}.js');

worker.onmessage = function(event) {
  var data = event.data;
  switch (data.target) {
    case 'window': {
      window[data.method]();
      break;
    }
    default: throw 'what?';
  }
};

