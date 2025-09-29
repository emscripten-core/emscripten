#if ASSERTIONS

// "30.0.0" -> 300000
function humanReadableVersionToPacked(str) {
  str = str.split('-')[0]; // Remove any trailing part from e.g. "12.53.3-alpha"
  var vers = str.split('.').slice(0, 3);
  while(vers.length < 3) vers.push('00');
  vers = vers.map((n, i, arr) => n.padStart(2, '0'));
  return vers.join('');
}
// 300000 -> "30.0.0"
var packedVersionToHumanReadable = n => [n / 10000 | 0, (n / 100 | 0) % 100, n % 100].join('.');

var currentNodeVersion = process?.versions?.node ? humanReadableVersionToPacked(process.versions.node) : Infinity;
if (currentNodeVersion < {{{ MIN_NODE_VERSION }}}) throw new Error(`This emscripten-generated code requires node v${ packedVersionToHumanReadable({{{ MIN_NODE_VERSION }}}) } (detected v${packedVersionToHumanReadable(currentNodeVersion)})`);

var currentSafariVersion = navigator?.userAgent?.includes("Safari/") && navigator.userAgent.match(/Version\/(\d+\.?\d*\.?\d*)/) ? humanReadableVersionToPacked(navigator.userAgent.match(/Version\/(\d+\.?\d*\.?\d*)/)[1]) : Infinity;
if (currentSafariVersion < {{{ MIN_SAFARI_VERSION }}}) throw new Error(`This emscripten-generated code requires Safari v${ packedVersionToHumanReadable({{{ MIN_SAFARI_VERSION }}}) } (detected v${currentSafariVersion})`);

var currentFirefoxVersion = navigator?.userAgent?.match(/Firefox\/(\d+(?:\.\d+)?)/) ? parseFloat(navigator.userAgent.match(/Firefox\/(\d+(?:\.\d+)?)/)[1]) : Infinity;
if (currentFirefoxVersion < {{{ MIN_FIREFOX_VERSION }}}) throw new Error(`This emscripten-generated code requires Firefox v{{{ MIN_FIREFOX_VERSION }}} (detected v${currentFirefoxVersion})`);

var currentChromeVersion = navigator?.userAgent?.match(/Chrome\/(\d+(?:\.\d+)?)/) ? parseFloat(navigator.userAgent.match(/Chrome\/(\d+(?:\.\d+)?)/)[1]) : Infinity;
if (currentChromeVersion < {{{ MIN_CHROME_VERSION }}}) throw new Error(`This emscripten-generated code requires Chrome v{{{ MIN_CHROME_VERSION }}} (detected v${currentChromeVersion})`);

#endif
