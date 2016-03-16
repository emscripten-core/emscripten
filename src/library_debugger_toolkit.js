var CyberDWARFHeapPrinter = function(cdFileLocation) {
  var BASIC_TYPE = 0,
      DERIVED_TYPE = 1,
      COMPOSITE_TYPE = 2,
      SUBROUTINE_TYPE = 3,
      SUBRANGE_INFO = 4,
      SUBPROGRAM_TYPE = 5,
      ENUMERATOR_TYPE = 6,
      STRING_REFERENCE = 10;

  var DW_TAG_array_type = 0x01,
      DW_TAG_class_type = 0x02,
      DW_TAG_entry_point = 0x03,
      DW_TAG_enumeration_type = 0x04,
      DW_TAG_formal_parameter = 0x05,
      DW_TAG_imported_declaration = 0x08,
      DW_TAG_label = 0x0a,
      DW_TAG_lexical_block = 0x0b,
      DW_TAG_member = 0x0d,
      DW_TAG_pointer_type = 0x0f,
      DW_TAG_reference_type = 0x10,
      DW_TAG_compile_unit = 0x11,
      DW_TAG_string_type = 0x12,
      DW_TAG_structure_type = 0x13,
      DW_TAG_subroutine_type = 0x15,
      DW_TAG_typedef = 0x16,
      DW_TAG_union_type = 0x17,
      DW_TAG_unspecified_parameters = 0x18,
      DW_TAG_variant = 0x19,
      DW_TAG_common_block = 0x1a,
      DW_TAG_common_inclusion = 0x1b,
      DW_TAG_inheritance = 0x1c,
      DW_TAG_inlined_subroutine = 0x1d,
      DW_TAG_module = 0x1e,
      DW_TAG_ptr_to_member_type = 0x1f,
      DW_TAG_set_type = 0x20,
      DW_TAG_subrange_type = 0x21,
      DW_TAG_with_stmt = 0x22,
      DW_TAG_access_declaration = 0x23,
      DW_TAG_base_type = 0x24,
      DW_TAG_catch_block = 0x25,
      DW_TAG_const_type = 0x26,
      DW_TAG_constant = 0x27,
      DW_TAG_enumerator = 0x28,
      DW_TAG_file_type = 0x29,
      DW_TAG_friend = 0x2a,
      DW_TAG_namelist = 0x2b,
      DW_TAG_namelist_item = 0x2c,
      DW_TAG_packed_type = 0x2d,
      DW_TAG_subprogram = 0x2e,
      DW_TAG_template_type_parameter = 0x2f,
      DW_TAG_template_value_parameter = 0x30,
      DW_TAG_thrown_type = 0x31,
      DW_TAG_try_block = 0x32,
      DW_TAG_variant_part = 0x33,
      DW_TAG_variable = 0x34,
      DW_TAG_volatile_type = 0x35,
      DW_TAG_dwarf_procedure = 0x36,
      DW_TAG_restrict_type = 0x37,
      DW_TAG_interface_type = 0x38,
      DW_TAG_namespace = 0x39,
      DW_TAG_imported_module = 0x3a,
      DW_TAG_unspecified_type = 0x3b,
      DW_TAG_partial_unit = 0x3c,
      DW_TAG_imported_unit = 0x3d,
      DW_TAG_condition = 0x3f,
      DW_TAG_shared_type = 0x40,
      DW_TAG_lo_user = 0x4080,
      DW_TAG_hi_user = 0xffff;

  var DW_ATE_address = 0x01,
      DW_ATE_boolean = 0x02,
      DW_ATE_complex_float = 0x03,
      DW_ATE_float = 0x04,
      DW_ATE_signed = 0x05,
      DW_ATE_signed_char = 0x06,
      DW_ATE_unsigned = 0x07,
      DW_ATE_unsigned_char = 0x08,
      DW_ATE_imaginary_float = 0x09,
      DW_ATE_packed_decimal = 0x0a,
      DW_ATE_numeric_string = 0x0b,
      DW_ATE_edited = 0x0c,
      DW_ATE_signed_fixed = 0x0d,
      DW_ATE_unsigned_fixed = 0x0e,
      DW_ATE_decimal_float = 0x0f,
      DW_ATE_lo_user = 0x80,
      DW_ATE_hi_user = 0xff;

  // Right now only support deref ops
  var DW_OP_deref = 0x06;

  var TYPE_ID_IDX = 0,
      NAME_IDX = 1,
      TAG_IDX = 2,
      BASE_TYPE_IDX = 3,
      OFFSET_IDX = 4,
      SIZE_IDX = 5,
      C_ELEMS_IDX = 7;

  function TypedVariable(base_ptr, type_id, dwarf_md) {
    // Resolve count represents how many pointers have been followed already
    if (typeof(dwarf_md) === "undefined") {
      this.resolve_count = 0;
    } else {
      this._parseDwarf(dwarf_md);
    }

    this.derives = [];
    this.primary = null;
    this.primary_type_id = "";
    this.base_ptr = base_ptr;

    this.type_id = type_id;
    this.built = false;
    this.resolved = false;
    this.name = "";
    this.element_name = "";
    this.value = "";
    this.pointer_count = 0;
    this.offset = 0;
    this.size = 0;
    this.standalone_id = 0;
    this.isMember = false;
    this.isInherited = false;
    this.definitionMissing = false;
  }

  TypedVariable.prototype.toString = function() {
    return "[TypedVariable < " + JSON.stringify(this.derives) + " > " + JSON.stringify(this.primary) + " (&" + this._getMyAddress() + ")]";
  }

  TypedVariable.prototype._initialBuild = function() {
    if (this.built) {
      return;
    }

    this.built = true;
    this._buildDeriveChain();
    this._processTypes();
  }

  // DWARF ops are metadata associated with intrinsics, describing what, if
  // anything a debugger needs to do to the address to get to the variable.
  // CyberDWARF inheriently acts a heap dumper, expecting a pointer to the
  // object on the heap so there's always an implicit dereference.
  // In the case of intrinsics, that's usually not the case, but LLVM has a lot
  // of freedom on laying out variable storage.
  //
  // An example would be a pointer to a pointer and the pointer itself, LLVM
  // might simply use 1 register, and add a derefence to the bare pointer
  //
  // Emscripten doesn't really offer a lot of creative options for memory layout
  // (thankfully), but there are occasional DW_OP_bit_piece that show up rarely
  // in what I've seen

  TypedVariable.prototype._parseDwarf = function(dwarf_md) {
    var dwarf_op = cyberdwarf["types"][dwarf_md];
    if (dwarf_op[0] !== 99) {
      throw "Got a bad type for a dwarf expression " + JSON.stringify(dwarf_op) + " " + dwarf_md;
    }
    // By default we're already at the pointer to this value
    this.resolve_count = 1;

    // Remove 1 from the resolve count for each deref
    for (var i = 1; i < dwarf_op.length; i++) {
      if (dwarf_op[i] !== DW_OP_deref) {
        throw "Support for non deref chain DWARF ops is currently missing (got OP" + dwarf_op[i] + ")";
      }
      this.resolve_count -= 1;
    }
  }

  TypedVariable.prototype._buildDeriveChain = function() {
    var cur_td = type_id_to_type_descriptor(this.type_id, this.base_ptr);
    var ptr = this.base_ptr;
    while (cur_td[TYPE_ID_IDX] == 1 || cur_td[TYPE_ID_IDX] == 10) {
      if (cur_td[TYPE_ID_IDX] == 1) {
        this.derives.push(cur_td);
        this.primary_type_id = cur_td[BASE_TYPE_IDX];
        if (cur_td[TAG_IDX] == DW_TAG_member) {
          ptr += cur_td[OFFSET_IDX];
        }
        cur_td = type_id_to_type_descriptor(cur_td[BASE_TYPE_IDX], ptr);
      } else {
        this.primary_type_id = cur_td[1];
        cur_td = type_id_to_type_descriptor(cur_td[1], ptr);
      }
    }

    this.primary = cur_td;
  }

  TypedVariable.prototype._getArraySizes = function() {
    var sizes = [];

    for (var i in this.primary[C_ELEMS_IDX]) {
      var sub_range = cyberdwarf['types'][this.primary[C_ELEMS_IDX][i]];
      if (sub_range[TYPE_ID_IDX] == SUBRANGE_INFO) {
        sizes.push(sub_range[1]);
      }
    }

    return sizes;
  }

  TypedVariable.prototype._processTypes = function() {
    var name_vec = [];

    for (var i in this.derives) {
      switch (this.derives[i][TAG_IDX]) {
        case DW_TAG_reference_type: {
          name_vec.unshift("&");
          this.pointer_count++;
        } break;
        case DW_TAG_pointer_type: {
          name_vec.unshift("*");
          this.pointer_count++;
        } break;
        case DW_TAG_const_type: {
          name_vec.unshift("const");
        } break;
        case DW_TAG_inheritance: {
          this.isInherited = true;
          name_vec.unshift(">");
          this.offset = this.derives[i][OFFSET_IDX] / 8;
          /* Disable for now, need to fix this to support bit slices
          if (this.derives[i][OFFSET_IDX] % 8 !== 0) {
            console.log("Weird offset ", this.offset, name_vec, this.type_id);
          }*/
        } break;
        case DW_TAG_member: {
          this.isMember = true;
          name_vec.push(":");
          name_vec.push(this.derives[i][NAME_IDX])
          this.element_name = this.derives[i][NAME_IDX];
          this.offset = this.derives[i][OFFSET_IDX] / 8;
          /* Disable for now, need to fix this to support bit slices
          if (this.derives[i][SIZE_IDX] % 8 !== 0) {
            console.log("Weird size ", this.offset, name_vec, this.derives[i][SIZE_IDX], this.type_id);
          }
          if (this.derives[i][OFFSET_IDX] % 8 !== 0) {
            console.log("Weird offset ", this.offset, name_vec, this.derives[i][SIZE_IDX], this.derives[i], this.type_id);
          }*/
        } break;
        case DW_TAG_typedef: {
          // Ignoring typedefs for the time being
        } break;
        case DW_TAG_volatile_type: {
          name_vec.unshift("volatile");
        } break;
        default:
          console.error("Unimplemented " + type_descriptor);
      }
    }

    if (this.primary[TYPE_ID_IDX] == BASIC_TYPE) {
      this.size = this.primary[4];
      if (this.primary[NAME_IDX]) {
        name_vec.unshift(this.primary[NAME_IDX]);
      }
    } else if (this.primary[TYPE_ID_IDX] == COMPOSITE_TYPE) {
      this.size = this.primary[SIZE_IDX];
      if (this.primary[NAME_IDX]) {
        name_vec.unshift(this.primary[NAME_IDX]);
      }

      switch (this.primary[TAG_IDX]) {
        case DW_TAG_structure_type: {
          name_vec.unshift("struct");
        } break;
        case DW_TAG_class_type: {
          name_vec.unshift("class");
        } break;
        case DW_TAG_enumeration_type: {
          name_vec.unshift("enum");
        } break;
        case DW_TAG_union_type: {
          name_vec.unshift("union");
        } break;
        case DW_TAG_array_type: {
          var sizes = this._getArraySizes().join("][");
          name_vec.unshift("[" + sizes + "]");
          var exemplar = new TypedVariable(this._getMyAddress(), this.primary[BASE_TYPE_IDX]);
          exemplar._initialBuild();
          name_vec.unshift(exemplar.name)
        } break;
        default:
          console.error("Unimplemented for composite " + this.primary);
      }
    }

    this.name = name_vec.join(" ");
  }

  TypedVariable.prototype._getMyAddress = function() {
    var base_addr = this.base_ptr + this.offset;
    for (var i = this.resolve_count; i < this.pointer_count; i++) {
      if (base_addr == 0) {
        return 0;
      }
      base_addr = heap["u32"][base_addr >> 2];
    }
    return base_addr;
  }

  function _get_array_key_from_idx(size_vec, offset) {
    var idxes = [];
    for (var i in size_vec) {
      idxes.push(offset % size_vec[i]);
      offset /= size_vec[i];
      offset |= 0;
    }
    // This reads better with C code, following the order of brackets
    return idxes.reverse().join(":");
  }

  TypedVariable.prototype._resolveBaseTypeValue = function() {
    this.resolved = true;
    if (this.primary[TYPE_ID_IDX] == BASIC_TYPE) {
      if (this.resolve_count > 0) {
        switch (this.primary[TAG_IDX]) {
          case DW_ATE_boolean:
          case DW_ATE_unsigned:
          case DW_ATE_float:
          case DW_ATE_signed: {
            this.value = this.base_ptr;
          }; break;
        }
      } else {
        if (this.base_ptr == 0) {
          this.value = null;
        } else {
          var heap_id = type_descriptor_to_heap_id(this.primary);
          var ptr = this._getMyAddress();
          this.value = heap[heap_id][ptr >> heap_shift[heap_id]];
        }
      }
    } else {
      this.value = {};

      if (this.primary[TAG_IDX] === DW_TAG_array_type) {
        // For computing offset, reverse them.
        var sizes = this._getArraySizes().reverse();
        var total_size = 1;
        for (var i in sizes) {
          total_size *= sizes[i];
        }

        var array_of = type_id_to_type_descriptor(this.primary[BASE_TYPE_IDX]);
        while (array_of[TYPE_ID_IDX] == 10) {
          array_of = type_id_to_type_descriptor(array_of[TYPE_ID_IDX]);
        }

        elem_size = ((this.primary[SIZE_IDX] / total_size) / 8) | 0;

        for (var i = 0; i < total_size; i++) {
          var offset = this._getMyAddress() + (i * elem_size);
          var pointed_elem = new TypedVariable(offset, this.primary[BASE_TYPE_IDX]);
          pointed_elem._initialBuild();
          this.value[_get_array_key_from_idx(sizes, i)] = pointed_elem;
        }
      } else {
        for (var i in this.primary[C_ELEMS_IDX]) {
          var cur_elem = new TypedVariable(this._getMyAddress(), this.primary[C_ELEMS_IDX][i]);

          cur_elem._initialBuild();

          if (cur_elem.size > 0) {
            if (cur_elem.isInherited || cur_elem.isMember) {
              var pointed_elem = new TypedVariable(cur_elem._getMyAddress(), cur_elem.primary_type_id);
              pointed_elem._initialBuild();
              pointed_elem.element_name = cur_elem.element_name;
              this.value[cur_elem.name] = pointed_elem;
            }
          }
        }
      }
    }
  }

  TypedVariable.prototype._buildForPrettyPrint = function(depth) {
    if (typeof(depth) === "undefined") {
      depth = 1;
    }

    if (depth < 1) {
      return;
    }

    this._initialBuild();
    this._resolveBaseTypeValue();

    if (this.primary[TYPE_ID_IDX] != 2 || this._getMyAddress() == 0) {
      return;
    }

    for (var i in this.value) {
      this.value[i]._buildForPrettyPrint(depth - 1);
    }
  }

  TypedVariable.prototype._prettyPrintToObjectHelper = function() {

    if (typeof(this.value) !== "object") {
      return this.value;
    }

    var visualizer = get_visualizer(this);
    if (visualizer !== null) {
      return visualizer.to_object(this);
    }

    if (this.value == null) {
      return "null";
    }

    var childNames = Object.keys(this.value);

    var retval = {};

    for (var i in childNames) {
      if (this.value[childNames[i]].resolved) {
        retval[childNames[i]] = this.value[childNames[i]]._prettyPrintToObjectHelper();
      } else if (this.value[childNames[i]].base_ptr === 0) {
        retval[childNames[i]] = null;
      } else {
        retval[childNames[i]] = "cd_tvptr(0x" + this.value[childNames[i]]._getMyAddress().toString(16) + ",'" + this.value[childNames[i]].type_id + "')";
      }
    }

    return retval;
  }

  TypedVariable.prototype.prettyPrintToObject = function(depth) {
    this._buildForPrettyPrint(depth);
    var retval = {};
    retval[this.name] = this._prettyPrintToObjectHelper();
    return retval;
  }

  var heap = {};
  var cyberdwarf = undefined;
  var heap_shift = {
    "u8" : 0,
    "u16": 1,
    "u32": 2,
    "i8" : 0,
    "i16": 1,
    "i32": 2,
    "f32": 2,
    "f64": 3
  }
  function install_heap($heap) {
    heap = {
      "u8" : new Uint8Array($heap),
      "u16": new Uint16Array($heap),
      "u32": new Uint32Array($heap),
      "i8" : new Int8Array($heap),
      "i16": new Int16Array($heap),
      "i32": new Int32Array($heap),
      "f32": new Float32Array($heap),
      "f64": new Float64Array($heap)
    };
  }

  var symbols = {};

  function install_cyberdwarf(data_cd) {
    cyberdwarf = JSON.parse(data_cd)["cyberdwarf"];
    cyberdwarf['types']['0'] = [0, "void", 7, 0, 32];
    invert_vtables();
  }

  function type_descriptor_to_heap_id(type_descriptor) {
    var id = "";
    switch (type_descriptor[TAG_IDX]) {
      case DW_ATE_boolean: case DW_ATE_unsigned: case DW_ATE_unsigned_char: id = "u"; break;
      case DW_ATE_signed: case DW_ATE_signed_char: id = "i"; break;
      case DW_ATE_float: id = "f"; break;
    }
    id += type_descriptor[4];
    return id;
  }

  function type_id_to_type_descriptor(type_id, ptr) {
    if (!isNaN(+type_id)) {
      return cyberdwarf["types"][type_id];
    }
    if (typeof(type_id) === "string") {
      type_id = resolve_from_vtable(ptr, type_id);
      var temp_type_id = cyberdwarf["type_name_map"][type_id];
      if (typeof(temp_type_id) === "undefined") {
        if (typeof(cyberdwarf["type_name_map"]["fd_" + type_id]) !== "undefined") {
          type_id = cyberdwarf["type_name_map"]["fd_" + type_id];
        }
      } else {
        type_id = temp_type_id;
      }
    }
    return cyberdwarf["types"][type_id];
  }

  function invert_vtables() {
    cyberdwarf["has_vtable"] = {};
    for (var i in cyberdwarf["vtable_offsets"]) {
      cyberdwarf["has_vtable"][cyberdwarf["vtable_offsets"][i]] = true;
    }
  }

  function resolve_from_vtable(val, type_name) {
    var lookup_name = "_ZTV" + type_name.substr(4);
    if (cyberdwarf["has_vtable"][lookup_name]) {
      var potential_vtable = "" + (heap["u32"][val >> 2] - 8);
      if (cyberdwarf["vtable_offsets"][potential_vtable]) {
        var ans = type_name.substr(0,4) + cyberdwarf["vtable_offsets"][potential_vtable].substr(4);
        return ans;
      }
    }
    return type_name;
  }

  var current_function = "";
  function set_current_function(func_name) {
    if (typeof(cyberdwarf["function_name_map"][func_name]) !== "undefined") {
      func_name = cyberdwarf["function_name_map"][func_name];
    }
    if (func_name.substring(0,1) == "_") {
      func_name = func_name.substring(1);
    }
    current_function = cyberdwarf["functions"][func_name];
  }

  function pretty_print_to_object(val, type_id, depth, dwarf_md) {
    install_heap(Module.HEAPU8.buffer);
    var base_type = new TypedVariable(val, current_function[type_id], dwarf_md);
    return base_type.prettyPrintToObject(depth);
  }

  function pretty_print_from_typename(val, type_name, depth, dwarf_md) {
    install_heap(Module.HEAPU8.buffer);
    var base_type = new TypedVariable(val, type_name, dwarf_md);
    return base_type.prettyPrintToObject(depth);
  }

  function stack_decoder(val, name, depth, on_heap) {
    var stack_frames = jsStackTrace().split("\n");

    // To find the function that called us, look for the first reference to calling an EM_ASM block
    var stack_offset = 0;
    for (; stack_offset < stack_frames.length; stack_offset++) {
        if (stack_frames[stack_offset].match("emscripten_asm_const")) break;
    }

    // Subtract 1 since we shouldn't have found it on the last frame
    if (stack_offset == stack_frames.length - 1) {
      console.error("Couldn't find the function the decoder was called from");
      return {};
    }

    var func_finder = new RegExp("at (?:Object\\.|)([^\\s]+)", "g");
    var result = func_finder.exec(stack_frames[stack_offset + 1]);

    if (result.length > 1) {
      set_current_function(result[1]);
    }

    var decoded = pretty_print_to_object(val, name, depth);
    return decoded;
  }

  function initialize_debugger(cb) {
    var cdFile;
    if (typeof Module['locateFile'] === 'function') {
      cdFileLocation = Module['locateFile'](cdFileLocation);
    } else if (Module['cdInitializerPrefixURL']) {
      cdFileLocation = Module['cdInitializerPrefixURL'] + cdFileLocation;
    }
    if (ENVIRONMENT_IS_NODE || ENVIRONMENT_IS_SHELL) {
      var data = Module['readBinary'](cdFileLocation);
      install_cyberdwarf(data);
      if (typeof(cb) !== "undefined") {
        cb();
      }
    }

    var decoded = pretty_print_to_object(val, name, depth);
    return decoded;
  }

  function initialize_debugger(cb) {
    var cdFile;
    if (typeof Module['locateFile'] === 'function') {
      cdFileLocation = Module['locateFile'](cdFileLocation);
    } else if (Module['cdInitializerPrefixURL']) {
      cdFileLocation = Module['cdInitializerPrefixURL'] + cdFileLocation;
    }
    if (ENVIRONMENT_IS_NODE || ENVIRONMENT_IS_SHELL) {
      var data = Module['read'](cdFileLocation);
      install_cyberdwarf(data);
    } else {
      var applyCDFile = function(data) {
        var decoder = new TextDecoder("utf8");
        install_cyberdwarf(decoder.decode(data));
        console.info("Debugger ready");
        if (typeof(cb) !== "undefined") {
          cb();
        }
      }
      function doBrowserLoad() {
        Module['readAsync'](cdFileLocation, applyCDFile, function() {
          throw 'could not load debug data ' + cdFileLocation;
        });
      }
      // fetch it from the network ourselves
      doBrowserLoad();
    }
  }

  var visualizers = [];

  function register_visualizer(visualizer) {
    visualizers.push(visualizer);
  }

  function get_visualizer(typed_var) {
    for (v in visualizers) {
      if (visualizers[v].can_visualize(typed_var)) {
        return visualizers[v];
      }
    }
    return null;
  }

  function get_intrinsic_data(id) {
    return cyberdwarf["intrinsics"][id];
  }

  register_visualizer({
    can_visualize: function(typed_var) {
      return typed_var.primary[NAME_IDX].startsWith("vector<");
    },
    to_object: function(val) {
      var base = val.value[Object.keys(val.value)[0]].value;
      //console.error(JSON.stringify(val, null, "  "));
      var begin = null;
      var end = null;
      for (var e in base) {
        if (base[e].element_name === "__begin_") {
          begin = base[e];
        }
        if (base[e].element_name === "__end_") {
          end = base[e];
        }
      }
      if (begin == null) {
        console.error(JSON.stringify(val, null, "  "));
      }
      var cur_ptr = begin._getMyAddress();
      var vals = [];
      while (cur_ptr < end._getMyAddress()) {
        var c = new TypedVariable(cur_ptr, begin.type_id);
        c._buildForPrettyPrint(10);
        vals.push(c._prettyPrintToObjectHelper());
        cur_ptr += begin.size / 8;
      }

      return vals;
    }
  });

  var var_tracker = {};

  function var_tracker_logger(a, b, c, d, e) {
    var_tracker[e] = [a, b, c, d, e];
  }

  function enable_var_tracker() {
    Module["cyberdwarf_debug_constant"] = var_tracker_logger;
    Module["cyberdwarf_debug_value"] = var_tracker_logger;
  }

  function disable_var_tracker() {
    Module["cyberdwarf_debug_constant"] = false;
    Module["cyberdwarf_debug_value"] = false;
  }

  function dump_tracked_var(name) {
    console.log(var_tracker[name]);
    console.log(JSON.stringify(pretty_print_from_typename(var_tracker[name][0], var_tracker[name][1], 4, var_tracker[name][3]), null, "  "));
  }

  return {
    "decode_from_stack": stack_decoder,
    "initialize_debugger": initialize_debugger,
    "set_current_function": set_current_function,
    "decode_var_by_var_name": pretty_print_to_object,
    "decode_var_by_type_name": pretty_print_from_typename,
    "register_visualizer": register_visualizer,
    "get_intrinsic_data": get_intrinsic_data,
    "enable_var_tracker": enable_var_tracker,
    "disable_var_tracker": disable_var_tracker,
    "dump_tracked_var": dump_tracked_var
  };
};

mergeInto(LibraryManager.library, { "cyberdwarf_Debugger": CyberDWARFHeapPrinter });
