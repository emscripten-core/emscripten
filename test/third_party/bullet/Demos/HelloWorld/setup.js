function startSimulation() {
  var i = 0;
  var interval = setInterval(function() {
    var pre = Date.now();
    Module._simulate();
    var delta = Date.now() - pre;
    var fps = (1000/delta).toFixed(2);
    console.log('frame ' + [i, fps, pre - startTime].join(', '));
    //for (var j = 0; j < 10; j++) Module._addBody();
    if (i === 30) clearInterval(interval);
    i++;
  }, 1);
}

var startTime = Date.now();
console.log('start at time 0 = ' + startTime);

