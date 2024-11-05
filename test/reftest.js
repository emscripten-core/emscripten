// Code for performing browser reftest where we compare generated canvas
// image with an expected output image.
//
// This code expects the reftestRebaseline global to exist.
// See make_reftest in test/common.py.

// We have some tests that don't perform rendering during `main` so
// the normal process of performing `doReftest` in `postRun` doesn't
// work.  These tests can delay the call to `doReftest` by calling
// `reftestBlock` and then calling `reftestUnblock` once they have
// done their rendering.
var reftestBlocked = false;

function reftestBlock() {
  reftestBlocked = true;
}

function reftestUnblock() {
  reftestBlocked = false;
  doReftest();
}

function doReftest() {
  if (reftestBlocked) return;
  if (doReftest.done) return;
  doReftest.done = true;
  var img = new Image();
  img.onload = () => {
    assert(img.width == Module.canvas.width, `Invalid width: ${Module.canvas.width}, should be ${img.width}`);
    assert(img.height == Module.canvas.height, `Invalid height: ${Module.canvas.height}, should be ${img.height}`);

    var canvas = document.createElement('canvas');
    canvas.width = img.width;
    canvas.height = img.height;
    var ctx = canvas.getContext('2d');
    ctx.drawImage(img, 0, 0);
    var expected = ctx.getImageData(0, 0, img.width, img.height).data;

    var actualUrl = Module.canvas.toDataURL();
    var actualImage = new Image();
    actualImage.onload = () => {
      /*
      document.body.appendChild(img); // for comparisons
      var div = document.createElement('div');
      div.innerHTML = '^=expected, v=actual';
      document.body.appendChild(div);
      document.body.appendChild(actualImage); // to grab it for creating the test reference
      */

      var actualCanvas = document.createElement('canvas');
      actualCanvas.width = actualImage.width;
      actualCanvas.height = actualImage.height;
      var actualCtx = actualCanvas.getContext('2d');
      actualCtx.drawImage(actualImage, 0, 0);
      var actual = actualCtx.getImageData(0, 0, actualImage.width, actualImage.height).data;

      var total = 0;
      var width = img.width;
      var height = img.height;
      for (var x = 0; x < width; x++) {
        for (var y = 0; y < height; y++) {
          total += Math.abs(expected[y*width*4 + x*4 + 0] - actual[y*width*4 + x*4 + 0]);
          total += Math.abs(expected[y*width*4 + x*4 + 1] - actual[y*width*4 + x*4 + 1]);
          total += Math.abs(expected[y*width*4 + x*4 + 2] - actual[y*width*4 + x*4 + 2]);
        }
      }
      // floor, to allow some margin of error for antialiasing
      var wrong = Math.floor(total / (img.width*img.height*3));
      if (wrong || reftestRebaseline) {
        // Generate a png of the actual rendered image and send it back
        // to the server.
        Module.canvas.toBlob((blob) => {
          sendFileToServer('actual.png', blob);
          reportResultToServer(wrong);
        })
      } else {
        reportResultToServer(wrong);
      }
    };
    actualImage.src = actualUrl;
  }
  img.src = 'expected.png';
};

function setupRefTest() {
  Module['postRun'] = doReftest;

  if (typeof WebGLClient !== 'undefined') {
    // trigger reftest from RAF as well, needed for workers where there is no pre|postRun on the main thread
    var realRAF = window.requestAnimationFrame;
    /** @suppress{checkTypes} */
    window.requestAnimationFrame = (func) => {
      return realRAF(() => {
        func();
        realRAF(doReftest);
      });
    };

    // trigger reftest from canvas render too, for workers not doing GL
    var realWOM = worker.onmessage;
    worker.onmessage = (event) => {
      realWOM(event);
      if (event.data.target === 'canvas' && event.data.op === 'render') {
        realRAF(doReftest);
      }
    };
  }
}

setupRefTest();
