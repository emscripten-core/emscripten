var canvas = document.getElementById('canvas');
if (canvas) canvas.removeAttribute('tabindex');
var div = document.createElement('div');
div.id = 'otherContent';
div.innerHTML = `
    <input type="button" id="b0" value="Default+None"> This simply requests fullscreen, use when no extra behavior is desirable. This is subject to a wild number of per-browser differences, see <a href="https://github.com/emscripten-core/emscripten/issues/2556">https://github.com/emscripten-core/emscripten/issues/2556</a>.<br />
    <input type="button" id="b1" value="Stretch"> Stretch the WebGL render target to cover the whole screen in pixel-perfect manner, in standard definition. <br />
    <input type="button" id="b2" value="Stretch+HiDPI"> Same as above, but use the actual native display resolution, instead of CSS resolution. <br />
    <input type="button" id="b3" value="Aspect"> Stretch the WebGL render target to cover the full screen, but retain aspect ratio. <br />
    <input type="button" id="b4" value="Aspect+HiDPI"> Same as above, but use the actual native display resolution, instead of CSS resolution.  <br />
    <input type="button" id="b5" value="Stretch+None"> Don't resize WebGL render target, and don't care about aspect ratio, simply stretch over the whole screen. (current Firefox and IE default behavior) <br />
    <input type="button" id="b6" value="Aspect+None"> Don't resize WebGL render target, but scale over the whole screen, preserving aspect ratio. <br />
    <input type="button" id="b15" value="Aspect+None+Nearest"> Same as above, but perform pixelated nearest-neighbor filtering instead of bilinear filtering. <br />
    <input type="button" id="b7" value="Center"> Don't resize the WebGL render target and don't scale the displayed content, but show it full screen. (current Chrome and Safari default behavior) <br />
<div>
    <br />
    <br />
    The Soft fullscreen modes are otherwise exactly like the above, except that they don't actually request fullscreen, but instead they present the canvas maximized in the client area of the page. This is more desirable in Firefox OS mobile packaged apps, where the application is already displayed full screen. Also it allows client desktop apps to use F11 for transitioning between fullscreen mode. <br />
    <input type="button" id="b8" value="Stretch"> <br />
    <input type="button" id="b9" value="Stretch+HiDPI"> <br />
    <input type="button" id="b10" value="Aspect"> <br />
    <input type="button" id="b11" value="Aspect+HiDPI"> <br />
    <input type="button" id="b12" value="Strech+None"> <br />
    <input type="button" id="b13" value="Aspect+None"> <br />
    <input type="button" id="b16" value="Aspect+None+Nearest"> <br />
    <input type="button" id="b14" value="Center">  <br />
</div>
`;
document.body.appendChild(div);
