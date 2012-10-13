/*
 * Javascript implementation for the Jansson JSON parser.
 * Source: https://github.com/akheron/jansson
 *
 * Implemented and tested with:
 * https://github.com/akheron/jansson/commit/cf1074e70ea42a1dea4d6e57b2b29532049bcd28
 */

var LibraryJansson = {

    $JANSSON: {

      load: function(string, flags, error) {
        var json_obj = eval('(' + string + ')');

        if (json_obj != null) {
          // Create the root node with the nodeID `1`
          g_json_context = [null, {"name" : "root", "node" : json_obj}];
          return 1;
        } else {
          g_json_context = null;
          return null;
        }
      },

      type: function(nodeID) {
        if (!g_json_context)
          return null;

        var node = g_json_context[nodeID].node;
        if (typeof(node) === 'object' && (node instanceof Array))
          return 'array'
        else
          return typeof(node);
      },

      getNode: function(parentNodeID) {
        if (!g_json_context)
          return null;

        var parentNode = g_json_context[parentNodeID];
    
        if (!parentNode) {
          console.log("Jansson: Node with ID `" + parentNodeID + "` not found.");
          return null;
        }

        // Add all child nodes of the parent node to the context.
        // Consequently we can access these child nodes with an ID (pointer)
        // TODO: Here is room for performance improvements
        if (!parentNode.begin) {
          var childNodeID = g_json_context.length;
          for(var childNode in parentNode.node) {
            var childNodeID = g_json_context.length;
            g_json_context[childNodeID] = {"name" : childNode, "node" : parentNode.node[childNode]};
            if (!parentNode.begin) parentNode.begin = childNodeID;
          };
          parentNode.end = childNodeID;
        }

        return parentNode;
      }

    },

    json_loads: function(string, flags, error) {
     return JANSSON.load(Pointer_stringify(string), flags, error);
    },

    json_loadb: function(buffer, buflen, flags, error) {
      return JANSSON.load(Pointer_stringify(buffer).substr(0, buflen), flags, error);
    },

    json_is_object: function(nodeID) {
      return (JANSSON.type(nodeID) === 'object');
    },

    json_is_array: function(nodeID) {
      return (JANSSON.type(nodeID) === 'array');
    },

    json_is_string: function(nodeID) {
      return (JANSSON.type(nodeID) === 'string');
    },

    json_is_integer: function(nodeID) {
      return (JANSSON.type(nodeID) === 'number');
    },

    json_is_real: function(nodeID) {
      return (JANSSON.type(nodeID) === 'number');
    },

    json_is_number: function(nodeID) {
      return (JANSSON.type(nodeID) === 'number');
    },

    json_is_null: function(nodeID) {
      var nodeType = JANSSON.type(nodeID);
      return (nodeType === 'undefined' || nodeType === 'null');
    },

    json_object_get: function(parentNodeID, key) {
      var key = Pointer_stringify(key);
      var parentNode = JANSSON.getNode(parentNodeID);

      if (!parentNode)
        return null;

      // Find the ID (pointer) of the requested child node
      for (var i=parentNode.begin; i<=parentNode.end; i++) {
        if (g_json_context[i].name == key) return i; 
      }
      
      return null;
    },

    json_object_iter: function(parentNodeID) {
      var parentNode = JANSSON.getNode(parentNodeID);

      if (!parentNode)
        return null;

      return parentNode.begin;
    },

    json_object_iter_next: function(parentNodeID, childNodeID) {
      var parentNode = JANSSON.getNode(parentNodeID);

      if (!parentNode)
        return null;

      if (childNodeID < parentNode.begin || childNodeID >= parentNode.end)
        return null;
      else
        return childNodeID+1;
    },

    json_array_size: function(parentNodeID, index) {
      var parentNode = JANSSON.getNode(parentNodeID);

      if (!parentNode)
        return 0;

      var size = parentNode.end - parentNode.begin + 1;

      if (size < 0)
       size = 0;

      return size;
    },

    json_array_get: function(parentNodeID, index) {
      var parentNode = JANSSON.getNode(parentNodeID);
      var position = parentNode.begin + index;

      if (position < parentNode.begin || position > parentNode.end)
        return null;
      else
        return position;
    },

    json_object_iter_key: function(nodeID) {
      var node = g_json_context[nodeID];

      if (!node)
        return null;

      return allocate(intArrayFromString(node.name), 'i8', ALLOC_NORMAL);
    },

    json_object_iter_value: function(nodeID) {
      return nodeID;
    },

    json_string_value: function(nodeID) {
      var node = g_json_context[nodeID];

      if (!node)
        return null;

      return allocate(intArrayFromString(node.node), 'i8', ALLOC_NORMAL);
    },

    json_integer_value: function(nodeID) {
      var node = g_json_context[nodeID];

      if (!node)
        return null;

      // Convert JSON number to an integer
      // c.f. http://stackoverflow.com/questions/596467/how-do-i-convert-a-float-to-an-int-in-javascript
      if (node.node<0)
        return Math.ceil(node.node);
      else
        return Math.floor(node.node);
    },

    json_real_value: function(nodeID) {
      var node = g_json_context[nodeID];

      if (!node)
        return null;

      return node.node;
    },

    json_number_value: function(nodeID) {
      var node = g_json_context[nodeID];

      if (!node)
        return null;

      return node.node;
    },

    json_delete: function(nodeID) {
      // We assume, if the root node's reference count is decreased, we can forget the context.
      if (nodeID == 1)
        g_json_context = null;
    }
};

autoAddDeps(LibraryJansson, '$JANSSON');
mergeInto(LibraryManager.library, LibraryJansson);
