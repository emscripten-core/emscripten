/*
 * Javascript implementation for the Jansson JSON parser.
 * Source: https://github.com/akheron/jansson
 *
 * Implemented and tested with:
 * https://github.com/akheron/jansson/commit/cf1074e70ea42a1dea4d6e57b2b29532049bcd28
 */

var LibraryJansson = {

    $JANSSON: {

      // Returns the node ID to which the pointer `nodePtr` points to
      getNodeIDByPtr: function(nodePtr) {
        if (nodePtr)
          return {{{ makeGetValue('nodePtr', '0', 'i32') }}};
        else
          return 0;
      },

      // Returns the Javascript node object which is referenced by `nodeID`
      getNodeByID: function(nodeID) {
        if (!g_json_context) {
          console.log("Jansson: No JSON context.");
          return null;
        }

        var node = g_json_context[nodeID];
        if (!node) {
          console.log("Jansson: Node with ID `" + nodeID + "` not found. Context has `" + g_json_context.length + "` nodes.");
          return null;
        }

        return node;
      },

      // Returns the Javascript node object to which is referenced by `nodePtr` (via node ID).
      getNodeByPtr: function(nodePtr) {
        var nodeID = JANSSON.getNodeIDByPtr(nodePtr);
        var node = JANSSON.getNodeByID(nodeID);
        return node;
      },

      // Returns the pointer to the `nodeID` of the node that is referenced by `nodeID` 
      getNodePtrByID: function(nodeID) {
        if (nodeID <= 0)
          return null;
        
        var node = JANSSON.getNodeByID(nodeID);

        if (!node)
          return null;

        return node['nodeIDPtr'];
      },

      // Adds a Javascript node object to the Javascript JSON context
      allocNode: function(name, node) {
        var id = g_json_context.length;
        g_json_context[id] = {};
        g_json_context[id]['name'] = name;
        g_json_context[id]['node'] = node;
        g_json_context[id]['nodeIDPtr'] = _malloc(32);
        {{{ makeSetValue('g_json_context[id][\'nodeIDPtr\']', '0', 'id', 'i32') }}}
        return id;
      },

      // Frees all memory that might have been allocated for a given node
      freeNode: function(node) {
        node['name'] = null;
        node['node'] = null;
        _free(node['nodeIDPtr']);
        _free(node['keyPtr']);
        _free(node['stringValuePtr']);
      },

      // Loads a string into the Javascript JSON context
      // Only the root node is loaded. Child nodes are loaded on demand via `loadChildNodes` method.
      load: function(string, flags, error) {
        // This is potentially a security problem.
        // TODO: Make sure everything is properly escaped
        var json_obj = JSON.parse(string);

        if (json_obj != null) {
          // The context is an array storing all child nodes.
          // These child nodes are added to the array on demand.
          // The root node is at index `1` (index `0` is intentionally `null`)
          g_json_context = [null];
          var nodeID = JANSSON.allocNode('root', json_obj);
          return JANSSON.getNodePtrByID(nodeID);
        } else {
          g_json_context = null;
          return null;
        }
      },

      // Loads the child nodes of a given node (if the node has any) into the Javascript JSON context.
      loadChildNodes: function(node) {        
        if (!node)
          return null;

        // Add all child nodes of the parent node to the context.
        // Consequently we can access these child nodes with an ID (pointer)
        // TODO: Here is room for performance improvements
        if (!node.begin) {
          var childNodeID = g_json_context.length;
          for(var childNode in node['node']) {
            var childNodeID = JANSSON.allocNode(childNode, node['node'][childNode]); 
            if (!node.begin) node.begin = childNodeID;
          };
          node.end = childNodeID;
        }

        return node;
      },

      // Returns the content type (object, array, string, number, null) of the node referenced by `nodePtr`
      type: function(nodePtr) {
        var node = JANSSON.getNodeByPtr(nodePtr);
        if (!node)
          return null;

        var content = node['node'];
        if (typeof(content) === 'object' && (content instanceof Array))
          return 'array'
        else
        {
          return typeof(content);
        }
      }

    },

    json_loads: function(string, flags, error) {
      return JANSSON.load(Pointer_stringify(string), flags, error);
    },

    json_loadb: function(buffer, buflen, flags, error) {
      return JANSSON.load(Pointer_stringify(buffer).substr(0, buflen), flags, error);
    },

    json_is_object: function(nodePtr) {
      return (JANSSON.type(nodePtr) === 'object');
    },

    json_is_array: function(nodePtr) {
      return (JANSSON.type(nodePtr) === 'array');
    },

    json_is_string: function(nodePtr) {
      return (JANSSON.type(nodePtr) === 'string');
    },

    json_is_integer: function(nodePtr) {
      return (JANSSON.type(nodePtr) === 'number');
    },

    json_is_real: function(nodePtr) {
      return (JANSSON.type(nodePtr) === 'number');
    },

    json_is_number: function(nodePtr) {
      return (JANSSON.type(nodePtr) === 'number');
    },

    json_is_null: function(nodePtr) {
      var nodeType = JANSSON.type(nodePtr);
      return (nodeType === 'undefined' || nodeType === 'null');
    },

    // Returns the pointer to the child node with the key `key` of the given parent node
    json_object_get: function(parentNodePtr, key) {
      var key = Pointer_stringify(key);
      var parentNode = JANSSON.getNodeByPtr(parentNodePtr);

      if (!parentNode)
        return null;

      JANSSON.loadChildNodes(parentNode);

      // Find the requested child node
      for (var i=parentNode.begin; i<=parentNode.end; i++) {
        if (g_json_context[i]['name'] === key)
          return JANSSON.getNodePtrByID(i);
      }
      
      return null;
    },

    // Returns the pointer to the first child node of the given parent node
    json_object_iter: function(parentNodePtr) {
      var parentNode = JANSSON.getNodeByPtr(parentNodePtr);

      if (!parentNode)
        return null;

      JANSSON.loadChildNodes(parentNode);

      return JANSSON.getNodePtrByID(parentNode.begin);
    },

    // Returns a pointer to the child node after the given child node of the given parent node
    json_object_iter_next: function(parentNodePtr, childNodePtr) {
      var parentNode = JANSSON.getNodeByPtr(parentNodePtr);
      var childNodeID = JANSSON.getNodeIDByPtr(childNodePtr);

      if (!parentNode)
        return null;

      if (childNodeID < parentNode.begin || childNodeID >= parentNode.end)
        return null;
      else
        return JANSSON.getNodePtrByID(childNodeID+1);
    },

    // Returns a integer containing the size of an array
    json_array_size: function(parentNodePtr) {
      var parentNode = JANSSON.getNodeByPtr(parentNodePtr);

      if (!parentNode)
        return 0;

      JANSSON.loadChildNodes(parentNode);

      var size = parentNode.end - parentNode.begin + 1;
      if (size < 0)
       size = 0;
      return size;
    },

    // Returns the pointer of the node with the index `index` in a given array parent node
    json_array_get: function(parentNodePtr, index) {
      var parentNode = JANSSON.getNodeByPtr(parentNodePtr);
      JANSSON.loadChildNodes(parentNode);

      var position = parentNode.begin + index;
      if (position < parentNode.begin || position > parentNode.end)
        return null;
      else
        return JANSSON.getNodePtrByID(position);
    },

    json_object_iter_key: function(nodePtr) {
      var node = JANSSON.getNodeByPtr(nodePtr);
      if (!node)
        return null;

      if (!node['keyPtr']) {
        node['keyPtr'] = allocate(intArrayFromString(node['name']), 'i8', ALLOC_NORMAL);
      }

      return node['keyPtr'];
    },

    json_object_iter_value: function(nodePtr) {
      return nodePtr;
    },

    json_string_value: function(nodePtr) {
      var node = JANSSON.getNodeByPtr(nodePtr);

      if (!node)
      {
        console.log("Jansson: invalid node in `function json_string_value`");
        return null;
      }
      
      if (!node['stringValuePtr']) {
        node['stringValuePtr'] = allocate(intArrayFromString(node['node']), 'i8', ALLOC_NORMAL);
      }

      return node['stringValuePtr'];
    },

    json_integer_value: function(nodePtr) {
      var node = JANSSON.getNodeByPtr(nodePtr);

      if (!node)
        return null;

      return node['node']|0;
    },

    json_real_value: function(nodePtr) {
      var node = JANSSON.getNodeByPtr(nodePtr);

      if (!node)
        return null;

      return node['node'];
    },

    json_number_value: function(nodePtr) {
      var node = JANSSON.getNodeByPtr(nodePtr);

      if (!node)
        return null;

      return node['node'];
    },

    json_delete: function(nodePtr) {
      // We assume, if the root node is deleted, all nodes and the context are deleted.
      if (JANSSON.getNodeIDByPtr(nodePtr) == 1) {
        for (var node in g_json_context) {
          JANSSON.freeNode(node);
        }

        g_json_context = null;
      }
    }
};

autoAddDeps(LibraryJansson, '$JANSSON');
mergeInto(LibraryManager.library, LibraryJansson);
