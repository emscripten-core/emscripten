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

  var TYPE_ID_IDX = 0,
      NAME_IDX = 1,
      TAG_IDX = 2,
      BASE_TYPE_IDX = 3,
      OFFSET_IDX = 4,
      SIZE_IDX = 5,
      C_ELEMS_IDX = 7;

  function TypedVariable(base_ptr, type_id) {
    this.derives = [];
    this.primary = null;
    this.primary_type_id = "";
    this.base_ptr = base_ptr;

    this.type_id = type_id;
    this.built = false;
    this.resolved = false;
    this.name = "";
    this.value = "";
    this.pointerCount = 0;
    this.offset = 0;
    this.size = 0;
    this.standalone_id = 0;
    this.dereference = true;
    this.isMember = false;
    this.isInherited = false
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

  TypedVariable.prototype._processTypes = function() {
    var name_vec = [];

    for (var i in this.derives) {
      switch (this.derives[i][TAG_IDX]) {
        case DW_TAG_reference_type: {
          name_vec.unshift("&");
          this.pointerCount++;
        } break;
        case DW_TAG_pointer_type: {
          name_vec.unshift("*");
          this.pointerCount++;
        } break;
        case DW_TAG_const_type: {
          name_vec.unshift("const");
        } break;
        case DW_TAG_inheritance: {
          this.isInherited = true;
          name_vec.unshift(">");
          this.offset = this.derives[i][OFFSET_IDX] / 8;
        } break;
        case DW_TAG_member: {
          this.isMember = true;
          name_vec.push(":");
          name_vec.push(this.derives[i][NAME_IDX])
          this.offset = this.derives[i][OFFSET_IDX] / 8;
        } break;
        case DW_TAG_typedef: {
          // Ignoring typedefs for the time being
        } break;
        case DW_TAG_volatile_type: {
          name_vec.unshift("volatile");
        }
        default:
          console.error("Unimplemented " + type_descriptor);
      }
    }

    if (this.primary[TYPE_ID_IDX] == BASIC_TYPE) {
      this.size = this.primary[4];
      name_vec.unshift(this.primary[NAME_IDX]);
    } else if (this.primary[TYPE_ID_IDX] == COMPOSITE_TYPE) {
      this.size = this.primary[SIZE_IDX];
      name_vec.unshift(this.primary[NAME_IDX]);

      switch (this.primary[TAG_IDX]) {
        case DW_TAG_structure_type: {
          name_vec.unshift("struct");
        } break;
        case DW_TAG_class_type: {
          name_vec.unshift("class");
        } break;
        case DW_TAG_enumeration_type: {
          name_vec.unshift("class");
        } break;
        case DW_TAG_union_type: {
          name_vec.unshift("union");
        } break;
        case DW_TAG_array_type: {
          name_vec.unshift("array");
        } break;
        default:
          console.error("Unimplemented for composite " + this.primary);
      }
    }

    this.name = name_vec.join(" ");
  }

  TypedVariable.prototype._getMyAddress = function() {
    var base_addr = this.base_ptr + this.offset;
    for (var i = 0; i < this.pointerCount; i++) {
      if (base_addr == 0) {
        return 0;
      }
      base_addr = heap["u32"][base_addr >> 2];
    }
    return base_addr;
  }

  TypedVariable.prototype._resolveBaseTypeValue = function() {
    this.resolved = true;
    if (this.primary[TYPE_ID_IDX] == BASIC_TYPE) {
      if (!this.dereference) {
        switch (this.primary[TAG_IDX]) {
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

      for (var i in this.primary[C_ELEMS_IDX]) {
        var cur_elem = new TypedVariable(this._getMyAddress(), this.primary[C_ELEMS_IDX][i]);

        cur_elem._initialBuild();

        if (cur_elem.size > 0 && (cur_elem.isInherited || cur_elem.isMember)) {
          var pointed_elem = new TypedVariable(cur_elem._getMyAddress(), cur_elem.primary_type_id);
          pointed_elem._initialBuild();
          this.value[cur_elem.name] = pointed_elem;
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

    if (this.value == null) {
      return "null";
    }

    var childNames = Object.keys(this.value);

    var retval = {};

    for (var i in childNames) {
      if (this.value[childNames[i]].resolved) {
        retval[childNames[i]] = this.value[childNames[i]]._prettyPrintToObjectHelper();
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
    invert_vtables();
  }

  function type_descriptor_to_heap_id(type_descriptor) {
    var id = "";
    switch (type_descriptor[TAG_IDX]) {
      case DW_ATE_unsigned: case DW_ATE_unsigned_char: id = "u"; break;
      case DW_ATE_signed: case DW_ATE_signed_char: id = "i"; break;
      case DW_ATE_float: id = "f"; break;
    }
    id += type_descriptor[4];
    return id;
  }

  function type_id_to_type_descriptor(type_id, ptr, dit) {
    if (!isNaN(+type_id)) {
      return cyberdwarf["types"][type_id];
    }
    if (typeof(type_id) === "string") {
      if (dit) {
        type_id = resolve_from_vtable(ptr, type_id);
      }
      type_id = cyberdwarf["type_name_map"][type_id];
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

  function pretty_print_to_object(val, type_id, depth) {
    install_heap(Module.HEAPU8.buffer);
    var base_type = new TypedVariable(val, current_function[type_id]);
    return base_type.prettyPrintToObject(depth);
  }

  function pretty_print_from_typename(val, type_name, depth) {
    install_heap(Module.HEAPU8.buffer);
    var base_type = new TypedVariable(val, type_name);
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

  return {
    "decode_from_stack": stack_decoder,
    "initialize_debugger": initialize_debugger,
    "set_current_function": set_current_function,
    "decode_var_by_var_name": pretty_print_to_object,
    "decode_var_by_type_name": pretty_print_from_typename
  };
};

mergeInto(LibraryManager.library, { "cyberdwarf_Debugger": CyberDWARFHeapPrinter });
