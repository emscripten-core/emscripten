/**
 * @license
 * Copyright 2024 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

#if ASSERTIONS

(function() {
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

  var TARGET_NOT_SUPPORTED = {{{ TARGET_NOT_SUPPORTED }}};

  var currentNodeVersion = typeof process !== 'undefined' && process?.versions?.node ? humanReadableVersionToPacked(process.versions.node) : TARGET_NOT_SUPPORTED;
#if MIN_NODE_VERSION == TARGET_NOT_SUPPORTED
  if (currentNodeVersion < TARGET_NOT_SUPPORTED) {
    throw new Error('not compiled for this environment (did you build to HTML and try to run it not on the web, or set ENVIRONMENT to something - like node - and run it someplace else - like on the web?)');
  }
#endif
  if (currentNodeVersion < {{{ MIN_NODE_VERSION }}}) {
    throw new Error(`This emscripten-generated code requires node v${ packedVersionToHumanReadable({{{ MIN_NODE_VERSION }}}) } (detected v${packedVersionToHumanReadable(currentNodeVersion)})`);
  }

  var currentSafariVersion = typeof navigator !== 'undefined' && navigator?.userAgent?.includes("Safari/") && navigator.userAgent.match(/Version\/(\d+\.?\d*\.?\d*)/) ? humanReadableVersionToPacked(navigator.userAgent.match(/Version\/(\d+\.?\d*\.?\d*)/)[1]) : TARGET_NOT_SUPPORTED;
#if MIN_SAFARI_VERSION == TARGET_NOT_SUPPORTED
  if (currentSafariVersion < TARGET_NOT_SUPPORTED) {
    throw new Error(`This page was compiled without support for Safari browser. Pass -sMIN_SAFARI_VERSION=${currentSafariVersion} or lower to enable support for this browser.`);
  }
#endif
  if (currentSafariVersion < {{{ MIN_SAFARI_VERSION }}}) {
    throw new Error(`This emscripten-generated code requires Safari v${ packedVersionToHumanReadable({{{ MIN_SAFARI_VERSION }}}) } (detected v${currentSafariVersion})`);
  }

  var currentFirefoxVersion = typeof navigator !== 'undefined' && navigator?.userAgent?.match(/Firefox\/(\d+(?:\.\d+)?)/) ? parseFloat(navigator.userAgent.match(/Firefox\/(\d+(?:\.\d+)?)/)[1]) : TARGET_NOT_SUPPORTED;
#if MIN_FIREFOX_VERSION == TARGET_NOT_SUPPORTED
  if (currentFirefoxVersion < TARGET_NOT_SUPPORTED) {
    throw new Error(`This page was compiled without support for Firefox browser. Pass -sMIN_FIREFOX_VERSION=${currentFirefoxVersion} or lower to enable support for this browser.`);
  }
#endif
  if (currentFirefoxVersion < {{{ MIN_FIREFOX_VERSION }}}) {
    throw new Error(`This emscripten-generated code requires Firefox v{{{ MIN_FIREFOX_VERSION }}} (detected v${currentFirefoxVersion})`);
  }

  var currentChromeVersion = typeof navigator !== 'undefined' && navigator?.userAgent?.match(/Chrome\/(\d+(?:\.\d+)?)/) ? parseFloat(navigator.userAgent.match(/Chrome\/(\d+(?:\.\d+)?)/)[1]) : TARGET_NOT_SUPPORTED;
#if MIN_CHROME_VERSION == TARGET_NOT_SUPPORTED
  if (currentChromeVersion < TARGET_NOT_SUPPORTED) {
    throw new Error(`This page was compiled without support for Chrome browser. Pass -sMIN_CHROME_VERSION=${currentChromeVersion} or lower to enable support for this browser.`);
  }
#endif
  if (currentChromeVersion < {{{ MIN_CHROME_VERSION }}}) {
    throw new Error(`This emscripten-generated code requires Chrome v{{{ MIN_CHROME_VERSION }}} (detected v${currentChromeVersion})`);
  }
})();

#endif
