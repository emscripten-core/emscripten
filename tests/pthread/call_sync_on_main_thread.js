mergeInto(LibraryManager.library, {
  // Test accessing a DOM element on the main thread.
  // This function returns the inner text of the div by ID "status"
  // Because it accesses the DOM, it must be called on the main thread.
  getDomElementParentInnerHTML__proxy: 'sync',
  getDomElementParentInnerHTML__sig: 'viii',
  getDomElementParentInnerHTML: function(domElementId, dst, size) {
    var id = UTF8ToString(domElementId);
    var text = document.getElementById(id).parentElement.innerHTML;
    stringToUTF8(text, dst, size);
  },

  receivesAndReturnsAnInteger__proxy: 'sync',
  receivesAndReturnsAnInteger__sig: 'ii',
  receivesAndReturnsAnInteger: function(i) {
    return i + 42;
  },

  isThisInWorker: function() {
    return ENVIRONMENT_IS_WORKER;
  },

  isThisInWorkerOnMainThread__proxy: 'sync',
  isThisInWorkerOnMainThread__sig: 'i',
  isThisInWorkerOnMainThread: function() {
    return ENVIRONMENT_IS_WORKER;
  }
});
