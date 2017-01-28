mergeInto(LibraryManager.library, {
  activeAttribs: 0,

  glEnableVertexAttribArray__deps: ['activeAttribs'],
  glEnableVertexAttribArray: function(i) {
    Module['print']('glEnableVertexAttribArray ' + i);
    _activeAttribs |= (1 << i);
  },

  glDisableVertexAttribArray__deps: ['activeAttribs'],
  glDisableVertexAttribArray: function(i) {
    Module['print']('glDisableVertexAttribArray ' + i);
    _activeAttribs &= ~(1 << i);
  },

  getActiveAttribs__deps: ['activeAttribs'],
  getActiveAttribs: function() {
    return _activeAttribs;
  }
});
