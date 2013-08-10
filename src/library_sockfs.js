mergeInto(LibraryManager.library, {
  $SOCKFS__postset: '__ATINIT__.push({ func: function() { SOCKFS.root = FS.mount(SOCKFS, {}, null); } });',
  $SOCKFS__deps: ['$FS'],
  $SOCKFS: {
    mount: function(mount) {
      var node = FS.createNode(null, '/', {{{ cDefine('S_IFDIR') }}} | 0777, 0);
      node.node_ops = SOCKFS.node_ops;
      node.stream_ops = SOCKFS.stream_ops;
      return node;
    },
    node_ops: {
    },
    stream_ops: {
    },
    websocket_sock_ops: {
    }
  }
});