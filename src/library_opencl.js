/**
* library_opencl.js
* Licence : https://github.com/wolfviking0/webcl-translator/blob/master/LICENSE
*
* Created by Anthony Liot.
* Copyright (c) 2013 Anthony Liot. All rights reserved.
*
* @class LibraryOpenCL
*/
var LibraryOpenCL = {  

  $CL__deps: ['$GL'],
  $CL: {
    // Init
    cl_init: 0,
    // Extensions
    cl_extensions: ["KHR_GL_SHARING","KHR_fp16","KHR_fp64"],
    // Private array of chars to use
    cl_digits: [1,2,3,4,5,6,7,8,9,0],
    // Kernel parser
    cl_kernels_sig: {},
    // Structs Kernels parser
    cl_structs_sig: {},
    // Pointer type (void*)
    cl_pn_type: [],
    cl_objects: {},
    cl_objects_map: {},
    cl_objects_retains: {},
    cl_objects_mem_callback: {},

#if CL_VALIDATOR
    cl_validator: {},
    cl_validator_argsize: {},
#endif    

    /**
     * Description
     * @method init
     * @return MemberExpression
     */
    init: function() {
      if (CL.cl_init == 0) {
#if CL_VALIDATOR
        console.log('%c WebCL-Translator + Validator V2.0 by Anthony Liot & Steven Eliuk ! ', 'background: #222; color: #bada55');
#else        
        console.log('%c WebCL-Translator V2.0 by Anthony Liot & Steven Eliuk ! ', 'background: #222; color: #bada55');
#endif        
        var nodejs = (typeof window === 'undefined');
        if(nodejs) {
          webcl = require('../webcl');
        }

        if (webcl == undefined) {
          alert("Unfortunately your system does not support WebCL. " +
          "Make sure that you have WebKit Samsung or Firefox Nokia plugin");

          console.error("Unfortunately your system does not support WebCL.\n");
          console.error("Make sure that you have WebKit Samsung or Firefox Nokia plugin\n");  
        } else {

          // Add webcl constant for parser
          // Object.defineProperty(webcl, "SAMPLER"      , { value : 0x1300,writable : false });
          // Object.defineProperty(webcl, "IMAGE2D"      , { value : 0x1301,writable : false });
          // Object.defineProperty(webcl, "IMAGE3D"      , { value : 0x1302,writable : false });          
          // Object.defineProperty(webcl, "UNSIGNED_LONG", { value : 0x1304,writable : false });
          // Object.defineProperty(webcl, "LONG"         , { value : 0x1303,writable : false });
          // Object.defineProperty(webcl, "MAP_READ"     , { value : 0x1   ,writable : false });
          // Object.defineProperty(webcl, "MAP_WRITE"    , { value : 0x2   ,writable : false });

          for (var i = 0; i < CL.cl_extensions.length; i ++) {

#if CL_GRAB_TRACE
              CL.webclCallStackTrace(""+webcl+".enableExtension",[CL.cl_extensions[i]]);
#endif  
            if (webcl.enableExtension(CL.cl_extensions[i])) {
              console.info("WebCL Init : extension "+CL.cl_extensions[i]+" supported.");
            } else {
              console.info("WebCL Init : extension "+CL.cl_extensions[i]+" not supported !!!");
            }
          }
          CL.cl_init = 1;
        }
      }

      return CL.cl_init;
    },
    
    /**
     * Description
     * @method udid
     * @param {} obj
     * @return _id
     */
    udid: function (obj) {    
      var _id;

      if (obj !== undefined) {

        if ( obj.hasOwnProperty('udid') ) {
         _id = obj.udid;

         if (_id !== undefined) {
           return _id;
         }
        }
      }

      var _uuid = [];

      _uuid[0] = CL.cl_digits[0 | Math.random()*CL.cl_digits.length-1]; // First digit of udid can't be 0
      for (var i = 1; i < 6; i++) _uuid[i] = CL.cl_digits[0 | Math.random()*CL.cl_digits.length];

      _id = _uuid.join('');

#if CL_DEBUG
      if (_id in CL.cl_objects) {
        console.error("/!\\ **********************");
        console.error("/!\\ UDID not unique !!!!!!");
        console.error("/!\\ **********************");        
      }
#endif
    
      // /!\ Call udid when you add inside cl_objects if you pass object in parameter
      if (obj !== undefined) {
        Object.defineProperty(obj, "udid", { value : _id,writable : false });
        CL.cl_objects[_id]=obj;
      }

      return _id;      
    },

    /**
     * Description
     * @method cast_long
     * @param {} arg_size
     * @return 
     */
    cast_long: function(arg_size) {
  
      var _sizelong = [];

      _sizelong.push(((arg_size & 0xFFFFFFFF00000000) >> 32));
      _sizelong.push((arg_size & 0xFFFFFFFF));
      
      // var _origin = x << 32 | y;

      return new Int32Array(_sizelong);
    },

    /**
     * Description
     * @method stringType
     * @param {} pn_type
     * @return 
     */
    stringType: function(pn_type) {
      switch(pn_type) {
        case webcl.SIGNED_INT8:
          return 'INT8';
        case webcl.SIGNED_INT16:
          return 'INT16';
        case webcl.SIGNED_INT32:
          return 'INT32';
        case webcl.UNSIGNED_INT8:
          return 'UINT8';
        case webcl.UNSIGNED_INT16:
          return 'UINT16';
        case webcl.UNSIGNED_INT32:
          return 'UINT32';
        case 0x1304 /*webcl.UNSIGNED_LONG*/:
          return 'ULONG';
        case 0x1303 /*webcl.SIGNED_LONG*/:
          return 'LONG';       
        case webcl.FLOAT:
          return 'FLOAT';
        case webcl.LOCAL:
          return '__local';   
        case 0x1300 /*webcl.SAMPLER*/:
          return 'sampler_t';   
        case 0x1301 /*webcl.IMAGE2D*/:
          return 'image2d_t';        
        case 0x1302 /*webcl.IMAGE3D*/:
          return 'image3d_t';            
        default:
          if (typeof(pn_type) == "string") return 'struct';
          return 'UNKNOWN';
      }
    },

    /**
     * Description
     * @method parseType
     * @param {} string
     * @return _value
     */
    parseType: function(string) {
      var _value = -1;
    
      // First ulong for the webcl validator
      if ( (string.indexOf("ulong") >= 0 ) || (string.indexOf("unsigned long") >= 0 ) ) {
        // \todo : long ???? 
        _value = 0x1304 /*webcl.UNSIGNED_LONG*/;  
      } else if ( string.indexOf("long") >= 0 ) {
        _value = 0x1303 /*webcl.SIGNED_LONG*/;
      } else if (string.indexOf("float") >= 0 ) {
        _value = webcl.FLOAT;
      } else if ( (string.indexOf("uchar") >= 0 ) || (string.indexOf("unsigned char") >= 0 ) ) {
        _value = webcl.UNSIGNED_INT8;
      } else if ( string.indexOf("char") >= 0 ) {
        _value = webcl.SIGNED_INT8;
      } else if ( (string.indexOf("ushort") >= 0 ) || (string.indexOf("unsigned short") >= 0 ) ) {
        _value = webcl.UNSIGNED_INT16;
      } else if ( string.indexOf("short") >= 0 ) {
        _value = webcl.SIGNED_INT16;                     
      } else if ( (string.indexOf("uint") >= 0 ) || (string.indexOf("unsigned int") >= 0 ) ) {
        _value = webcl.UNSIGNED_INT32;          
      } else if ( ( string.indexOf("int") >= 0 ) || ( string.indexOf("enum") >= 0 ) ) {
        _value = webcl.SIGNED_INT32;
      } else if ( string.indexOf("image3d_t") >= 0 ) {
        _value = 0x1302 /*webcl.IMAGE3D*/;        
      } else if ( string.indexOf("image2d_t") >= 0 ) {
        _value = 0x1301 /*webcl.IMAGE2D*/;
      } else if ( string.indexOf("sampler_t") >= 0 ) {
        _value = 0x1300 /*webcl.SAMPLER*/;
      }

      return _value;
    },

    /**
     * Description
     * @method parseStruct
     * @param {} kernel_string
     * @param {} struct_name
     * @return 
     */
    parseStruct: function(kernel_string,struct_name) {

      // Experimental parse of Struct
      // Search kernel function like 'struct_name { }' or '{ } struct_name'
      // --------------------------------------------------------------------------------
      // Step 1 : Search pattern struct_name { }
      // Step 2 : if no result : Search pattern { } struct_name
      // Step 3 : if no result : return
      // Step 4 : split by ; // Num of variable of the structure  : int toto; float tata;
      // Step 5 : split by , // Num of variable for each type     : float toto,tata,titi;
      // Step 6 : Search pattern [num] // Array Variable          : float toto[4];
      // Step 7 : Search type of the line
      // Step 8 : if exist add type else search other struct
      // --------------------------------------------------------------------------------

      CL.cl_structs_sig[struct_name] = [];

      // First search if is #define
      var _re_define = new RegExp("#[\ ]*define[\ ]*"+struct_name+"[\ ]*[A-Za-z0-9_\s]*");
      var _define = kernel_string.match(_re_define);

      if (_define != null && _define.length == 1) {

        // Get type of the line
        var _str = _define[0];
        var _type = CL.parseType(_str);
        
        if (_type != -1) {
          CL.cl_structs_sig[struct_name].push(_type);
        } else {
          var _lastSpace = _str.lastIndexOf(" ");
          var _res = _str.substr(_lastSpace + 1,_str.length - _lastSpace);

          CL.parseStruct(kernel_string,_res);
        }
    
        return;
      }

      // Second search if is typedef type name;
      var _re_typedef = new RegExp("typedef[\ ]*[A-Za-z0-9_\s]*[\ ]*"+struct_name+"[\ ]*;");
      var _typedef = kernel_string.match(_re_typedef);

      if (_typedef != null && _typedef.length == 1) {

        // Get type of the line
        var _str = _typedef[0];
        var _type = CL.parseType(_str);

        if (_type != -1) {
          CL.cl_structs_sig[struct_name].push(_type);
        } else {
          _str = _str.replace(/^\s+|\s+$/g, ""); // trim
          var _firstSpace = _str.indexOf(" ");
          var _lastSpace = _str.lastIndexOf(" ");
          var _res = _str.substr(_firstSpace + 1,_lastSpace - _firstSpace - 1);
          
          CL.parseStruct(kernel_string,_res);
        }
        
        return;
      }

      // search pattern : struct_name { } ;
      var _re_before = new RegExp(struct_name+"[\ ]"+"\{([^}]+)\}");

      // search pattern : { } struct_name;
      var _re_after = new RegExp("\{([^}]+)\}"+"[\ ]"+struct_name);

      var _res = kernel_string.match(_re_before);
      var _contains_struct = "";
      
      if (_res != null && _res.length == 2) {
        _contains_struct = _res[1];
      } else {
        _res = kernel_string.match(_re_after);
        if (_res != null && _res.length == 2) {
            _contains_struct = _res[1];
        } else {
#if CL_DEBUG   
          console.error("Unknow Structure '"+struct_name+"', not found inside the kernel ...");
#endif
          return;
        }
      }

      var _var = _contains_struct.split(";");
      for (var i = 0; i < _var.length-1; i++ ) {
        // Need for unsigned int width, height;
        var _subvar = _var[i].split(","); 
        
        // Get type of the line
        var _type = CL.parseType(_var[i]);
      
        // Need for float mu[4];
        var _arrayNum = 0;
        _res = _var[i].match(/[0-9]+/); 
        if (_res != null) _arrayNum = _res;
      
        if ( _type != -1) {
          for (var j = 0; j < Math.max(_subvar.length,_arrayNum) ; j++ ) {
            CL.cl_structs_sig[struct_name].push(_type);
          }
        } else {
          // Search name of the parameter
          var _struct = _subvar[0].replace(/^\s+|\s+$/g, ""); // trim
          var _name = "";
          var _start = _struct.lastIndexOf(" "); 
          for (var j = _start - 1; j >= 0 ; j--) {
            var _chara = _struct.charAt(j);
            if (_chara == ' ' && _name.length > 0) {
              break;
            } else if (_chara != ' ') {
              _name = _chara + _name;
            }
          }
          
          // If struct is unknow search it
          if (!(_name in CL.cl_structs_sig && CL.cl_structs_sig[_name].length > 0)) {
            CL.parseStruct(kernel_string,_name);
          }

          for (var j = 0; j < Math.max(_subvar.length,_arrayNum) ; j++ ) {
            CL.cl_structs_sig[struct_name] = CL.cl_structs_sig[struct_name].concat(CL.cl_structs_sig[_name]);  
          }
        }
      }
    },

    /**
     * Description
     * @method parseKernel
     * @param {} kernel_string
     * @return _mini_kernel_string
     */
    parseKernel: function(kernel_string) {

#if 0
      console.info("Original Kernel String : ");
      console.info("--------------------------------------------------------------------");
      console.info(kernel_string);
      console.info("--------------------------------------------------------------------");
#endif

      // Experimental parse of Kernel
      // ----------------------------
      //
      // /!\ The minify kernel could be use by the program but some trouble with line
      // /!\ containing macro #define, for the moment only use the minify kernel for 
      // /!\ parsing __kernel and struct
      //
      // Search kernel function like __kernel ... NAME ( p1 , p2 , p3)  
      // --------------------------------------------------------------------------------
      // Step 1 : Minimize kernel removing all the comment and \r \n \t and multispace
      // Step 2 : Search pattern __kernel ... ( ... )
      // Step 3 : For each kernel
      // Step 3 . 1 : Search Open Brace
      // Step 3 . 2 : Search Kernel Name
      // Step 3 . 3 : Search Kernel Parameter
      // Step 3 . 4 : Grab { name : [ param, ... ] }
      // --------------------------------------------------------------------------------

      // Remove all comments ...
      var _mini_kernel_string  = kernel_string.replace(/(?:((["'])(?:(?:\\\\)|\\\2|(?!\\\2)\\|(?!\2).|[\n\r])*\2)|(\/\*(?:(?!\*\/).|[\n\r])*\*\/)|(\/\/[^\n\r]*(?:[\n\r]+|$))|((?:=|:)\s*(?:\/(?:(?:(?!\\*\/).)|\\\\|\\\/|[^\\]\[(?:\\\\|\\\]|[^]])+\])+\/))|((?:\/(?:(?:(?!\\*\/).)|\\\\|\\\/|[^\\]\[(?:\\\\|\\\]|[^]])+\])+\/)[gimy]?\.(?:exec|test|match|search|replace|split)\()|(\.(?:exec|test|match|search|replace|split)\((?:\/(?:(?:(?!\\*\/).)|\\\\|\\\/|[^\\]\[(?:\\\\|\\\]|[^]])+\])+\/))|(<!--(?:(?!-->).)*-->))/g
, "");
      
      // Remove all char \n \r \t ...
      _mini_kernel_string = _mini_kernel_string.replace(/\n/g, " ");
      _mini_kernel_string = _mini_kernel_string.replace(/\r/g, " ");

      // Remove all the multispace
      _mini_kernel_string = _mini_kernel_string.replace(/\s{2,}/g, " ");

      // Search pattern : __kernel ... ( ... )
      // var _matches = _mini_kernel_string.match(/__kernel[A-Za-z0-9_\s]+\(([^)]+)\)/g);
      // if (_matches == null) {
      //   console.error("/!\\ Not found kernel !!!");
      //   return;
      // }

      // Search kernel (Pattern doesn't work with extra __attribute__)
      var _matches = [];
      var _found = 1;
      var _stringKern = _mini_kernel_string;
      var _security = 10;

      // Search all the kernel
      while (_found && _security) {
        // Just in case no more than 10 loop
        _security --;

        var _pattern = "__kernel ";
        var _kern = _stringKern.indexOf(_pattern);

        if (_kern == -1) {
          _pattern = " kernel ";
          _kern = _stringKern.indexOf(" kernel ");
          if (_kern == -1) { 
            _pattern = "kernel ";
            _kern = _stringKern.indexOf("kernel ");
            if (_kern == -1) {
              _found = 0;
              continue;
            } else if (_kern != 0) {
              console.error("/!\\ Find word 'kernel' but is not a real kernel  .. ("+_kern+")");
              _stringKern = _stringKern.substr(_kern + _pattern.length,_stringKern.length - _kern);
              continue;
            }
          }
        }

        _stringKern = _stringKern.substr(_kern + _pattern.length,_stringKern.length - _kern);
 
        var _brace = _stringKern.indexOf("{");
        var _stringKern2 = _stringKern.substr(0,_brace);
        var _braceOpen = _stringKern2.lastIndexOf("(");
        var _braceClose = _stringKern2.lastIndexOf(")");
        var _stringKern3 = _stringKern2.substr(0,_braceOpen).replace(/^\s+|\s+$/g, ""); // trim
        var _space = _stringKern3.lastIndexOf(" ");

        _stringKern2 = _stringKern2.substr(_space + 1,_braceClose);

        // Add the kernel result like name_kernel(..., ... ,...)
        _matches.push(_stringKern2);
      }

      // For each kernel ....
      for (var i = 0; i < _matches.length; i ++) {
        // Search the open Brace
        var _brace = _matches[i].lastIndexOf("(");

        // Part before '('
        var _first_part = _matches[i].substr(0,_brace);
        _first_part = _first_part.replace(/^\s+|\s+$/g, ""); // trim

        // Part after ')'
        var _second_part = _matches[i].substr(_brace+1,_matches[i].length-_brace-2);
        _second_part = _second_part.replace(/^\s+|\s+$/g, ""); // trim

        // Search name part
        var _name = _first_part.substr(_first_part.lastIndexOf(" ") + 1);

        // If name already present reparse it may be is another test with not the same num of parameter ....
        if (_name in CL.cl_kernels_sig) {
          delete CL.cl_kernels_sig[_name]
        }

        // Search parameter part
        var _param = [];

#if CL_VALIDATOR        
        var _param_validator = [];
        var _param_argsize_validator = [];
#endif        
        var _array = _second_part.split(","); 
        for (var j = 0; j < _array.length; j++) {
          var _type = CL.parseType(_array[j]);

          if (_array[j].indexOf("__local") >= 0 ) {
            _param.push(webcl.LOCAL);

#if CL_VALIDATOR
            if (_array[j].indexOf("ulong _wcl") == -1 ) {
              _param_validator.push(_param.length - 1);
            } else {
              _param_argsize_validator.push(_param.length - 1);
            }
#endif    

          } else if (_type == -1) {
                       
            _array[j] = _array[j].replace(/^\s+|\s+$/g, "");
            _array[j] = _array[j].replace("*", "");

            var _start = _array[j].lastIndexOf(" "); 
            if (_start != -1) {
              var _kernels_struct_name = "";
              // Search Parameter type Name
              for (var k = _start - 1; k >= 0 ; k--) {

                var _chara = _array[j].charAt(k);
                if (_chara == ' ' && _kernels_struct_name.length > 0) {
                  break;
                } else if (_chara != ' ') {
                  _kernels_struct_name = _chara + _kernels_struct_name;
                }
              }             

              // Parse struct only if is not already inside the map
              if (!(_kernels_struct_name in CL.cl_structs_sig))
                CL.parseStruct(_mini_kernel_string, _kernels_struct_name);
            
              // Add the name of the struct inside the map of param kernel
              _param.push(_kernels_struct_name);         

            } else {
#if CL_DEBUG
              console.error("Unknow parameter type inside '"+_array[j]+"', can be a struct, use float by default ...");
#endif        
              _param.push(webcl.FLOAT);
            }

#if CL_VALIDATOR
            if (_array[j].indexOf("ulong _wcl") == -1 ) {
              _param_validator.push(_param.length - 1);
            } else {
              _param_argsize_validator.push(_param.length - 1);
            }
#endif    

          } else {
            _param.push(_type);

#if CL_VALIDATOR
            if (_array[j].indexOf("ulong _wcl") == -1 ) {
              _param_validator.push(_param.length - 1);
            } else {
              _param_argsize_validator.push(_param.length - 1);
            }
#endif    
          }
        }        

        CL.cl_kernels_sig[_name] = _param;

#if CL_VALIDATOR        
        CL.cl_validator[_name] = _param_validator;
        CL.cl_validator_argsize[_name] = _param_argsize_validator;
#endif
      }

#if 0         
      console.info("Mini Kernel String : ");
      console.info("--------------------------------------------------------------------");
      console.info(_mini_kernel_string);
      console.info("--------------------------------------------------------------------");
#endif
//#if 0
      for (var name in CL.cl_kernels_sig) {
        var _length = CL.cl_kernels_sig[name].length;
        var _str = "";
        for (var i = 0; i < _length ; i++) {
          var _type = CL.cl_kernels_sig[name][i];
          _str += _type + "("+CL.stringType(_type)+")";
          if (i < _length - 1) _str += ", ";
        }

        console.info("Kernel " + name + "(" + _length + ")");  
        console.info("\t" + _str);  

#if CL_VALIDATOR
        console.info("\tValidator Info : ");
        console.info("\t\tARG PARAM KERNEL"); 
        var _str = "( ";
        var _length = CL.cl_validator[name].length;
        for (var i = 0 ; i < _length ; i++) {
            _str += CL.cl_validator[name][i];
            if (i < _length - 1) _str += ", ";
        }
        _str += " )";
        console.info("\t\t\t"+_str);
        console.info("\t\tARG SIZE PARAM KERNEL (ulong _wcl...)"); 
        var _str = "( ";
        var _length = CL.cl_validator_argsize[name].length;
        for (var i = 0 ; i < _length ; i++) {
            _str += CL.cl_validator_argsize[name][i];
            if (i < _length - 1) _str += ", ";
        }
        _str += " )";
        console.info("\t\t\t"+_str);
#endif

      }

      for (var name in CL.cl_structs_sig) {
        var _length = CL.cl_structs_sig[name].length;
        var _str = "";
        for (var i = 0; i < _length ; i++) {
          var _type = CL.cl_structs_sig[name][i];
          _str += _type + "("+CL.stringType(_type)+")";
          if (i < _length - 1) _str += ", ";
        }

        console.info("\n\tStruct " + name + "(" + _length + ")");  
        console.info("\t\t" + _str);              
      }
//#endif
      return _mini_kernel_string;

    },

    /**
     * Description
     * @method getImageSizeType
     * @param {} image
     * @return _sizeType
     */
    getImageSizeType: function (image) {
      var _sizeType = 0;

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[image]+".getInfo",[webcl.IMAGE_FORMAT]);
#endif   
      
      var _info = CL.cl_objects[image].getInfo(webcl.IMAGE_FORMAT);

      switch (_info.channelType) {
        case webcl.SNORM_INT8:
        case webcl.SIGNED_INT8:
        case webcl.UNORM_INT8:        
        case webcl.UNSIGNED_INT8:
          _sizeType = 1;
          break;
        case webcl.SNORM_INT16:
        case webcl.SIGNED_INT16:
        case webcl.UNORM_INT16:        
        case webcl.UNSIGNED_INT16:
        case webcl.HALF_FLOAT:
          _sizeType = 2;      
          break;
        case webcl.SIGNED_INT32:
        case webcl.UNSIGNED_INT32:      
        case webcl.FLOAT:
          _sizeType = 4;
          break;
        default:
          console.error("getImageSizeType : This channel type is not yet implemented => "+_info.channelType);
      }

      return _sizeType;
    },


    /**
     * Description
     * @method getImageFormatType
     * @param {} image
     * @return _type
     */
    getImageFormatType: function (image) {
      var _type = 0;

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[image]+".getInfo",[webcl.IMAGE_FORMAT]);
#endif   

      var _info = CL.cl_objects[image].getInfo(webcl.IMAGE_FORMAT);

      switch (_info.channelType) {
        case webcl.SNORM_INT8:
        case webcl.SIGNED_INT8:
          _type = webcl.SIGNED_INT8;
          break;
        case webcl.UNORM_INT8:        
        case webcl.UNSIGNED_INT8:
          _type = webcl.UNSIGNED_INT8;
          break;
        case webcl.SNORM_INT16:
        case webcl.SIGNED_INT16:
          _type = webcl.SIGNED_INT16;
          break;
        case webcl.UNORM_INT16:        
        case webcl.UNSIGNED_INT16:
          _type = webcl.UNSIGNED_INT16;
          break;
        case webcl.SIGNED_INT32:
          _type = webcl.SIGNED_INT32;
        case webcl.UNSIGNED_INT32:
          _type = webcl.UNSIGNED_INT32;
          break;        
        case webcl.FLOAT:
          _type = webcl.FLOAT;
          break;
        default:
          console.error("getImageFormatType : This channel type is not yet implemented => "+_info.channelType);
      }

      return _type;
    },

    /**
     * Description
     * @method getImageSizeOrder
     * @param {} image
     * @return _sizeOrder
     */
    getImageSizeOrder: function (image) {
      var _sizeOrder = 0;

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[image]+".getInfo",[webcl.IMAGE_FORMAT]);
#endif   

      var _info = CL.cl_objects[image].getInfo(webcl.IMAGE_FORMAT);

      switch (_info.channelOrder) {
        case webcl.R:
        case webcl.A:
        case webcl.INTENSITY:
        case webcl.LUMINANCE:
          _sizeOrder = 1;
          break;
        case webcl.RG:
        case webcl.RA:
          _sizeOrder = 2;
          break;
        case webcl.RGB:
          _sizeOrder = 3;
          break; 
        case webcl.RGBA:
        case webcl.BGRA:
        case webcl.ARGB:      
          _sizeOrder = 4;
          break;        
        default:
          console.error("getImageFormatType : This channel order is not yet implemented => "+_info.channelOrder);
      }

      return _sizeOrder;
    },
    /**
     * Description
     * @method getHostPtrArray
     * @param {} size
     * @param {} type
     * @return _host_ptr
     */
    getHostPtrArray: function(size,type) { 

      var _host_ptr = null;

      if (type.length == 0) {
#if CL_DEBUG
        console.error("getHostPtrArray : error unknow type with length null "+type);
#endif
      }

      if (type.length == 1) {
        switch(type[0][0]) {
          case webcl.SIGNED_INT8:
            _host_ptr = new Int8Array( size );
            break;
          case webcl.SIGNED_INT16:
            _host_ptr = new Int16Array( size >> 1 );
            break;
          case webcl.SIGNED_INT32:
            _host_ptr = new Int32Array( size >> 2 );
            break;
          case webcl.UNSIGNED_INT8:
            _host_ptr = new Uint8Array( size );
            break;
          case webcl.UNSIGNED_INT16:
            _host_ptr = new Uint16Array( size >> 1 );
            break;
          case webcl.UNSIGNED_INT32:
            _host_ptr = new Uint32Array( size >> 2 );
            break;         
          default:
            _host_ptr = new Float32Array( size >> 2 );
            break;
        }
      } else {
        _host_ptr = new Float32Array( size >> 2 );
      }

      return _host_ptr;
    },

    /**
     * Description
     * @method getCopyPointerToArray
     * @param {} ptr
     * @param {} size
     * @param {} type
     * @return _host_ptr
     */
    getCopyPointerToArray: function(ptr,size,type) { 

      var _host_ptr = null;

      if (type.length == 0) {
#if CL_DEBUG
        console.error("getCopyPointerToArray : error unknow type with length null "+type);
#endif
      }

      if (type.length == 1) {
        switch(type[0][0]) {
          case webcl.SIGNED_INT8:
            _host_ptr = new Int8Array( {{{ makeHEAPView('8','ptr','ptr+size') }}} );
            break;
          case webcl.SIGNED_INT16:
            _host_ptr = new Int16Array( {{{ makeHEAPView('16','ptr','ptr+size') }}} );
            break;
          case webcl.SIGNED_INT32:
            _host_ptr = new Int32Array( {{{ makeHEAPView('32','ptr','ptr+size') }}} );
            break;
          case webcl.UNSIGNED_INT8:
            _host_ptr = new Uint8Array( {{{ makeHEAPView('U8','ptr','ptr+size') }}} );
            break;
          case webcl.UNSIGNED_INT16:
            _host_ptr = new Uint16Array( {{{ makeHEAPView('U16','ptr','ptr+size') }}} );
            break;
          case webcl.UNSIGNED_INT32:
            _host_ptr = new Uint32Array( {{{ makeHEAPView('U32','ptr','ptr+size') }}} );
            break;         
          default:
            _host_ptr = new Float32Array( {{{ makeHEAPView('F32','ptr','ptr+size') }}} );
            break;
        }
      } else {
        _host_ptr = new Float32Array( {{{ makeHEAPView('F32','ptr','ptr+size') }}} );
        
        // console.info("------");
        // _host_ptr = new DataView(new ArrayBuffer(size));

        // var _offset = 0;
        // for (var i = 0; i < type.length; i++) {
        //   var _type = type[i][0];
        //   var _num = type[i][1];
        //   switch(_type) {
        //     case webcl.SIGNED_INT8:
        //       _host_ptr.setInt8(_offset,new Int8Array( {{{ makeHEAPView('8','ptr+_offset','ptr+_offset+_num') }}} ));
        //       console.info("setInt8 : "+_offset+ " - "+(_offset+_num)+" / "+size );
        //       _offset += _num;
        //       break;
        //     case webcl.SIGNED_INT16:
        //       _host_ptr.setInt16(_offset,new Int16Array( {{{ makeHEAPView('16','ptr+_offset','ptr+_offset+_num*2') }}} ));
        //       console.info("setInt16 : "+_offset+ " - "+(_offset+_num*2)+" / "+size );
        //       _offset += 2*_num;
        //       break;
        //     case webcl.SIGNED_INT32:
        //       _host_ptr.setInt32(_offset,new Int32Array( {{{ makeHEAPView('32','ptr+_offset','ptr+_offset+_num*4') }}} ));
        //       console.info("setInt32 : "+_offset+ " - "+(_offset+_num*4)+" / "+size );
        //       _offset += 4*_num;
        //       break;
        //     case webcl.UNSIGNED_INT8:
        //       _host_ptr.setUint8(_offset,new Uint8Array( {{{ makeHEAPView('U8','ptr+_offset','ptr+_offset+_num') }}} ));
        //       console.info("setUint8 : "+_offset+ " - "+(_offset+_num)+" / "+size );
        //       _offset += _num;
        //       break;
        //     case webcl.UNSIGNED_INT16:
        //       host_ptr.setUint16(_offset,new Uint16Array( {{{ makeHEAPView('U16','ptr+_offset','ptr+_offset+_num*2') }}} ));
        //       console.info("setUint16 : "+_offset+ " - "+(_offset+_num*2)+" / "+size );
        //       _offset += 2*_num;
        //       break;
        //     case webcl.UNSIGNED_INT32:
        //       _host_ptr.setUint32(_offset,new Uint32Array( {{{ makeHEAPView('U32','ptr+_offset','ptr+_offset+_num*4') }}} ));
        //       console.info("setUint32 : "+_offset+ " - "+(_offset+_num*4)+" / "+size );
        //       _offset += 4*_num;
        //       break;         
        //     default:
        //       _host_ptr.setFloat32(_offset,new Float32Array( {{{ makeHEAPView('F32','ptr+_offset','ptr+_offset+_num*4') }}} ));
        //       console.info("setFloat32 : "+_offset+ " - "+(_offset+_num*4)+" / "+size );
        //       _offset += 4*_num;
        //       break;
        //   }
        // }
      }

      return _host_ptr;
    },

    /**
     * Description
     * @method getReferencePointerToArray
     * @param {} ptr
     * @param {} size
     * @param {} type
     * @return _host_ptr
     */
    getReferencePointerToArray: function(ptr,size,type) {  
      var _host_ptr = null;

      if (type.length == 0) {
#if CL_DEBUG        
        console.error("getCopyPointerToArray : error unknow type with length null "+type);
#endif
      }

      if (type.length == 1) {
        switch(type[0][0]) {
          case webcl.SIGNED_INT8:
            _host_ptr = {{{ makeHEAPView('8','ptr','ptr+size') }}};
            break;
          case webcl.SIGNED_INT16:
            _host_ptr = {{{ makeHEAPView('16','ptr','ptr+size') }}};
            break;
          case webcl.SIGNED_INT32:
            _host_ptr = {{{ makeHEAPView('32','ptr','ptr+size') }}};
            break;
          case webcl.UNSIGNED_INT8:
            _host_ptr = {{{ makeHEAPView('U8','ptr','ptr+size') }}};
            break;
          case webcl.UNSIGNED_INT16:
            _host_ptr = {{{ makeHEAPView('U16','ptr','ptr+size') }}};
            break;
          case webcl.UNSIGNED_INT32:
            _host_ptr = {{{ makeHEAPView('U32','ptr','ptr+size') }}};
            break;         
          default:
            _host_ptr = {{{ makeHEAPView('F32','ptr','ptr+size') }}};
            break;
        }
      } else {
        _host_ptr = {{{ makeHEAPView('F32','ptr','ptr+size') }}};
        
        // console.info("------");
        // _host_ptr = new DataView(new ArrayBuffer(size));

        // var _offset = 0;
        // for (var i = 0; i < type.length; i++) {
        //   var _type = type[i][0];
        //   var _num = type[i][1];
        //   switch(_type) {
        //     case webcl.SIGNED_INT8:
        //       _host_ptr.setInt8(_offset,{{{ makeHEAPView('8','ptr+_offset','ptr+_offset+_num') }}} );
        //       console.info("setInt8 : "+_offset+ " - "+(_offset+_num)+" / "+size );
        //       _offset += _num;
        //       break;
        //     case webcl.SIGNED_INT16:
        //       _host_ptr.setInt16(_offset,{{{ makeHEAPView('16','ptr+_offset','ptr+_offset+_num*2') }}} );
        //       console.info("setInt16 : "+_offset+ " - "+(_offset+_num*2)+" / "+size );
        //       _offset += 2*_num;
        //       break;
        //     case webcl.SIGNED_INT32:
        //       _host_ptr.setInt32(_offset,{{{ makeHEAPView('32','ptr+_offset','ptr+_offset+_num*4') }}} );
        //       console.info("setInt32 : "+_offset+ " - "+(_offset+_num*4)+" / "+size );
        //       _offset += 4*_num;
        //       break;
        //     case webcl.UNSIGNED_INT8:
        //       _host_ptr.setUint8(_offset,{{{ makeHEAPView('U8','ptr+_offset','ptr+_offset+_num') }}} );
        //       console.info("setUint8 : "+_offset+ " - "+(_offset+_num)+" / "+size );
        //       _offset += _num;
        //       break;
        //     case webcl.UNSIGNED_INT16:
        //       host_ptr.setUint16(_offset,{{{ makeHEAPView('U16','ptr+_offset','ptr+_offset+_num*2') }}} );
        //       console.info("setUint16 : "+_offset+ " - "+(_offset+_num*2)+" / "+size );
        //       _offset += 2*_num;
        //       break;
        //     case webcl.UNSIGNED_INT32:
        //       _host_ptr.setUint32(_offset,{{{ makeHEAPView('U32','ptr+_offset','ptr+_offset+_num*4') }}} );
        //       console.info("setUint32 : "+_offset+ " - "+(_offset+_num*4)+" / "+size );
        //       _offset += 4*_num;
        //       break;         
        //     default:
        //       _host_ptr.setFloat32(_offset,{{{ makeHEAPView('F32','ptr+_offset','ptr+_offset+_num*4') }}} );
        //       console.info("setFloat32 : "+_offset+ " - "+(_offset+_num*4)+" / "+size );
        //       _offset += 4*_num;
        //       break;
        //   }
        // }
      }

      return _host_ptr;
    },

    /**
     * Description
     * @method catchError
     * @param {} e
     * @return _error
     */
    catchError: function(e) {
      console.error(e);
      var _error = -1;

      if (e instanceof WebCLException) {
        var _str=e.message;
        var _n=_str.lastIndexOf(" ");
        _error = _str.substr(_n+1,_str.length-_n-1);
      }
      
      return _error;
    },

#if CL_GRAB_TRACE     
    stack_trace_offset: -1,
    stack_trace_complete: "// Javascript webcl Stack Trace\n(*) => all the stack_trace are print before the JS function call except for enqueueReadBuffer\n",
    stack_trace: "",

    /**
     * Description
     * @method webclBeginStackTrace
     * @param {} name
     * @param {} parameter
     * @return 
     */
    webclBeginStackTrace: function(name,parameter) {
      if (CL.stack_trace_offset == -1) {
        CL.stack_trace_offset = "";
      } else {
        CL.stack_trace_offset += "\t";
      }

      CL.stack_trace += "\n" + CL.stack_trace_offset + name + "("

      CL.webclCallParameterStackTrace(parameter);

      CL.stack_trace += ")\n";
    },
                                                              
    /**
     * Description
     * @method webclCallStackTrace
     * @param {} name
     * @param {} parameter
     * @return 
     */
    webclCallStackTrace: function(name,parameter) {
      CL.stack_trace += CL.stack_trace_offset + "\t->" + name + "("

      CL.webclCallParameterStackTrace(parameter);

      CL.stack_trace += ")\n";
    },

    /**
     * Description
     * @method webclCallParameterStackTrace
     * @param {} parameter
     * @return 
     */
    webclCallParameterStackTrace: function(parameter) {
      
      for (var i = 0; i < parameter.length - 1 ; i++) {
        if ( 
          (parameter[i] instanceof Uint8Array)    ||
          (parameter[i] instanceof Uint16Array)   ||
          (parameter[i] instanceof Uint32Array)   ||
          (parameter[i] instanceof Int8Array)     ||
          (parameter[i] instanceof Int16Array)    ||
          (parameter[i] instanceof Int32Array)    ||
          (parameter[i] instanceof Float32Array)  ||          
          (parameter[i] instanceof ArrayBuffer)   ||            
          (parameter[i] instanceof Array)){ 

          CL.stack_trace += "[";  
          for (var j = 0; j < Math.min(25,parameter[i].length - 1) ; j++) {
            CL.stack_trace += parameter[i][j] + ",";
          }
          if (parameter[i].length > 25) {
            CL.stack_trace += " ... ,";
          }
          if (parameter[i].length >= 1) {
            CL.stack_trace += parameter[i][parameter[i].length - 1];
          }
          CL.stack_trace += "],";
        } else {
          CL.stack_trace += parameter[i] + ",";  
        }
      }

      if (parameter.length >= 1) {
        if ( 
          (parameter[parameter.length - 1] instanceof Uint8Array)    ||
          (parameter[parameter.length - 1] instanceof Uint16Array)   ||
          (parameter[parameter.length - 1] instanceof Uint32Array)   ||
          (parameter[parameter.length - 1] instanceof Int8Array)     ||
          (parameter[parameter.length - 1] instanceof Int16Array)    ||
          (parameter[parameter.length - 1] instanceof Int32Array)    ||
          (parameter[parameter.length - 1] instanceof Float32Array)  ||          
          (parameter[parameter.length - 1] instanceof ArrayBuffer)   ||  
          (parameter[parameter.length - 1] instanceof Array)){ 

          CL.stack_trace += "[";  
          for (var j = 0; j < Math.min(25,parameter[parameter.length - 1].length - 1) ; j++) {
            CL.stack_trace += parameter[parameter.length - 1][j] + ",";
          }
          if (parameter[parameter.length - 1].length > 25) {
            CL.stack_trace += " ... ,";
          }
          if (parameter[parameter.length - 1].length >= 1) {
            CL.stack_trace += parameter[parameter.length - 1][parameter[parameter.length - 1].length - 1];
          }
          CL.stack_trace += "]";
        } else {
          CL.stack_trace += parameter[parameter.length - 1]; 
        }
      }
    },

    /**
     * Description
     * @method webclEndStackTrace
     * @param {} result
     * @param {} message
     * @param {} exception
     * @return 
     */
    webclEndStackTrace: function(result,message,exception) {
      CL.stack_trace += CL.stack_trace_offset + "\t\t=>Result (" + result[0];
      if (result.length >= 2) {
        CL.stack_trace += " : ";
      }

      for (var i = 1; i < result.length - 1 ; i++) {
        CL.stack_trace += ( result[i] == 0 ? '0' : {{{ makeGetValue('result[i]', '0', 'i32') }}} ) + " - ";
      }

      if (result.length >= 2) {
        CL.stack_trace +=  ( result[result.length - 1] == 0 ? '0' : {{{ makeGetValue('result[result.length - 1]', '0', 'i32') }}} );
      }

      CL.stack_trace += ") - Message (" + message + ") - Exception (" + exception + ")\n";

#if CL_PRINT_TRACE
      console.info(CL.stack_trace);
      //alert(CL.stack_trace); // Useful for step by step debugging
#endif   
      CL.stack_trace_complete += CL.stack_trace;
      CL.stack_trace = "";

      if (CL.stack_trace_offset == "") {
        CL.stack_trace_offset = -1;
      } else {
        CL.stack_trace_offset = CL.stack_trace_offset.substr(0,CL.stack_trace_offset.length-1);
      }
    },
#endif
  },

  /**
   * Description
   * @method webclPrintStackTrace
   * @param {} param_value
   * @param {} param_value_size
   * @return MemberExpression
   */
  webclPrintStackTrace: function(param_value,param_value_size) {
#if CL_GRAB_TRACE
    var _size = {{{ makeGetValue('param_value_size', '0', 'i32') }}} ;
    
    if (_size == 0) {
      {{{ makeSetValue('param_value_size', '0', 'CL.stack_trace_complete.length + 1', 'i32') }}} /* Size of char stack */;
    } else {
      writeStringToMemory(CL.stack_trace_complete, param_value);
    }
#else
    {{{ makeSetValue('param_value_size', '0', '0', 'i32') }}}
#endif    
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clSetTypePointer
   * @param {} pn_type
   * @param {} num_pn_type
   * @return MemberExpression
   */
  clSetTypePointer: function(pn_type, num_pn_type) {
    /*pn_type : CL_SIGNED_INT8,CL_SIGNED_INT16,CL_SIGNED_INT32,CL_UNSIGNED_INT8,CL_UNSIGNED_INT16,CL_UNSIGNED_INT32,CL_FLOAT*/
    
    // Clean
    CL.cl_pn_type = [];

#if CL_DEBUG    
    var _debug = "clSetTypePointer : ("+num_pn_type+") [";
#endif    

    var _old_pn_type = -1;
    var _num_pn_type = 0;
    for (var i = 0; i < num_pn_type ; i++) {
      var _pn_type = {{{ makeGetValue('pn_type', 'i*4', 'i32') }}}

      if (_pn_type != _old_pn_type) {
        if (_num_pn_type > 0)
          CL.cl_pn_type.push([_old_pn_type,_num_pn_type]);       

        _old_pn_type = _pn_type;
        _num_pn_type = 1;
      } else {
        _num_pn_type ++;
      }

#if CL_DEBUG    
      if (i > 0) {
        _debug += ",";
      }

      _debug += CL.stringType(_pn_type);
#endif

    }

    if (_num_pn_type > 0)
      CL.cl_pn_type.push([_old_pn_type,_num_pn_type]);       
  
#if CL_DEBUG    
    _debug += "]";
    console.info(_debug);
#endif

    return webcl.SUCCESS;
  },
  
  /**
   * Description
   * @method clGetPlatformIDs
   * @param {} num_entries
   * @param {} platforms
   * @param {} num_platforms
   * @return MemberExpression
   */
  clGetPlatformIDs: function(num_entries,platforms,num_platforms) {

#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetPlatformIDs",[num_entries,platforms,num_platforms]);
#endif

    // Init webcl variable if necessary
    if (CL.init() == 0) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],"webcl is not found !!!!","");
#endif
      return webcl.INVALID_VALUE;
    }

    if ( num_entries == 0 && platforms != 0) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],"num_entries is equal to zero and platforms is not NULL","");
#endif
      return webcl.INVALID_VALUE;
    }

    if ( num_platforms == 0 && platforms == 0) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],"both num_platforms and platforms are NULL","");
#endif
      return webcl.INVALID_VALUE;
    }

    var _platforms = null;

    try { 

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(webcl+".getPlatforms",[]);
#endif
      _platforms = webcl.getPlatforms();

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,platforms,num_platforms],"",e.message);
#endif
      return _error;
    }

    if (num_platforms != 0) {
      {{{ makeSetValue('num_platforms', '0', '_platforms.length', 'i32') }}} /* Num of platforms */;
    } 

    if (platforms != 0) {
      for (var i = 0; i < Math.min(num_entries,_platforms.length); i++) {
        var _id = CL.udid(_platforms[i]);
        {{{ makeSetValue('platforms', 'i*4', '_id', 'i32') }}};
      }
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,platforms,num_platforms],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetPlatformInfo
   * @param {} platform
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetPlatformInfo: function(platform,param_name,param_value_size,param_value,param_value_size_ret) {
    
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetPlatformInfo",[platform,param_name,param_value_size,param_value,param_value_size_ret]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(platform in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_PLATFORM],"platform are not in the map","");
#endif
      return webcl.INVALID_PLATFORM;
    }
#endif    
  
    var _info = null;
  
    try { 

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[platform]+".getInfo",[param_name]);
#endif        

      switch (param_name) {
        case 0x0902 /*CL_PLATFORM_NAME*/ :
          _info = "WEBCL_PLATFORM_NAME";
        break;
        case 0x0903 /*CL_PLATFORM_VENDOR*/ :
          _info = "WEBCL_PLATFORM_VENDOR";
        break;
          case 0x0904 /*CL_PLATFORM_EXTENSIONS*/ :
          _info = "WEBCL_PLATFORM_EXTENSIONS";
        break;
        default:
          _info = CL.cl_objects[platform].getInfo(param_name);  
      }      
    } catch (e) {
      
      var _error = CL.catchError(e);
      var _info = "undefined";

      if (param_value != 0) {
        writeStringToMemory(_info, param_value);
      }
  
      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '_info.length + 1', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

    if (param_name == webcl.PLATFORM_VERSION) _info += " "; 
    
    if (param_value != 0) {
      writeStringToMemory(_info, param_value);
    }
  
    if (param_value_size_ret != 0) {
      {{{ makeSetValue('param_value_size_ret', '0', '_info.length + 1', 'i32') }}};
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;

  },

  /**
   * Description
   * @method clGetDeviceIDs
   * @param {} platform
   * @param {} device_type_i64_1
   * @param {} device_type_i64_2
   * @param {} num_entries
   * @param {} devices
   * @param {} num_devices
   * @return MemberExpression
   */
  clGetDeviceIDs: function(platform,device_type_i64_1,device_type_i64_2,num_entries,devices,num_devices) {
    // Assume the device_type is i32 
    assert(device_type_i64_2 == 0, 'Invalid device_type i64');

#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetDeviceIDs",[platform,device_type_i64_1,num_entries,devices,num_devices]);
#endif
    
    // Init webcl variable if necessary
    if (CL.init() == 0) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],"webcl is not found !!!!","");
#endif
      return webcl.INVALID_VALUE;
    }

    if ( num_entries == 0 && devices != 0) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],"num_entries is equal to zero and device is not NULL","");
#endif
      return webcl.INVALID_VALUE;
    }

    if ( num_devices == 0 && devices == 0) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],"both num_devices and device are NULL","");
#endif
      return webcl.INVALID_VALUE;
    }

    if ( platform != 0 && !(platform in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_PLATFORM],"platform is not a valid platform","");
#endif
      return webcl.INVALID_PLATFORM;  
    }

    var _device = null;

    try {

      // If platform is NULL use the first platform found ...
      if (platform == 0) {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(webcl+".getPlatforms",[]);
#endif          
        var _platforms = webcl.getPlatforms();
        if (_platforms.length == 0) {
#if CL_GRAB_TRACE
          CL.webclEndStackTrace([webcl.INVALID_PLATFORM],"platform not found","");
#endif
          return webcl.INVALID_PLATFORM;  
        }

        // Create a new UDID 
        platform = CL.udid(_platforms[0]);
      } 

      var _platform = CL.cl_objects[platform];

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(_platform+".getDevices",[device_type_i64_1]);
#endif       
        
      _devices = _platform.getDevices(device_type_i64_1);

    } catch (e) {

      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,devices,num_devices],"",e.message);
#endif
      return _error;
    }

    if (num_devices != 0) {
      {{{ makeSetValue('num_devices', '0', '_devices.length', 'i32') }}} /* Num of device */;
    } 

    if (devices != 0) {
      for (var i = 0; i < Math.min(num_entries,_devices.length); i++) {
        var _id = CL.udid(_devices[i]);
        {{{ makeSetValue('devices', 'i*4', '_id', 'i32') }}};
      }
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,devices,num_devices],"","");
#endif
    return webcl.SUCCESS;

  },

  /**
   * Description
   * @method clGetDeviceInfo
   * @param {} device
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetDeviceInfo: function(device,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetDeviceInfo",[device,param_name,param_value_size,param_value,param_value_size_ret]);
#endif

#if CL_CHECK_VALID_OBJECT
      if (!(device in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_DEVICE],"device are not in the map","");
#endif
        return webcl.INVALID_DEVICE;
      }
#endif
  
    var  _info = null;

    try { 

        var _object = CL.cl_objects[device];

#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+_object+".getInfo",[param_name]);
#endif        
      switch (param_name) {
        case 0x1001 /*CL_DEVICE_VENDOR_ID*/ :
          _info = parseInt(CL.udid(_object));
        break;
        case 0x102B /*CL_DEVICE_NAME*/ :
          var _type = _object.getInfo(webcl.DEVICE_TYPE);
          switch (_type) {
            case webcl.DEVICE_TYPE_CPU:
              _info = "WEBCL_DEVICE_CPU";
            break;
            case webcl.DEVICE_TYPE_GPU:
              _info = "WEBCL_DEVICE_GPU";
            break;
            case webcl.DEVICE_TYPE_ACCELERATOR:
              _info = "WEBCL_DEVICE_ACCELERATOR";
            break;
            case webcl.DEVICE_TYPE_DEFAULT:
              _info = "WEBCL_DEVICE_DEFAULT";
            break;
          }
        break;
        case 0x102C /*CL_DEVICE_VENDOR*/ :
          _info = "WEBCL_DEVICE_VENDOR";
        break;
        case 0x1030 /*CL_DEVICE_EXTENSIONS*/ :
          _info = webcl.getSupportedExtensions().join(' ') ; 
        break;
        case 0x101A /*CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE*/ :
          _info = _object.getInfo(webcl.DEVICE_MEM_BASE_ADDR_ALIGN) >> 3;
        break;
        default:
          _info = _object.getInfo(param_name);
      }  
    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {
        {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      }
    
      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }
        
    if(typeof(_info) == "number") {

      if (param_value_size == 8) {
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i64') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '8', 'i32') }}};
      } else {
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};
      } 
      
    } else if(typeof(_info) == "boolean") {

      if (param_value != 0) (_info == true) ? {{{ makeSetValue('param_value', '0', '1', 'i32') }}} : {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } else if(typeof(_info) == "string") {

      _info += " ";

      if (param_value != 0) writeStringToMemory(_info, param_value);
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '_info.length + 1', 'i32') }}};

    } else if(typeof(_info) == "object") {
      
      if (_info instanceof Int32Array) {
       
        for (var i = 0; i < Math.min(param_value_size>>2,_info.length); i++) {
          if (param_value != 0) {{{ makeSetValue('param_value', 'i*4', '_info[i]', 'i32') }}};
        }
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '_info.length * 4', 'i32') }}};
      
      } else if (_info instanceof WebCLPlatform) {
     
        var _id = CL.udid(_info);
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_id', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};
      
      } else if (_info == null) {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      } else {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
        return webcl.INVALID_VALUE;
      }
    } else {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
      return webcl.INVALID_VALUE;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clCreateContext
   * @param {} properties
   * @param {} num_devices
   * @param {} devices
   * @param {} pfn_notify
   * @param {} user_data
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateContext: function(properties,num_devices,devices,pfn_notify,user_data,cl_errcode_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateContext",[properties,num_devices,devices,pfn_notify,user_data,cl_errcode_ret]);
#endif

    // Init webcl variable if necessary
    if (CL.init() == 0) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"Can't initialise webcl","");
#endif
      return 0; // NULL Pointer      
    }
    
    var _id = null;
    var _context = null;

    try { 

      var _platform = null;
      var _devices = [];
      var _glclSharedContext = false;

      // Verify the device, theorically on OpenCL there are CL_INVALID_VALUE when devices or num_devices is null,
      // WebCL can work using default device / platform, we check only if parameter are set.
      for (var i = 0; i < num_devices; i++) {
        var _idxDevice = {{{ makeGetValue('devices', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT        
        if (_idxDevice in CL.cl_objects) {
#endif          
          _devices.push(CL.cl_objects[_idxDevice]);
#if CL_CHECK_VALID_OBJECT        
        } else {
          if (cl_errcode_ret != 0) {
            {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_DEVICE', 'i32') }}};
          }

#if CL_GRAB_TRACE
          CL.webclEndStackTrace([0,cl_errcode_ret],"devices contains an invalid device","");
#endif
          return 0;  
        }
#endif        
      }

      // Verify the property
      var _propertiesCounter = 0;
      var _properties = [];

      if (properties != 0) {
        while(1) {
          var _readprop = {{{ makeGetValue('properties', '_propertiesCounter*4', 'i32') }}};
          _properties.push(_readprop);

          if (_readprop == 0) break;

          switch (_readprop) {
            case webcl.CONTEXT_PLATFORM:
              _propertiesCounter ++;
              var _idxPlatform = {{{ makeGetValue('properties', '_propertiesCounter*4', 'i32') }}};
              _properties.push(_idxPlatform);

#if CL_CHECK_VALID_OBJECT              
              if (_idxPlatform in CL.cl_objects) {
#endif                
                _platform = CL.cl_objects[_idxPlatform];
#if CL_CHECK_VALID_OBJECT
              } else {
                if (cl_errcode_ret != 0) {
                  {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_PLATFORM', 'i32') }}};
                }

#if CL_GRAB_TRACE
                CL.webclEndStackTrace([0,cl_errcode_ret],"platform value specified in properties is not a valid platform","");
#endif
                return 0;  
              }
#endif              
              break;

            // /!\ This part, it's for the CL_GL_Interop
            case (0x200A) /*CL_GLX_DISPLAY_KHR*/:
            case (0x2008) /*CL_GL_CONTEXT_KHR*/:
            case (0x200C) /*CL_CGL_SHAREGROUP_KHR*/:            
              _propertiesCounter ++;
              _glclSharedContext = true;
              
              break;

            default:
              if (cl_errcode_ret != 0) {
                {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_PROPERTY', 'i32') }}};
              }

#if CL_GRAB_TRACE
              CL.webclEndStackTrace([0,cl_errcode_ret],"context property name '"+_readprop+"' in properties is not a supported property name","");
#endif
              return 0; 
          };

          _propertiesCounter ++;
        }
      }

      if (num_devices > 0) {
 
        if (_glclSharedContext) {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(webcl+".createContext",[Module.ctx, _devices]);
#endif          
          _context = webcl.createContext(Module.ctx,_devices);  
        } else {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(webcl+".createContext",[_devices]);
#endif          
          _context = webcl.createContext(_devices);  
        }

      } else if (_platform != null) {
        
        if (_glclSharedContext) {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(webcl+".createContext",[Module.ctx, _platform]);
#endif          
          _context = webcl.createContext(Module.ctx,_platform);  
        } else {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(webcl+".createContext",[_platform]);
#endif          
          _context = webcl.createContext(_platform);  
        }

      } else {

        if (cl_errcode_ret != 0) {
          {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
        }

#if CL_GRAB_TRACE
        CL.webclEndStackTrace([0,cl_errcode_ret],"webcl can't create context !!!!","");
#endif
        return 0; // NULL Pointer      
      }

    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    }

    _id = CL.udid(_context);

    // Add properties array for getInfo
    Object.defineProperty(_context, "properties", { value : _properties,writable : false });

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clCreateContextFromType
   * @param {} properties
   * @param {} device_type_i64_1
   * @param {} device_type_i64_2
   * @param {} pfn_notify
   * @param {} user_data
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateContextFromType: function(properties,device_type_i64_1,device_type_i64_2,pfn_notify,user_data,cl_errcode_ret) {
    // Assume the device_type is i32 
    assert(device_type_i64_2 == 0, 'Invalid device_type i64');
    
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateContextFromType",[properties,device_type_i64_1,pfn_notify,user_data,cl_errcode_ret]);
#endif

    // Init webcl variable if necessary
    if (CL.init() == 0) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"Can't initialise webcl","");
#endif
      return 0; // NULL Pointer      
    }

    var _id = null;
    var _context = null;

    try { 

      var _platform = null;
      var _deviceType = device_type_i64_1;
      var _glclSharedContext = false;
      var _properties = [];

      // Verify the property
      if (properties != 0) {
        var _propertiesCounter = 0;
        while(1) {
          var _readprop = {{{ makeGetValue('properties', '_propertiesCounter*4', 'i32') }}};
          _properties.push(_readprop);

          if (_readprop == 0) break;

          switch (_readprop) {
            case webcl.CONTEXT_PLATFORM:
              _propertiesCounter ++;
              var _idxPlatform = {{{ makeGetValue('properties', '_propertiesCounter*4', 'i32') }}};
              _properties.push(_idxPlatform);

#if CL_CHECK_VALID_OBJECT              
              if (_idxPlatform in CL.cl_objects) {
#endif                
                _platform = CL.cl_objects[_idxPlatform];
#if CL_CHECK_VALID_OBJECT      
              } else {
                if (cl_errcode_ret != 0) {
                  {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_PLATFORM', 'i32') }}};
                }

#if CL_GRAB_TRACE
                CL.webclEndStackTrace([0,cl_errcode_ret],"platform value specified in properties is not a valid platform","");
#endif
                return 0;  
              }
#endif              
              break;

            // /!\ This part, it's for the CL_GL_Interop
            case (0x200A) /*CL_GLX_DISPLAY_KHR*/:
            case (0x2008) /*CL_GL_CONTEXT_KHR*/:
            case (0x200C) /*CL_CGL_SHAREGROUP_KHR*/:            
              _propertiesCounter ++;
              _glclSharedContext = true;
              break;

            default:
              if (cl_errcode_ret != 0) {
                {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_PROPERTY', 'i32') }}};
              }

#if CL_GRAB_TRACE
              CL.webclEndStackTrace([0,cl_errcode_ret],"context property name '"+_readprop+"' in properties is not a supported property name","");
#endif
              return 0; 
          };

          _propertiesCounter ++;
        }
      }

      if (_deviceType != 0 && _platform != null) {

        if (_glclSharedContext) {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(webcl+".createContext",[Module.ctx, _platform,_deviceType]);
#endif          
          _context = webcl.createContext(Module.ctx, _platform,_deviceType);  
        } else {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(webcl+".createContext",[_platform,_deviceType]);
#endif          
          _context = webcl.createContext(_platform,_deviceType);  
        }
            
      } else if (_deviceType != 0) {

        if (_glclSharedContext) {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(webcl+".createContext",[Module.ctx, _deviceType]);
#endif          
          _context = webcl.createContext(Module.ctx,_deviceType);  
        } else {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(webcl+".createContext",[_deviceType]);
#endif          
          _context = webcl.createContext(_deviceType);  
        }

      } else {
        if (cl_errcode_ret != 0) {
          {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
        }

#if CL_GRAB_TRACE
        CL.webclEndStackTrace([0,cl_errcode_ret],"webcl can't create context !!!!","");
#endif
        return 0; // NULL Pointer   
      }
   
    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    }

    _id = CL.udid(_context);

    // Add properties array for getInfo
    Object.defineProperty(_context, "properties", { value : _properties,writable : false });

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clRetainContext
   * @param {} context
   * @return MemberExpression
   */
  clRetainContext: function(context) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clRetainContext",[context]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(context in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_CONTEXT],CL.cl_objects[context]+" is not a valid OpenCL context","");
#endif
      return webcl.INVALID_CONTEXT;
    }
#endif 

    if (!(context in CL.cl_objects_retains)) {
      CL.cl_objects_retains[context] = 1;
    } else {
      CL.cl_objects_retains[context] = CL.cl_objects_retains[context] + 1;
    }
       
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clReleaseContext
   * @param {} context
   * @return MemberExpression
   */
  clReleaseContext: function(context) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clReleaseContext",[context]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(context in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_CONTEXT],CL.cl_objects[context]+" is not a valid OpenCL context","");
#endif
      return webcl.INVALID_CONTEXT;
    }
#endif  

    // If is an object retain don't release it until retains > 0...
    if (context in CL.cl_objects_retains) {

      var _retain = CL.cl_objects_retains[context] - 1;

      CL.cl_objects_retains[context] = _retain;

      if (_retain >= 0) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif      
        return webcl.SUCCESS;
      }
    }

    try {

#if CL_GRAB_TRACE
        CL.webclCallStackTrace(CL.cl_objects[context]+".release",[]);
#endif        
        CL.cl_objects[context].release();
        delete CL.cl_objects[context];     

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetContextInfo
   * @param {} context
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetContextInfo: function(context,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetContextInfo",[context,param_name,param_value_size,param_value,param_value_size_ret]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(context in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_CONTEXT],CL.cl_objects[context]+" is not a valid OpenCL context","");
#endif
      return webcl.INVALID_CONTEXT;
    }
#endif 

    var _info = null;

    try { 

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[context]+".getInfo",[param_name]);
#endif   

      if (param_name == 0x1080 /* CL_CONTEXT_REFERENCE_COUNT */) {
        _info = 0;

        if (context in CL.cl_objects) {
          _info++;
        }

        if (context in CL.cl_objects_retains) {
          _info+=CL.cl_objects_retains[context];
        }

      }  else if (param_name == 0x1082 /* CL_CONTEXT_PROPERTIES */) {
      
        _info = "WebCLContextProperties";

      } else {

        _info = CL.cl_objects[context].getInfo(param_name);

      }
      

    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {
        {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      }
    
      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }
    
     if (_info == "WebCLContextProperties") {

      var _size = 0;

      if (param_value != 0) {

        if ( CL.cl_objects[context].hasOwnProperty('properties') ) {
          var _properties = CL.cl_objects[context].properties;

          for (elt in _properties) {
            {{{ makeSetValue('param_value', '_size*4', '_properties[elt]', 'i32') }}};
            _size ++;

          }
        }
      }

      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '_size*4', 'i32') }}};

    } else if(typeof(_info) == "number") {

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } else if(typeof(_info) == "boolean") {

      if (param_value != 0) (_info == true) ? {{{ makeSetValue('param_value', '0', '1', 'i32') }}} : {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } else if(typeof(_info) == "object") {

      if (_info instanceof WebCLPlatform) {
     
        var _id = CL.udid(_info);
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_id', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

      } else if (_info instanceof Array) {

        for (var i = 0; i < Math.min(param_value_size>>2,_info.length); i++) {
          var _id = CL.udid(_info[i]);
          if (param_value != 0) {{{ makeSetValue('param_value', 'i*4', '_id', 'i32') }}};
        }
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '_info.length*4', 'i32') }}};

      } else if (_info == null) {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      } else {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
        return webcl.INVALID_VALUE;
      }
    } else {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
      return webcl.INVALID_VALUE;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clCreateCommandQueue
   * @param {} context
   * @param {} device
   * @param {} properties_1
   * @param {} properties_2
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateCommandQueue: function(context,device,properties_1,properties_2,cl_errcode_ret) {
    // Assume the properties is i32 
    assert(properties_2 == 0, 'Invalid properties i64');

#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateCommandQueue",[context,device,properties_1,cl_errcode_ret]);
#endif

    var _id = null;
    var _command = null;

    // Context must be created
#if CL_CHECK_VALID_OBJECT    
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"context '"+context+"' is not a valid context","");
#endif
      return 0; 
    }
#endif    

    // Context must be created
#if CL_CHECK_VALID_OBJECT    
    if (!(device in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_DEVICE', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"device '"+device+"' is not a valid device","");
#endif
      return 0; 
    }
#endif    

    try { 

#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[context]+".createCommandQueue",[CL.cl_objects[device],properties_1]);
#endif      

      _command = CL.cl_objects[context].createCommandQueue(CL.cl_objects[device],properties_1);

    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    }

    _id = CL.udid(_command);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clRetainCommandQueue
   * @param {} command_queue
   * @return MemberExpression
   */
  clRetainCommandQueue: function(command_queue) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clRetainCommandQueue",[command_queue]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],CL.cl_objects[context]+" is not a valid OpenCL command_queue","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 

    if (!(command_queue in CL.cl_objects_retains)) {
      CL.cl_objects_retains[command_queue] = 1;
    } else {
      CL.cl_objects_retains[command_queue] = CL.cl_objects_retains[command_queue] + 1;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clReleaseCommandQueue
   * @param {} command_queue
   * @return MemberExpression
   */
  clReleaseCommandQueue: function(command_queue) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clReleaseCommandQueue",[command_queue]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],CL.cl_objects[command_queue]+" is not a valid OpenCL command_queue","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif

    // If is an object retain don't release it until retains > 0...
    if (command_queue in CL.cl_objects_retains) {

      var _retain = CL.cl_objects_retains[command_queue] - 1;

      CL.cl_objects_retains[command_queue] = _retain;

      if (_retain >= 0) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif      
        return webcl.SUCCESS;
      }
    }

    try {

#if CL_GRAB_TRACE
        CL.webclCallStackTrace(CL.cl_objects[command_queue]+".release",[]);
#endif        
        CL.cl_objects[command_queue].release();
        delete CL.cl_objects[command_queue];  

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetCommandQueueInfo
   * @param {} command_queue
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetCommandQueueInfo: function(command_queue,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetCommandQueueInfo",[command_queue,param_name,param_value_size,param_value,param_value_size_ret]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],CL.cl_objects[command_queue]+" is not a valid OpenCL command_queue","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif

    var _info = null;

    try { 

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".getInfo",[param_name]);
#endif        

      if (param_name == 0x1092 /* CL_QUEUE_REFERENCE_COUNT */) {
        _info = 0;

        if (command_queue in CL.cl_objects) {
          _info++;
        }

        if (command_queue in CL.cl_objects_retains) {
          _info+=CL.cl_objects_retains[command_queue];
        }

      } else {
        _info = CL.cl_objects[command_queue].getInfo(param_name);
      }

    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {
        {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      }
    
      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

    if(typeof(_info) == "number") {

      if (param_value_size == 8) {
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i64') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '8', 'i32') }}};            
      } else {
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};            
      } 

    } else if(typeof(_info) == "object") {

      if ( (_info instanceof WebCLDevice) || (_info instanceof WebCLContext)) {
     
        var _id = CL.udid(_info);
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_id', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

      } else if (_info == null) {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      } else {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
        return webcl.INVALID_VALUE;
      }
    } else {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
      return webcl.INVALID_VALUE;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clCreateBuffer
   * @param {} context
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} size
   * @param {} host_ptr
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateBuffer__deps: ['clEnqueueWriteBuffer'],
  clCreateBuffer: function(context,flags_i64_1,flags_i64_2,size,host_ptr,cl_errcode_ret) {
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');
    
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateBuffer",[context,flags_i64_1,size,host_ptr,cl_errcode_ret]);
#endif
#if CL_CHECK_SET_POINTER    
    if (CL.cl_pn_type.length == 0 && host_ptr != 0) console.info("/!\\ clCreateBuffer : you don't call clSetTypePointer for host_ptr parameter");
#endif

    var _id = null;
    var _buffer = null;

    // Context must be created
#if CL_CHECK_VALID_OBJECT    
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"context '"+context+"' is not a valid context","");
#endif
      return 0; 
    }
#endif
    
    var _flags;

    if (flags_i64_1 & webcl.MEM_READ_WRITE) {
      _flags = webcl.MEM_READ_WRITE;
    } else if (flags_i64_1 & webcl.MEM_WRITE_ONLY) {
      _flags = webcl.MEM_WRITE_ONLY;
    } else if (flags_i64_1 & webcl.MEM_READ_ONLY) {
      _flags = webcl.MEM_READ_ONLY;
    } else {
      _flags |= webcl.MEM_READ_WRITE;
    }

    var _host_ptr = null;

    if ( host_ptr != 0 ) _host_ptr = CL.getCopyPointerToArray(host_ptr,size,CL.cl_pn_type); 
    else if (
      (flags_i64_1 & (1 << 4) /* CL_MEM_ALLOC_HOST_PTR  */) ||
      (flags_i64_1 & (1 << 5) /* CL_MEM_COPY_HOST_PTR   */) ||
      (flags_i64_1 & (1 << 3) /* CL_MEM_USE_HOST_PTR    */)
      ) {
      _host_ptr = CL.getHostPtrArray(size,CL.cl_pn_type);
    } 

    try {

#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[context]+".createBuffer",[_flags,size,_host_ptr]);
#endif      
    
      if (_host_ptr != null) {
        _buffer = CL.cl_objects[context].createBuffer(_flags,size,_host_ptr);
      } else
        _buffer = CL.cl_objects[context].createBuffer(_flags,size);

    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
      }
      
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    }

    _id = CL.udid(_buffer);

    // \todo need to be remove when firefox will be support hot_ptr
    /**** **** **** **** **** **** **** ****/
    if (_host_ptr != null) {
      if (navigator.userAgent.toLowerCase().indexOf('firefox') != -1) {
        // Search command
        var commandqueue = null;
        for (var obj in CL.cl_objects) {
          if (CL.cl_objects[obj] instanceof WebCLCommandQueue) {
            commandqueue = CL.cl_objects[obj];
            break;
          }
        }
        
        if (commandqueue != null) {
          _clEnqueueWriteBuffer(obj,_id,true,0,size,host_ptr,0,0,0);
        } else {
          if (cl_errcode_ret != 0) {
            {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
          }

#if CL_CHECK_SET_POINTER    
          CL.cl_pn_type = [];
#endif
#if CL_GRAB_TRACE
          CL.webclEndStackTrace([0,cl_errcode_ret],"Firefox doesn't support host_ptr (Not found command queue)","");
#endif
          return 0; 
        }
      }
    }
    /**** **** **** **** **** **** **** ****/

#if CL_CHECK_SET_POINTER    
    CL.cl_pn_type = [];
#endif    
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clCreateSubBuffer
   * @param {} buffer
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} buffer_create_type
   * @param {} buffer_create_info
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateSubBuffer: function(buffer,flags_i64_1,flags_i64_2,buffer_create_type,buffer_create_info,cl_errcode_ret) {
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');
    
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateSubBuffer",[buffer,flags_i64_1,buffer_create_type,buffer_create_info,cl_errcode_ret]);
#endif

    var _id = null;
    var _subbuffer = null;

    // Context must be created
#if CL_CHECK_VALID_OBJECT    
    if (!(buffer in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_MEM_OBJECT', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"Mem object '"+buffer+"' is not a valid buffer","");
#endif
      return 0; 
    }
#endif
    
    var _flags;
    var _origin;
    var _sizeInBytes;

    if (flags_i64_1 & webcl.MEM_READ_WRITE) {
      _flags = webcl.MEM_READ_WRITE;
    } else if (flags_i64_1 & webcl.MEM_WRITE_ONLY) {
      _flags = webcl.MEM_WRITE_ONLY;
    } else if (flags_i64_1 & webcl.MEM_READ_ONLY) {
      _flags = webcl.MEM_READ_ONLY;
    } else {
      _flags |= webcl.MEM_READ_WRITE;
    }
  
    if (flags_i64_1 & ~_flags) {
      console.error("clCreateSubBuffer : This flag is not yet implemented => "+(flags_i64_1 & ~_flags));
    }

    if (buffer_create_info != 0) {
      _origin = {{{ makeGetValue('buffer_create_info', '0', 'i32') }}};
      _sizeInBytes = {{{ makeGetValue('buffer_create_info', '4', 'i32') }}};
    } else {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"buffer_create_info is NULL","");
#endif

      return 0; 
    }

    try {

#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[buffer]+".createSubBuffer",[_flags,_origin,_sizeInBytes]);
#endif      

      _subbuffer = CL.cl_objects[buffer].createSubBuffer(_flags,_origin,_sizeInBytes);

    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    }

    _id = CL.udid(_subbuffer);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clCreateImage2D
   * @param {} context
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} image_format
   * @param {} image_width
   * @param {} image_height
   * @param {} image_row_pitch
   * @param {} host_ptr
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateImage2D: function(context,flags_i64_1,flags_i64_2,image_format,image_width,image_height,image_row_pitch,host_ptr,cl_errcode_ret) {
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');
    
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateImage2D",[context,flags_i64_1,image_format,image_width,image_height,image_row_pitch,host_ptr,cl_errcode_ret]);
#endif

    var _id = null;
    var _image = null;

    // Context must be created
#if CL_CHECK_VALID_OBJECT    
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"context '"+context+"' is not a valid context","");
#endif
      return 0; 
    }
#endif    
    
    var _flags;

    if (flags_i64_1 & webcl.MEM_READ_WRITE) {
      _flags = webcl.MEM_READ_WRITE;
    } else if (flags_i64_1 & webcl.MEM_WRITE_ONLY) {
      _flags = webcl.MEM_WRITE_ONLY;
    } else if (flags_i64_1 & webcl.MEM_READ_ONLY) {
      _flags = webcl.MEM_READ_ONLY;
    } else {
      _flags |= webcl.MEM_WRITE_ONLY
    }

    var _host_ptr = null;
    var _channel_order = webcl.RGBA;
    var _channel_type = webcl.UNORM_INT8;

    if (image_format != 0) {
      _channel_order = {{{ makeGetValue('image_format', '0', 'i32') }}};
      _channel_type = {{{ makeGetValue('image_format', '4', 'i32') }}};
    } else {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_IMAGE_FORMAT_DESCRIPTOR', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"image_format is NULL","");
#endif

      return 0; 
    }

    var _type = webcl.FLOAT;
    var _sizeType = 4;
    var _sizeOrder = 1;    

    switch (_channel_type) {
      case webcl.SNORM_INT8:
      case webcl.SIGNED_INT8:
        _sizeType = 1;
        _type = webcl.SIGNED_INT8;
        break;
      case webcl.UNORM_INT8:        
      case webcl.UNSIGNED_INT8:
        _sizeType = 1;
        _type = webcl.UNSIGNED_INT8;
        break;
      case webcl.SNORM_INT16:
      case webcl.SIGNED_INT16:
        _sizeType = 2;
        _type = webcl.SIGNED_INT16;
        break;
      case webcl.UNORM_INT16:        
      case webcl.UNSIGNED_INT16:
      case webcl.HALF_FLOAT:
        _sizeType = 2;      
        _type = webcl.UNSIGNED_INT16;
        break;
      case webcl.SIGNED_INT32:
        _sizeType = 4;
        _type = webcl.SIGNED_INT32;
      case webcl.UNSIGNED_INT32:
        _sizeType = 4;
        _type = webcl.UNSIGNED_INT32;
        break;        
      case webcl.FLOAT:
        _sizeType = 4;
        _type = webcl.FLOAT;
        break;
      default:
        console.error("clCreateImage2D : This channel type is not yet implemented => "+_channel_type);
    }

    switch (_channel_order) {
      case webcl.R:
      case webcl.A:
      case webcl.INTENSITY:
      case webcl.LUMINANCE:
        _sizeOrder = 1;
        break;
      case webcl.RG:
      case webcl.RA:
        _sizeOrder = 2;
        break;
      case webcl.RGB:
        _sizeOrder = 3;
        break; 
      case webcl.RGBA:
      case webcl.BGRA:
      case webcl.ARGB:      
        _sizeOrder = 4;
        break;        
      default:
        console.error("clCreateImage2D : This channel order is not yet implemented => "+_channel_order);
    }

    var _size = image_width * image_height * _sizeOrder;

    // console.info("/!\\ clCreateImage2D : Compute the size of ptr with image Info '"+_size+"'... need to be more tested");

    if ( host_ptr != 0 ) _host_ptr = CL.getCopyPointerToArray(host_ptr,_size,_type); 
    else if (
      (flags_i64_1 & (1 << 4) /* CL_MEM_ALLOC_HOST_PTR  */) ||
      (flags_i64_1 & (1 << 5) /* CL_MEM_COPY_HOST_PTR   */) ||
      (flags_i64_1 & (1 << 3) /* CL_MEM_USE_HOST_PTR    */)
      ) {
      _host_ptr = CL.getHostPtrArray(_size,_type);
    } 

    var _descriptor = { channelOrder:_channel_order, channelType:_channel_type, width:image_width, height:image_height, rowPitch:image_row_pitch }

    try {

#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[context]+".createImage",[_flags,"{channelOrder:"+_channel_order+", channelType:"+_channel_type+", width:"+image_width+", height:"+image_height+", rowPitch:"+image_row_pitch +"}",_host_ptr]);
#endif      

      if (_host_ptr != null)
        _image = CL.cl_objects[context].createImage(_flags,_descriptor,_host_ptr);
      else
        _image = CL.cl_objects[context].createImage(_flags,_descriptor);

    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    }

    _id = CL.udid(_image);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clCreateImage3D
   * @param {} context
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} image_format
   * @param {} image_width
   * @param {} image_height
   * @param {} image_depth
   * @param {} image_row_pitch
   * @param {} image_slice_pitch
   * @param {} host_ptr
   * @param {} cl_errcode_ret
   * @return Literal
   */
  clCreateImage3D: function(context,flags_i64_1,flags_i64_2,image_format,image_width,image_height,image_depth,image_row_pitch,image_slice_pitch,host_ptr,cl_errcode_ret) {
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');
    
    console.error("clCreateImage3D: Can't be implemented - Differences between WebCL and OpenCL 1.1\n");

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
    }

    return 0;
  },

  /**
   * Description
   * @method clRetainMemObject
   * @param {} memobj
   * @return MemberExpression
   */
  clRetainMemObject: function(memobj) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clRetainMemObject",[memobj]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(memobj in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],CL.cl_objects[memobj]+" is not a valid OpenCL memobj","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    if (!(memobj in CL.cl_objects_retains)) {
      CL.cl_objects_retains[memobj] = 1;
    } else {
      CL.cl_objects_retains[memobj] = CL.cl_objects_retains[memobj] + 1;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif        

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clReleaseMemObject
   * @param {} memobj
   * @return MemberExpression
   */
  clReleaseMemObject: function(memobj) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clReleaseMemObject",[memobj]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(memobj in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],CL.cl_objects[memobj]+" is not a valid OpenCL memobj","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif

    // If is an object retain don't release it until retains > 0...
    if (memobj in CL.cl_objects_retains) {

      var _retain = CL.cl_objects_retains[memobj] - 1;

      CL.cl_objects_retains[memobj] = _retain;

      if (_retain >= 0) {
        
        // Call the callback 
        if (memobj in CL.cl_objects_mem_callback) {
          if (CL.cl_objects_mem_callback[memobj].length > 0)
            CL.cl_objects_mem_callback[memobj].pop()();
        }

#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif      
        return webcl.SUCCESS;
      }
    }

    try {

      // Call the callback 
      if (memobj in CL.cl_objects_mem_callback) {
        if (CL.cl_objects_mem_callback[memobj].length > 0)
          CL.cl_objects_mem_callback[memobj].pop()();
      }

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(CL.cl_objects[memobj]+".release",[]);
#endif        
      CL.cl_objects[memobj].release();
      delete CL.cl_objects[memobj];  

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetSupportedImageFormats
   * @param {} context
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} image_type
   * @param {} num_entries
   * @param {} image_formats
   * @param {} num_image_formats
   * @return MemberExpression
   */
  clGetSupportedImageFormats: function(context,flags_i64_1,flags_i64_2,image_type,num_entries,image_formats,num_image_formats) {
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');

#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetSupportedImageFormats",[context,flags_i64_1,image_type,num_entries,image_formats,num_image_formats]);
#endif

    // Context must be created
#if CL_CHECK_VALID_OBJECT    
    if (!(context in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_CONTEXT],"context '"+context+"' is not a valid context","");
#endif
      return webcl.INVALID_CONTEXT; 
    }
#endif
    if (image_type != webcl.MEM_OBJECT_IMAGE2D) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],"image_type "+image_type+" are not valid","");
#endif
      return webcl.INVALID_VALUE;       
    }
    
    var _flags;

    if (flags_i64_1 & webcl.MEM_READ_WRITE) {
      _flags = webcl.MEM_READ_WRITE;
    } else if (flags_i64_1 & webcl.MEM_WRITE_ONLY) {
      _flags = webcl.MEM_WRITE_ONLY;
    } else if (flags_i64_1 & webcl.MEM_READ_ONLY) {
      _flags = webcl.MEM_READ_ONLY;
    } else {

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],"values specified "+flags_i64_1+" in flags are not valid","");
#endif

      return webcl.INVALID_VALUE; 
    }

    if (flags_i64_1 & ~_flags) {
      console.error("clGetSupportedImageFormats : This flag is not yet implemented => "+(flags_i64_1 & ~_flags));
    }

    var _descriptor_list = null;

    try {

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(CL.cl_objects[context]+".getSupportedImageFormats",[_flags]);
#endif        

      _descriptor_list = CL.cl_objects[context].getSupportedImageFormats(_flags);

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

    var _counter = 0;
    for (var i = 0; i < Math.min(num_entries,_descriptor_list.length); i++) {
      var _descriptor = _descriptor_list[i];

      if (image_formats != 0) {
        {{{ makeSetValue('image_formats', '_counter*4', '_descriptor.channelOrder', 'i32') }}};
        _counter++;
        {{{ makeSetValue('image_formats', '_counter*4', '_descriptor.channelType', 'i32') }}};
        _counter++;
      }
    }

    if (num_image_formats != 0) {
      {{{ makeSetValue('num_image_formats', '0', '_descriptor_list.length', 'i32') }}};
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetMemObjectInfo
   * @param {} memobj
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetMemObjectInfo: function(memobj,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetMemObjectInfo",[memobj,param_name,param_value_size,param_value,param_value_size_ret]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(memobj in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],CL.cl_objects[memobj]+" is not a valid OpenCL memobj","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif

    var _info = null;

    try { 

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[memobj]+".getInfo",[param_name]);
#endif        
      
      if (param_name == 0x1105 /*CL_MEM_REFERENCE_COUNT*/) {

        _info = 0;

        if (memobj in CL.cl_objects) {
          _info++;
        }

        if (memobj in CL.cl_objects_retains) {
          _info+=CL.cl_objects_retains[memobj];
        }

      } else if (param_name == 0x1104 /*CL_MEM_MAP_COUNT*/) {
        
        _info = 0;

      } else {
        _info = CL.cl_objects[memobj].getInfo(param_name);  
      }
      

    } catch (e) {

      var _error = CL.catchError(e);

      if (param_value != 0) {
        {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      }
    
      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

    if(typeof(_info) == "number") {

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } else if(typeof(_info) == "object") {

      if (_info instanceof WebCLImageDescriptor) {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', 'webcl.MEM_OBJECT_IMAGE2D', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

      } else if (_info instanceof WebCLBuffer) {
     
        var _id = CL.udid(_info);
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_id', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

      } else if (_info == null) {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      } else {
        console.error("clGetMemObjectInfo : "+typeof(_info)+" not yet implemented");
      }
    } else {
      console.error("clGetMemObjectInfo : "+typeof(_info)+" not yet implemented");
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetImageInfo
   * @param {} image
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetImageInfo: function(image,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetImageInfo",[image,param_name,param_value_size,param_value,param_value_size_ret]);
#endif

#if CL_CHECK_VALID_OBJECT    
    if (!(image in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"image '"+image+"' is not a valid image","");
#endif
      return webcl.INVALID_MEM_OBJECT; 
    }
#endif

    var _info = null;

    try { 

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[image]+".getInfo",[param_name]);
#endif        

      _info = CL.cl_objects[image].getInfo(param_name);

    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {
        {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      }
    
      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

    var _sizeType = CL.getImageSizeType(image);
    
    switch (param_name) {
      case (webcl.IMAGE_FORMAT) :
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info.channelOrder', 'i32') }}};
        if (param_value != 0) {{{ makeSetValue('param_value', '4', '_info.channelType', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '8', 'i32') }}};
        break;
      case (webcl.IMAGE_ELEMENT_SIZE) :
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_sizeType', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};
        break;
      case (webcl.IMAGE_ROW_PITCH) :
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info.rowPitch', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};
        break;
      case (webcl.IMAGE_WIDTH) :
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info.width', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};
        break;
      case (webcl.IMAGE_HEIGHT) :
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info.height', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};
        break;
      default:
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_VALUE],param_name+" not yet implemente","");
#endif
        return webcl.INVALID_VALUE;
    } 

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clSetMemObjectDestructorCallback
   * @param {} memobj
   * @param {} pfn_notify
   * @param {} user_data
   * @return MemberExpression
   */
  clSetMemObjectDestructorCallback: function(memobj,pfn_notify,user_data) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clSetMemObjectDestructorCallback",[memobj,pfn_notify,user_data]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(memobj in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],CL.cl_objects[memobj]+" is not a valid OpenCL Mem Object","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 
  
    var _array = []

    if (memobj in CL.cl_objects_mem_callback) {
      _array = CL.cl_objects_mem_callback[memobj];
    }

    var _callback = null
    if (pfn_notify != 0) {
      _callback = function() { FUNCTION_TABLE[pfn_notify](memobj , user_data) };
    }

    _array.push(_callback);

    CL.cl_objects_mem_callback[memobj] = _array;

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif        
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clCreateSampler
   * @param {} context
   * @param {} normalized_coords
   * @param {} addressing_mode
   * @param {} filter_mode
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateSampler: function(context,normalized_coords,addressing_mode,filter_mode,cl_errcode_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateSampler",[context,normalized_coords,addressing_mode,filter_mode,cl_errcode_ret]);
#endif

    var _id = null;
    var _sampler = null;

    // Context must be created
#if CL_CHECK_VALID_OBJECT    
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"context '"+context+"' is not a valid context","");
#endif
      return 0; 
    }
#endif
    try {
      
      if ( addressing_mode == 0x1130 /* CL_ADDRESS_NONE */ ) addressing_mode = webcl.ADDRESS_CLAMP;

#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[context]+".createSampler",[normalized_coords,addressing_mode,filter_mode]);
#endif      

      _sampler = CL.cl_objects[context].createSampler(normalized_coords,addressing_mode,filter_mode);
      
    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    }

    _id = CL.udid(_sampler);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clRetainSampler
   * @param {} sampler
   * @return MemberExpression
   */
  clRetainSampler: function(sampler) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clRetainSampler",[sampler]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(sampler in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_SAMPLER],CL.cl_objects[sampler]+" is not a valid OpenCL sampler","");
#endif
      return webcl.INVALID_SAMPLER;
    }
#endif 

    if (!(sampler in CL.cl_objects_retains)) {
      CL.cl_objects_retains[sampler] = 1;
    } else {
      CL.cl_objects_retains[sampler] = CL.cl_objects_retains[sampler] + 1;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif        
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clReleaseSampler
   * @param {} sampler
   * @return MemberExpression
   */
  clReleaseSampler: function(sampler) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clReleaseSampler",[sampler]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(sampler in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_SAMPLER],CL.cl_objects[sampler]+" is not a valid OpenCL sampler","");
#endif
      return webcl.INVALID_SAMPLER;
    }
#endif

    // If is an object retain don't release it until retains > 0...
    if (sampler in CL.cl_objects_retains) {

      var _retain = CL.cl_objects_retains[sampler] - 1;

      CL.cl_objects_retains[sampler] = _retain;

      if (_retain >= 0) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif      
        return webcl.SUCCESS;
      }
    }

    try {

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(CL.cl_objects[sampler]+".release",[]);
#endif        
      CL.cl_objects[sampler].release();
      delete CL.cl_objects[sampler];

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetSamplerInfo
   * @param {} sampler
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetSamplerInfo: function(sampler,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetSamplerInfo",[sampler,param_name,param_value_size,param_value,param_value_size_ret]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(sampler in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_SAMPLER],CL.cl_objects[sampler]+" is not a valid OpenCL sampler","");
#endif
      return webcl.INVALID_SAMPLER;
    }
#endif
  
    var _info = null;

    try { 

#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[sampler]+".getInfo",[param_name]);
#endif      

      if (param_name == 0x1150 /*  CL_SAMPLER_REFERENCE_COUNT */) {
        _info = 0;

        if (sampler in CL.cl_objects) {
          _info++;
        }

        if (sampler in CL.cl_objects_retains) {
          _info+=CL.cl_objects_retains[sampler];
        }

      } else {
        _info = CL.cl_objects[sampler].getInfo(param_name);
      }
        
            
    } catch (e) {

      var _error = CL.catchError(e);

      if (param_value != 0) {
        {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      }

      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }
        
    if(typeof(_info) == "number") {

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } else if(typeof(_info) == "boolean") {

      if (param_value != 0) (_info == true) ? {{{ makeSetValue('param_value', '0', '1', 'i32') }}} : {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } else if(typeof(_info) == "object") {

      if (_info instanceof WebCLContext) {
 
        var _id = CL.udid(_info);
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_id', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

      } else if (_info == null) {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      } else {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
        return webcl.INVALID_VALUE;
      }
    } else {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
      return webcl.INVALID_VALUE;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clCreateProgramWithSource
   * @param {} context
   * @param {} count
   * @param {} strings
   * @param {} lengths
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateProgramWithSource: function(context,count,strings,lengths,cl_errcode_ret) {
    
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateProgramWithSource",[context,count,strings,lengths,cl_errcode_ret]);
#endif

    var _id = null;
    var _program = null;

    // Context must be created
#if CL_CHECK_VALID_OBJECT    
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"context '"+context+"' is not a valid context","");
#endif
      return 0; 
    }
#endif   

    try {
      
      var _string = "";

      for (var i = 0; i < count; i++) {
        if (lengths) {
          var _len = {{{ makeGetValue('lengths', 'i*4', 'i32') }}};
          if (_len < 0) {
            _string += Pointer_stringify({{{ makeGetValue('strings', 'i*4', 'i32') }}});   
          } else {
            _string += Pointer_stringify({{{ makeGetValue('strings', 'i*4', 'i32') }}}, _len);   
          }
        } else {
          _string += Pointer_stringify({{{ makeGetValue('strings', 'i*4', 'i32') }}}); 
        }
      }

      CL.parseKernel(_string);

#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[context]+".createProgramWithSource",[_string]);
#endif      

      _program = CL.cl_objects[context].createProgram(_string);
  
    } catch (e) {
      var _error = CL.catchError(e);

      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    }

    _id = CL.udid(_program);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clCreateProgramWithBinary
   * @param {} context
   * @param {} num_devices
   * @param {} device_list
   * @param {} lengths
   * @param {} binaries
   * @param {} cl_binary_status
   * @param {} cl_errcode_ret
   * @return Literal
   */
  clCreateProgramWithBinary: function(context,num_devices,device_list,lengths,binaries,cl_binary_status,cl_errcode_ret) {
    console.error("clCreateProgramWithBinary: Can't be implemented - Differences between WebCL and OpenCL 1.1\n");
    
    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
    }

    return 0;

  },

  /**
   * Description
   * @method clRetainProgram
   * @param {} program
   * @return MemberExpression
   */
  clRetainProgram: function(program) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clRetainProgram",[program]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(program in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_PROGRAM],CL.cl_objects[program]+" is not a valid OpenCL program","");
#endif
      return webcl.INVALID_PROGRAM;
    }
#endif 

    if (!(program in CL.cl_objects_retains)) {
      CL.cl_objects_retains[program] = 1;
    } else {
      CL.cl_objects_retains[program] = CL.cl_objects_retains[program] + 1;
    }
        
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif   
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clReleaseProgram
   * @param {} program
   * @return MemberExpression
   */
  clReleaseProgram: function(program) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clReleaseProgram",[program]);
#endif

#if CL_CHECK_VALID_OBJECT
    if (!(program in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_SAMPLER],CL.cl_objects[program]+" is not a valid OpenCL program","");
#endif
      return webcl.INVALID_PROGRAM;
    }
#endif

    // If is an object retain don't release it until retains > 0...
    if (program in CL.cl_objects_retains) {

      var _retain = CL.cl_objects_retains[program] - 1;

      CL.cl_objects_retains[program] = _retain;

      if (_retain >= 0) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif      
        return webcl.SUCCESS;
      }
    }

    try {

#if CL_GRAB_TRACE
        CL.webclCallStackTrace(CL.cl_objects[program]+".release",[]);
#endif        
        CL.cl_objects[program].release();
        delete CL.cl_objects[program]; 

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;

  },

  /**
   * Description
   * @method clBuildProgram
   * @param {} program
   * @param {} num_devices
   * @param {} device_list
   * @param {} options
   * @param {} pfn_notify
   * @param {} user_data
   * @return MemberExpression
   */
  clBuildProgram: function(program,num_devices,device_list,options,pfn_notify,user_data) {

#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clBuildProgram",[program,num_devices,device_list,options,pfn_notify,user_data]);
#endif
#if CL_CHECK_VALID_OBJECT
    // Program must be created
    if (!(program in CL.cl_objects)) {

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_PROGRAM],"program '"+program+"' is not a valid program","");
#endif

      return webcl.INVALID_PROGRAM; 
    }
#endif
    try {

      var _devices = [];
      var _option = (options == 0) ? "" : Pointer_stringify(options); 

      if (device_list != 0 && num_devices > 0 ) {
        for (var i = 0; i < num_devices ; i++) {
          var _device = {{{ makeGetValue('device_list', 'i*4', 'i32') }}}
#if CL_CHECK_VALID_OBJECT          
          if (_device in CL.cl_objects) {
#endif            
            _devices.push(CL.cl_objects[_device]);
#if CL_CHECK_VALID_OBJECT
          }
#endif          
        }
      }

      // If device_list is NULL value, the program executable is built for all devices associated with program.
      if (_devices.length == 0) {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(CL.cl_objects[program]+".getInfo",[webcl.PROGRAM_DEVICES]);
#endif          
        _devices = CL.cl_objects[program].getInfo(webcl.PROGRAM_DEVICES); 
      }

      var _callback = null
      if (pfn_notify != 0) {
        /**
         * Description
         * @return 
         */
        _callback = function() { 
          console.info("\nCall ( clBuildProgram ) callback function : FUNCTION_TABLE["+pfn_notify+"]("+program+", "+user_data+")");
          FUNCTION_TABLE[pfn_notify](program, user_data) 
        };
      }

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(CL.cl_objects[program]+".build",[_devices,_option,_callback]);
#endif        
      
      CL.cl_objects[program].build(_devices,_option,_callback);

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;      

  },

  /**
   * Description
   * @method clUnloadCompiler
   * @return MemberExpression
   */
  clUnloadCompiler: function() {
    console.error("clUnloadCompiler: Can't be implemented - Differences between WebCL and OpenCL 1.1\n");
    
    return webcl.INVALID_VALUE;;
  },

  /**
   * Description
   * @method clGetProgramInfo
   * @param {} program
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetProgramInfo: function(program,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetProgramInfo",[program,param_name,param_value_size,param_value,param_value_size_ret]);
#endif
#if CL_CHECK_VALID_OBJECT
    // Program must be created
    if (!(program in CL.cl_objects)) {

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_PROGRAM],"program '"+program+"' is not a valid program","");
#endif

      return webcl.INVALID_PROGRAM; 
    }
#endif

    var _info = null;

    try { 
#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[program]+".getInfo",[param_name]);
#endif        
      if (param_name == 0x1160 /* CL_PROGRAM_REFERENCE_COUNT */) {
        _info = 0;

        if (program in CL.cl_objects) {
          _info++;
        }

        if (program in CL.cl_objects_retains) {
          _info+=CL.cl_objects_retains[program];
        }

      } else {
        _info = CL.cl_objects[program].getInfo(param_name);
      }
      
    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {
        {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      }

      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

    if(typeof(_info) == "number") {

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } else if(typeof(_info) == "string") {
      if (param_value != 0) {
        writeStringToMemory(_info, param_value);
      }

      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '_info.length + 1', 'i32') }}};
      }
    } else if(typeof(_info) == "object") {

      if (_info instanceof WebCLContext) {

        var _id = CL.udid(_info);
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_id', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

      } else if (_info instanceof Array) {

        for (var i = 0; i < Math.min(param_value_size>>2,_info.length); i++) {
          var _id = CL.udid(_info[i]);
          if (param_value != 0) {{{ makeSetValue('param_value', 'i*4', '_id', 'i32') }}};
        }
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '_info.length * 4', 'i32') }}};

      } else if (_info == null) {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      } else {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
        return webcl.INVALID_VALUE;
      }
    } else {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
      return webcl.INVALID_VALUE;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetProgramBuildInfo
   * @param {} program
   * @param {} device
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetProgramBuildInfo: function(program,device,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetProgramBuildInfo",[program,device,param_name,param_value_size,param_value,param_value_size_ret]);
#endif
#if CL_CHECK_VALID_OBJECT
    // Program must be created
    if (!(program in CL.cl_objects)) {

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_PROGRAM],"program '"+program+"' is not a valid program","");
#endif

      return webcl.INVALID_PROGRAM; 
    }
    if (!(device in CL.cl_objects)) {

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_PROGRAM],"device '"+device+"' is not a valid device","");
#endif

      return webcl.INVALID_DEVICE; 
    }

#endif

    var _info = null;

    try { 

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+CL.cl_objects[program]+".getBuildInfo",[device,param_name]);
#endif        

      _info = CL.cl_objects[program].getBuildInfo(CL.cl_objects[device], param_name);

    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {
        {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      }

      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

    if(typeof(_info) == "number") {

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } else if(typeof(_info) == "string") {
      if (param_value != 0) {
        writeStringToMemory(_info, param_value);
      }
    
      if (param_value_size_ret != 0) {
        {{{ makeSetValue('param_value_size_ret', '0', '_info.length + 1', 'i32') }}};
      }
    } else {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_VALUE],typeof(_info)+" not yet implemented","");
#endif
      return webcl.INVALID_VALUE;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clCreateKernel
   * @param {} program
   * @param {} kernel_name
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateKernel: function(program,kernel_name,cl_errcode_ret) {
    
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateKernel",[program,kernel_name,cl_errcode_ret]);
#endif

    var _id = null;
    var _kernel = null;
    var _name = (kernel_name == 0) ? "" : Pointer_stringify(kernel_name);

    // program must be created
#if CL_CHECK_VALID_OBJECT    
    if (!(program in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_PROGRAM', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"program '"+program+"' is not a valid program","");
#endif
      return 0; 
    }
#endif
    try {
    
#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[program]+".createKernel",[_name]);
#endif      

      _kernel = CL.cl_objects[program].createKernel(_name);
      
      Object.defineProperty(_kernel, "name", { value : _name,writable : false });
      Object.defineProperty(_kernel, "sig", { value : CL.cl_kernels_sig[_name],writable : false });

#if CL_VALIDATOR
      Object.defineProperty(_kernel, "val_param", { value : CL.cl_validator[_name],writable : false });
      Object.defineProperty(_kernel, "val_param_argsize", { value : CL.cl_validator_argsize[_name],writable : false });
#endif

#if CL_DEBUG
      console.info("clCreateKernel : Kernel '"+_kernel.name+"', has "+_kernel.sig+" parameters !!!!");
#if CL_VALIDATOR
      console.info("\tValidator info");
      console.info("\t\t" + _kernel.val_param);
      console.info("\t\t" + _kernel.val_param_argsize);        
#endif
#endif      
      
    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
      }

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    }

    _id = CL.udid(_kernel);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clCreateKernelsInProgram
   * @param {} program
   * @param {} num_kernels
   * @param {} kernels
   * @param {} num_kernels_ret
   * @return MemberExpression
   */
  clCreateKernelsInProgram: function(program,num_kernels,kernels,num_kernels_ret) {
    
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateKernelsInProgram",[program,num_kernels,kernels,num_kernels_ret]);
#endif

    // program must be created
#if CL_CHECK_VALID_OBJECT
    if (!(program in CL.cl_objects)) {

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_PROGRAM],"program '"+program+"' is not a valid program","");
#endif
      return webcl.INVALID_PROGRAM; 
    }
#endif
    try {
    
#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[program]+".createKernelsInProgram",[]);
#endif      

      var _kernels = CL.cl_objects[program].createKernelsInProgram();

      for (var i = 0; i < Math.min(num_kernels,_kernels.length); i++) {
        var _id = CL.udid(_kernels[i]);
        if (kernels != 0) {{{ makeSetValue('kernels', 'i*4', '_id', 'i32') }}};
        
        var _name = _kernels[i].getInfo(webcl.KERNEL_FUNCTION_NAME);

        Object.defineProperty(_kernels[i], "name", { value : _name,writable : false });
        Object.defineProperty(_kernels[i], "sig", { value : CL.cl_kernels_sig[_name],writable : false });

#if CL_VALIDATOR
        Object.defineProperty(_kernels[i], "val_param", { value : CL.cl_validator[_name],writable : false });
        Object.defineProperty(_kernels[i], "val_param_argsize", { value : CL.cl_validator_argsize[_name],writable : false });
#endif

#if CL_DEBUG
        console.info("clCreateKernelsInProgram : Kernel '"+_kernels[i].name+"', has "+_kernels[i].sig+" parameters !!!!");
#if CL_VALIDATOR
        console.info("\tValidator info");
        console.info("\t\t" + _kernels[i].val_param);
        console.info("\t\t" + _kernels[i].val_param_argsize);        
#endif
#endif  

      }
           
      if (num_kernels_ret != 0) {{{ makeSetValue('num_kernels_ret', '0', '_kernels.length', 'i32') }}};

    } catch (e) {

      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif
      return _error; 
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clRetainKernel
   * @param {} kernel
   * @return MemberExpression
   */
  clRetainKernel: function(kernel) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clRetainKernel",[kernel]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(kernel in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_KERNEL],"WebCLKernel '"+kernel+"' are not inside the map","");
#endif
      return webcl.INVALID_KERNEL;
    }
#endif 

    if (!(kernel in CL.cl_objects_retains)) {
      CL.cl_objects_retains[kernel] = 1;
    } else {
      CL.cl_objects_retains[kernel] = CL.cl_objects_retains[kernel] + 1;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif   
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clReleaseKernel
   * @param {} kernel
   * @return MemberExpression
   */
  clReleaseKernel: function(kernel) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clReleaseKernel",[kernel]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(kernel in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_KERNEL],"WebCLKernel '"+kernel+"' are not inside the map","");
#endif
      return webcl.INVALID_KERNEL;
    }
#endif

    // If is an object retain don't release it until retains > 0...
    if (kernel in CL.cl_objects_retains) {

      var _retain = CL.cl_objects_retains[kernel] - 1;

      CL.cl_objects_retains[kernel] = _retain;

      if (_retain >= 0) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif      
        return webcl.SUCCESS;
      }
    }

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(CL.cl_objects[kernel]+".release",[]);
#endif    

    try {

      CL.cl_objects[kernel].release();
        
    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

    delete CL.cl_objects[kernel];

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;
  },  

  /**
   * Description
   * @method clSetKernelArg
   * @param {} kernel
   * @param {} arg_index
   * @param {} arg_size
   * @param {} arg_value
   * @return MemberExpression
   */
  clSetKernelArg: function(kernel,arg_index,arg_size,arg_value) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clSetKernelArg",[kernel,arg_index,arg_size,arg_value]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(kernel in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_KERNEL],CL.cl_objects[kernel]+" is not a valid OpenCL kernel","");
#endif
      return webcl.INVALID_KERNEL;
    }
#endif
    if (CL.cl_objects[kernel].sig.length < arg_index) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_KERNEL],CL.cl_objects[kernel]+" doesn't contains sig array","");
#endif
      return webcl.INVALID_KERNEL;          
    }

    var _kernel = CL.cl_objects[kernel];

#if CL_VALIDATOR
    var _posarg = _kernel.val_param[arg_index];
#else
    var _posarg = arg_index;
#endif

    var _sig = _kernel.sig[_posarg];
    
    try {

      // LOCAL ARG
      if (_sig == webcl.LOCAL) {

        var _array = new Uint32Array([arg_size]);

#if CL_GRAB_TRACE
        CL.webclCallStackTrace(_kernel+".setArg<<__local>>",[_posarg,_array]);
#endif     
        _kernel.setArg(_posarg,_array);

#if CL_VALIDATOR 
        var _sizearg = CL.cast_long(arg_size);

        if (_kernel.val_param_argsize.indexOf(_posarg+1) >= 0) {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(_kernel+".setArg<<VALIDATOR>>",[_posarg+1,_sizearg]);
#endif        
          _kernel.setArg(_posarg+1,_sizearg);
        }
#endif

      } else {

        var _value = {{{ makeGetValue('arg_value', '0', 'i32') }}};

        // WEBCL OBJECT ARG
        if (_value in CL.cl_objects) {

#if CL_GRAB_TRACE
          CL.webclCallStackTrace(_kernel+".setArg",[_posarg,CL.cl_objects[_value]]);
#endif        
          _kernel.setArg(_posarg,CL.cl_objects[_value]);
          
          if (! (CL.cl_objects[_value] instanceof WebCLSampler)) {

#if CL_VALIDATOR 
          
#if CL_GRAB_TRACE
            CL.webclCallStackTrace(CL.cl_objects[_value]+".getInfo",[webcl.MEM_SIZE]);
#endif     
            var _size = CL.cl_objects[_value].getInfo(webcl.MEM_SIZE);
            var _sizearg = CL.cast_long(_size);

            if (_kernel.val_param_argsize.indexOf(_posarg+1) >= 0) {
#if CL_GRAB_TRACE
              CL.webclCallStackTrace(_kernel+".setArg<<VALIDATOR>>",[_posarg+1,_sizearg]);
#endif        
              _kernel.setArg(_posarg+1,_sizearg);
            }
#endif      
          }
          
        } else {

          var _array = CL.getReferencePointerToArray(arg_value,arg_size,[[_sig,1]]);
         
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(_kernel+".setArg",[_posarg,_array]);
#endif        
          _kernel.setArg(_posarg,_array);

#if CL_VALIDATOR
          var _sizearg = CL.cast_long(arg_size);

          if (_kernel.val_param_argsize.indexOf(_posarg+1) >= 0) {
#if CL_GRAB_TRACE
            CL.webclCallStackTrace(_kernel+".setArg<<VALIDATOR>>",[_posarg+1,_sizearg]);
#endif        
            _kernel.setArg(_posarg+1,_sizearg);
          }
#endif
        }
      }
    } catch (e) {
      var name = _kernel.getInfo(webcl.KERNEL_FUNCTION_NAME);
      var num = _kernel.getInfo(webcl.KERNEL_NUM_ARGS);
      console.info("AL "+ name +" -> "+ num + " parameters : ");
      for (var i = 0; i < num; i++) {
        
        try {
          var webCLKernelArgInfo = _kernel.getArgInfo(i);
          console.info("\t" +i+" -> "+webCLKernelArgInfo.name +" : "+webCLKernelArgInfo.typeName+" : "+webCLKernelArgInfo.addressQualifier+ " : " + webCLKernelArgInfo.accessQualifier );
        } catch(e) {
          console.error("ARRGGGGGGGGG");
        }
        
      }

      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetKernelInfo
   * @param {} kernel
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetKernelInfo: function(kernel,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetKernelInfo",[kernel,param_name,param_value_size,param_value,param_value_size_ret]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(kernel in CL.cl_objects)) {
      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_KERNEL],"WebCLKernel '"+kernel+"' are not inside the map","");
#endif
      return webcl.INVALID_KERNEL;
    }
#endif 
#if CL_GRAB_TRACE
    CL.webclCallStackTrace(""+CL.cl_objects[kernel]+".getInfo",[param_name]);
#endif   

    try { 
  
      var _info = null;

      if (param_name == 0x1192 /* CL_KERNEL_REFERENCE_COUNT */) {
        _info = 0;

        if (kernel in CL.cl_objects) {
          _info++;
        }

        if (kernel in CL.cl_objects_retains) {
          _info+=CL.cl_objects_retains[kernel];
        }

      } else {
        _info = CL.cl_objects[kernel].getInfo(param_name);
      }

      if(typeof(_info) == "number") {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

      } else if(typeof(_info) == "string") {

        if (param_value != 0) writeStringToMemory(_info, param_value);
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '_info.length + 1', 'i32') }}};
    
      } else if(typeof(_info) == "object") {

        if ( (_info instanceof WebCLContext) || (_info instanceof WebCLProgram) ){
   
          var _id = CL.udid(_info);
          if (param_value != 0) {{{ makeSetValue('param_value', '0', '_id', 'i32') }}};
          if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

        } else {

          console.error("clGetKernelInfo: unknow type of info '"+_info+"'")

          if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
          if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

        }
      } else {

        console.error("clGetKernelInfo: unknow type of info '"+_info+"'")

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      }
    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetKernelWorkGroupInfo
   * @param {} kernel
   * @param {} device
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetKernelWorkGroupInfo: function(kernel,device,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetKernelWorkGroupInfo",[kernel,device,param_name,param_value_size,param_value,param_value_size_ret]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(kernel in CL.cl_objects)) {
      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_KERNEL],"WebCLKernel '"+kernel+"' are not inside the map","");
#endif
      return webcl.INVALID_KERNEL;
    }
#endif 
#if CL_CHECK_VALID_OBJECT
    if (!(device in CL.cl_objects)) {
      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_DEVICE],"WebCLDevice '"+device+"' are not inside the map","");
#endif
      return webcl.INVALID_DEVICE;
    }
#endif 

    try {

      var _info = CL.cl_objects[kernel].getWorkGroupInfo(CL.cl_objects[device], param_name);

      if(typeof(_info) == "number") {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

      } else if (_info instanceof Int32Array) {
       
        for (var i = 0; i < Math.min(param_value_size>>2,_info.length); i++) {
          if (param_value != 0) {{{ makeSetValue('param_value', 'i*4', '_info[i]', 'i32') }}};
        }
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '_info.length * 4', 'i32') }}};
      
      } else {

        console.error("clGetKernelWorkGroupInfo: unknow type of info '"+_info+"'")
        
        if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      }

    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
      
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clWaitForEvents
   * @param {} num_events
   * @param {} event_list
   * @return MemberExpression
   */
  clWaitForEvents: function(num_events,event_list) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clWaitForEvents",[num_events,event_list]);
#endif

    var _events = [];

    for (var i = 0; i < num_events; i++) {
      var _event = {{{ makeGetValue('event_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT  
      if (!(_event in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event+"' are not inside the map","");
#endif    
        return webcl.INVALID_EVENT; 
      }
#endif
      
      _events.push(CL.cl_objects[_event]) 
    }

    try {

#if CL_GRAB_TRACE
      CL.webclCallStackTrace(""+webcl+".waitForEvents",[_events]);
#endif      
      webcl.waitForEvents(_events);

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetEventInfo
   * @param {} event
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetEventInfo: function(event,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetEventInfo",[event,param_name,param_value_size,param_value,param_value_size_ret]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(event in CL.cl_objects)) {
      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+event+"' are not inside the map","");
#endif
      return webcl.INVALID_EVENT;
    }
#endif 

    try { 

      var _info = CL.cl_objects[event].getInfo(param_name);

      if(typeof(_info) == "number") {

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

      } else if(typeof(_info) == "object") {

        if ( (_info instanceof WebCLContext) || (_info instanceof WebCLCommandQueue) ){
   
          var _id = CL.udid(_info);
          if (param_value != 0) {{{ makeSetValue('param_value', '0', '_id', 'i32') }}};
          if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

        } else {

          console.error("clGetEventInfo: unknow type of info '"+_info+"'")

          if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
          if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

        }
      } else {

        console.error("clGetEventInfo: unknow type of info '"+_info+"'")

        if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
        if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      }
    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clCreateUserEvent
   * @param {} context
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateUserEvent: function(context,cl_errcode_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateUserEvent",[context,cl_errcode_ret]);
#endif
#if CL_CHECK_VALID_OBJECT      
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"WebCLContext '"+context+"' are not inside the map","");
#endif
      return 0; 
    }
#endif

    var _id = null;
    var _event = null;
    
#if CL_GRAB_TRACE
    CL.webclCallStackTrace( CL.cl_objects[context]+".createUserEvent",[]);
#endif 

    try {
     
      _event = CL.cl_objects[context].createUserEvent();
      
    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};

    _id = CL.udid(_event);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;

  },

  /**
   * Description
   * @method clRetainEvent
   * @param {} event
   * @return MemberExpression
   */
  clRetainEvent: function(event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clRetainEvent",[event]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(event in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+event+"' are not inside the map","");
#endif
      return webcl.INVALID_EVENT;
    }
#endif 

    if (!(event in CL.cl_objects_retains)) {
      CL.cl_objects_retains[event] = 1;
    } else {
      CL.cl_objects_retains[event] = CL.cl_objects_retains[event] + 1;
    }
       
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif           
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clReleaseEvent
   * @param {} event
   * @return MemberExpression
   */
  clReleaseEvent: function(event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clReleaseEvent",[event]);
#endif
#if CL_CHECK_VALID_OBJECT
    if (!(event in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+event+"' are not inside the map","");
#endif
      return webcl.INVALID_EVENT;
    }
#endif

    // If is an object retain don't release it until retains > 0...
    if (event in CL.cl_objects_retains) {

      var _retain = CL.cl_objects_retains[event] - 1;

      CL.cl_objects_retains[event] = _retain;

      if (_retain >= 0) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif      
        return webcl.SUCCESS;
      }
    }

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(CL.cl_objects[event]+".release",[]);
#endif    

    try {

      CL.cl_objects[event].release();
        
    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

    delete CL.cl_objects[event];

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clSetUserEventStatus
   * @param {} event
   * @param {} execution_status
   * @return MemberExpression
   */
  clSetUserEventStatus: function(event,execution_status) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clSetUserEventStatus",[event,execution_status]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(event in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+event+"' are not inside the map","");
#endif 

      return webcl.INVALID_EVENT;
    }
#endif

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(CL.cl_objects[event]+".setUserEventStatus",[execution_status]);
#endif        

    try {

        CL.cl_objects[event].setUserEventStatus(execution_status);

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clSetEventCallback
   * @param {} event
   * @param {} command_exec_callback_type
   * @param {} pfn_notify
   * @param {} user_data
   * @return MemberExpression
   */
  clSetEventCallback: function(event,command_exec_callback_type,pfn_notify,user_data) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clSetEventCallback",[event,command_exec_callback_type,pfn_notify,user_data]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(event in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+event+"' are not inside the map","");
#endif 

      return webcl.INVALID_EVENT;
    }
#endif

    var _callback = null
    if (pfn_notify != 0) {
      _callback = function() { 
        console.info("\nCall ( clSetEventCallback ) callback function : FUNCTION_TABLE["+pfn_notify+"]("+event+", "+command_exec_callback_type+" , "+user_data+")");
        FUNCTION_TABLE[pfn_notify](event, command_exec_callback_type , user_data); 
      };
    }

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(CL.cl_objects[event]+".setCallback",[command_exec_callback_type,_callback]);
#endif     

    try {

      CL.cl_objects[event].setCallback(command_exec_callback_type,_callback);

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetEventProfilingInfo
   * @param {} event
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetEventProfilingInfo: function(event,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetEventProfilingInfo",[event,param_name,param_value_size,param_value,param_value_size_ret]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(event in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+event+"' are not inside the map","");
#endif 

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

      return webcl.INVALID_EVENT;
    }
#endif 

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(""+CL.cl_objects[event]+".getProfilingInfo",[param_name]);
#endif        

    try { 

      var _info = CL.cl_objects[event].getProfilingInfo(param_name);

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clFlush
   * @param {} command_queue
   * @return MemberExpression
   */
  clFlush: function(command_queue) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clFlush",[command_queue]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".flush",[]);
#endif        

    try {
        
      CL.cl_objects[command_queue].flush();

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clFinish
   * @param {} command_queue
   * @return MemberExpression
   */
  clFinish: function(command_queue) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clFinish",[command_queue]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".finish",[]);
#endif        

    try {

      CL.cl_objects[command_queue].finish();

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clEnqueueReadBuffer
   * @param {} command_queue
   * @param {} buffer
   * @param {} blocking_read
   * @param {} offset
   * @param {} cb
   * @param {} ptr
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueReadBuffer: function(command_queue,buffer,blocking_read,offset,cb,ptr,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueReadBuffer",[command_queue,buffer,blocking_read,offset,cb,ptr,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_SET_POINTER    
    if (CL.cl_pn_type.length == 0) console.info("/!\\ clEnqueueReadBuffer : you don't call clSetTypePointer for ptr parameter");
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 
 
    var _event_wait_list = [];
    var _host_ptr = CL.getReferencePointerToArray(ptr,cb,CL.cl_pn_type);
  
    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_CHECK_SET_POINTER    
        CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif

      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 

    try {

      if (event != 0) {
        var _event = new WebCLEvent();
        CL.cl_objects[command_queue].enqueueReadBuffer(CL.cl_objects[buffer],blocking_read,offset,cb,_host_ptr,_event_wait_list,_event);
#if CL_GRAB_TRACE
        // It's the only callStackTrace call after the call for have info about the read host ptr
        CL.webclCallStackTrace("(*)"+CL.cl_objects[command_queue]+".enqueueReadBuffer",[CL.cl_objects[buffer],blocking_read,offset,cb,_host_ptr,_event_wait_list,_event]);
#endif           
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
        CL.cl_objects[command_queue].enqueueReadBuffer(CL.cl_objects[buffer],blocking_read,offset,cb,_host_ptr,_event_wait_list);
#if CL_GRAB_TRACE
        // It's the only callStackTrace call after the call for have info about the read host ptr
        CL.webclCallStackTrace("(*)"+CL.cl_objects[command_queue]+".enqueueReadBuffer",[CL.cl_objects[buffer],blocking_read,offset,cb,_host_ptr,_event_wait_list]);
#endif    
      } 
    } catch (e) {
      var _error = CL.catchError(e);
        
#if CL_CHECK_SET_POINTER    
      CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_CHECK_SET_POINTER    
    CL.cl_pn_type = [];
#endif        
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS;    
  },

  /**
   * Description
   * @method clEnqueueReadBufferRect
   * @param {} command_queue
   * @param {} buffer
   * @param {} blocking_read
   * @param {} buffer_origin
   * @param {} host_origin
   * @param {} region
   * @param {} buffer_row_pitch
   * @param {} buffer_slice_pitch
   * @param {} host_row_pitch
   * @param {} host_slice_pitch
   * @param {} ptr
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueReadBufferRect: function(command_queue,buffer,blocking_read,buffer_origin,host_origin,region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,ptr,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueReadBufferRect",[command_queue,buffer,blocking_read,buffer_origin,host_origin,region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,ptr,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_SET_POINTER    
    if (CL.cl_pn_type.length == 0) console.info("/!\\ clEnqueueReadBufferRect : you don't call clSetTypePointer for ptr parameter");
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    var _event_wait_list = [];
    
    var _buffer_origin = [];
    var _host_origin = [];
    var _region = [];

    for (var i = 0; i < 3; i++) {
      _buffer_origin.push({{{ makeGetValue('buffer_origin', 'i*4', 'i32') }}});
      _host_origin.push({{{ makeGetValue('host_origin', 'i*4', 'i32') }}});
      _region.push({{{ makeGetValue('region', 'i*4', 'i32') }}});            
    }

    // console.info("/!\\ clEnqueueReadBufferRect : Check the size of the ptr '"+_region.reduce(function (a, b) { return a * b; })+"'... need to be more tested");
    var _host_ptr = CL.getReferencePointerToArray(ptr,_region.reduce(function (a, b) { return a * b; }),CL.cl_pn_type);

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_CHECK_SET_POINTER    
        CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif

      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueReadBufferRect",[CL.cl_objects[buffer],blocking_read,_buffer_origin,_host_origin,_region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,_host_ptr,_event_wait_list,_event]);
#endif     
   
    try {

      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueReadBufferRect",[CL.cl_objects[buffer],blocking_read,_buffer_origin,_host_origin,_region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,_host_ptr,_event_wait_list,_event]);
#endif          
        CL.cl_objects[command_queue].enqueueReadBufferRect(CL.cl_objects[buffer],blocking_read,_buffer_origin,_host_origin,_region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,_host_ptr,_event_wait_list,_event);
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueReadBufferRect",[CL.cl_objects[buffer],blocking_read,_buffer_origin,_host_origin,_region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,_host_ptr,_event_wait_list]);
#endif          
        CL.cl_objects[command_queue].enqueueReadBufferRect(CL.cl_objects[buffer],blocking_read,_buffer_origin,_host_origin,_region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,_host_ptr,_event_wait_list);
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_CHECK_SET_POINTER    
      CL.cl_pn_type = [];
#endif   
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_CHECK_SET_POINTER    
    CL.cl_pn_type = [];
#endif   
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS;    
  },

  /**
   * Description
   * @method clEnqueueWriteBuffer
   * @param {} command_queue
   * @param {} buffer
   * @param {} blocking_write
   * @param {} offset
   * @param {} cb
   * @param {} ptr
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueWriteBuffer: function(command_queue,buffer,blocking_write,offset,cb,ptr,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueWriteBuffer",[command_queue,buffer,blocking_write,offset,cb,ptr,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_SET_POINTER    
    if (CL.cl_pn_type.length == 0) console.info("/!\\ clEnqueueWriteBuffer : you don't call clSetTypePointer for ptr parameter");
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    var _event_wait_list = [];
    var _host_ptr = CL.getReferencePointerToArray(ptr,cb,CL.cl_pn_type);

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_CHECK_SET_POINTER    
        CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif

      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 

    try {
          
      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueWriteBuffer",[CL.cl_objects[buffer],blocking_write,offset,cb,_host_ptr,_event_wait_list,_event]);
#endif          
        CL.cl_objects[command_queue].enqueueWriteBuffer(CL.cl_objects[buffer],blocking_write,offset,cb,_host_ptr,_event_wait_list,_event);    
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueWriteBuffer",[CL.cl_objects[buffer],blocking_write,offset,cb,_host_ptr,_event_wait_list]);
#endif     
        CL.cl_objects[command_queue].enqueueWriteBuffer(CL.cl_objects[buffer],blocking_write,offset,cb,_host_ptr,_event_wait_list);    
      }

    } catch (e) {
      var _error = CL.catchError(e);
 
#if CL_CHECK_SET_POINTER    
      CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_CHECK_SET_POINTER    
    CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;  
  },

  /**
   * Description
   * @method clEnqueueWriteBufferRect
   * @param {} command_queue
   * @param {} buffer
   * @param {} blocking_write
   * @param {} buffer_origin
   * @param {} host_origin
   * @param {} region
   * @param {} buffer_row_pitch
   * @param {} buffer_slice_pitch
   * @param {} host_row_pitch
   * @param {} host_slice_pitch
   * @param {} ptr
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueWriteBufferRect: function(command_queue,buffer,blocking_write,buffer_origin,host_origin,region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,ptr,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueWriteBufferRect",[command_queue,buffer,blocking_write,buffer_origin,host_origin,region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,ptr,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_SET_POINTER    
    if (CL.cl_pn_type.length == 0) console.info("/!\\ clEnqueueWriteBufferRect : you don't call clSetTypePointer for ptr parameter");
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    var _event_wait_list = [];
    
    var _host_ptr = CL.getReferencePointerToArray(ptr,cb,CL.cl_pn_type);

    var _buffer_origin = [];
    var _host_origin = [];
    var _region = [];

    for (var i = 0; i < 3; i++) {
      _buffer_origin.push({{{ makeGetValue('buffer_origin', 'i*4', 'i32') }}});
      _host_origin.push({{{ makeGetValue('host_origin', 'i*4', 'i32') }}});
      _region.push({{{ makeGetValue('region', 'i*4', 'i32') }}});            
    }

    // console.info("/!\\ clEnqueueWriteBufferRect : Check the size of the ptr '"+_region.reduce(function (a, b) { return a * b; })+"'... need to be more tested");
    var _host_ptr = CL.getReferencePointerToArray(ptr,_region.reduce(function (a, b) { return a * b; }),CL.cl_pn_type);

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_CHECK_SET_POINTER    
        CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif

      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 

    try {

      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueWriteBufferRect",[CL.cl_objects[buffer],blocking_write,_buffer_origin,_host_origin,_region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,_host_ptr,_event_wait_list,_event]);   
#endif            
        CL.cl_objects[command_queue].enqueueWriteBufferRect(CL.cl_objects[buffer],blocking_write,_buffer_origin,_host_origin,_region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,_host_ptr,_event_wait_list,_event);   
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};  
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueWriteBufferRect",[CL.cl_objects[buffer],blocking_write,_buffer_origin,_host_origin,_region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,_host_ptr,_event_wait_list]);   
#endif           
        CL.cl_objects[command_queue].enqueueWriteBufferRect(CL.cl_objects[buffer],blocking_write,_buffer_origin,_host_origin,_region,buffer_row_pitch,buffer_slice_pitch,host_row_pitch,host_slice_pitch,_host_ptr,_event_wait_list);  
      }
       
    } catch (e) {
      var _error = CL.catchError(e);

#if CL_CHECK_SET_POINTER    
      CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_CHECK_SET_POINTER    
    CL.cl_pn_type = [];
#endif  
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;  
  },

  /**
   * Description
   * @method clEnqueueCopyBuffer
   * @param {} command_queue
   * @param {} src_buffer
   * @param {} dst_buffer
   * @param {} src_offset
   * @param {} dst_offset
   * @param {} cb
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueCopyBuffer: function(command_queue,src_buffer,dst_buffer,src_offset,dst_offset,cb,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueCopyBuffer",[command_queue,src_buffer,dst_buffer,src_offset,dst_offset,cb,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(src_buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+src_buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(dst_buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+dst_buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    var _event_wait_list = [];

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif

      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 

    try {
  
      if (event != 0) {
        var _event = new WebCLEvent(); 
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyBuffer",[CL.cl_objects[src_buffer],CL.cl_objects[dst_buffer],src_offset,dst_offset,cb,_event_wait_list,_event]);
#endif          
        CL.cl_objects[command_queue].enqueueCopyBuffer(CL.cl_objects[src_buffer],CL.cl_objects[dst_buffer],src_offset,dst_offset,cb,_event_wait_list,_event);    
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyBuffer",[CL.cl_objects[src_buffer],CL.cl_objects[dst_buffer],src_offset,dst_offset,cb,_event_wait_list]);
#endif               
        CL.cl_objects[command_queue].enqueueCopyBuffer(CL.cl_objects[src_buffer],CL.cl_objects[dst_buffer],src_offset,dst_offset,cb,_event_wait_list);    
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS;  
  },

  /**
   * Description
   * @method clEnqueueCopyBufferRect
   * @param {} command_queue
   * @param {} src_buffer
   * @param {} dst_buffer
   * @param {} src_origin
   * @param {} dst_origin
   * @param {} region
   * @param {} src_row_pitch
   * @param {} src_slice_pitch
   * @param {} dst_row_pitch
   * @param {} dst_slice_pitch   
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueCopyBufferRect: function(command_queue,src_buffer,dst_buffer,src_origin,dst_origin,region,src_row_pitch,src_slice_pitch,dst_row_pitch,dst_slice_pitch,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueCopyBufferRect",[command_queue,src_buffer,dst_buffer,src_origin,dst_origin,region,src_row_pitch,src_slice_pitch,dst_row_pitch,dst_slice_pitch,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(src_buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+src_buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(dst_buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+dst_buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    var _event_wait_list = [];
    var _src_origin = [];
    var _dest_origin = [];
    var _region = [];

    for (var i = 0; i < 3; i++) {
      _src_origin.push( {{{ makeGetValue('src_origin', 'i*4', 'i32') }}});
      _dest_origin.push( {{{ makeGetValue('dst_origin', 'i*4', 'i32') }}});
      _region.push( {{{ makeGetValue('region', 'i*4', 'i32') }}});   
    }          

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif

      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 

    try {
  
      if (event != 0) {
        var _event = new WebCLEvent(); 
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyBufferRect",[CL.cl_objects[src_buffer],CL.cl_objects[dst_buffer],_src_origin,_dest_origin,_region,src_row_pitch,src_slice_pitch,dst_row_pitch,dst_slice_pitch,_event_wait_list,_event]);
#endif          
        CL.cl_objects[command_queue].enqueueCopyBufferRect(CL.cl_objects[src_buffer],CL.cl_objects[dst_buffer],_src_origin,_dest_origin,_region,src_row_pitch,src_slice_pitch,dst_row_pitch,dst_slice_pitch,_event_wait_list,_event);
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyBufferRect",[CL.cl_objects[src_buffer],CL.cl_objects[dst_buffer],_src_origin,_dest_origin,_region,src_row_pitch,src_slice_pitch,dst_row_pitch,dst_slice_pitch,_event_wait_list]);
#endif               
        CL.cl_objects[command_queue].enqueueCopyBufferRect(CL.cl_objects[src_buffer],CL.cl_objects[dst_buffer],_src_origin,_dest_origin,_region,src_row_pitch,src_slice_pitch,dst_row_pitch,dst_slice_pitch,_event_wait_list);    
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS;  
  },

  /**
   * Description
   * @method clEnqueueReadImage
   * @param {} command_queue
   * @param {} image
   * @param {} blocking_read
   * @param {} origin
   * @param {} region
   * @param {} row_pitch
   * @param {} slice_pitch
   * @param {} ptr
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueReadImage: function(command_queue,image,blocking_read,origin,region,row_pitch,slice_pitch,ptr,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueReadImage",[command_queue,image,blocking_read,origin,region,row_pitch,slice_pitch,ptr,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(image in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+image+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    var _event_wait_list = [];

    var _origin = [];
    var _region = [];
    var _size = CL.getImageSizeType(image);
    var _channel = CL.getImageFormatType(image);

    for (var i = 0; i < 2; i++) {
      _origin.push( {{{ makeGetValue('origin', 'i*4', 'i32') }}});
      _region.push( {{{ makeGetValue('region', 'i*4', 'i32') }}});  
      _size *= _region[i];     
    }          

    // console.info("/!\\ clEnqueueReadImage : Check the size of the ptr '"+_size+"'... need to be more tested");
    var _host_ptr = CL.getReferencePointerToArray(ptr,_size,[_channel,1]);

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif
        
      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 

    try {      

      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueReadImage",[CL.cl_objects[image],blocking_read,_origin,_region,row_pitch,_host_ptr,_event_wait_list,_event]);
#endif          
        CL.cl_objects[command_queue].enqueueReadImage(CL.cl_objects[image],blocking_read,_origin,_region,row_pitch,_host_ptr,_event_wait_list, _event);
        //{{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueReadImage",[CL.cl_objects[image],blocking_read,_origin,_region,row_pitch,_host_ptr,_event_wait_list]);
#endif  
        CL.cl_objects[command_queue].enqueueReadImage(CL.cl_objects[image],blocking_read,_origin,_region,row_pitch,_host_ptr,_event_wait_list);
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    return webcl.SUCCESS; 
  },

  /**
   * Description
   * @method clEnqueueWriteImage
   * @param {} command_queue
   * @param {} image
   * @param {} blocking_write
   * @param {} origin
   * @param {} region
   * @param {} input_row_pitch
   * @param {} input_slice_pitch
   * @param {} ptr
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueWriteImage: function(command_queue,image,blocking_write,origin,region,input_row_pitch,input_slice_pitch,ptr,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueWriteImage",[command_queue,image,blocking_write,origin,region,input_row_pitch,input_slice_pitch,ptr,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(image in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+image+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    var _event_wait_list = [];

    var _origin = [];
    var _region = [];

    var _size = CL.getImageSizeType(image);
    var _channel = CL.getImageFormatType(image);

    for (var i = 0; i < 2; i++) {
      _origin.push({{{ makeGetValue('origin', 'i*4', 'i32') }}});
      _region.push({{{ makeGetValue('region', 'i*4', 'i32') }}});  
      _size *= _region[i];     
    }          

    // console.info("/!\\ clEnqueueWriteImage : Check the size of the ptr '"+_size+"'... need to be more tested");
    var _host_ptr = CL.getReferencePointerToArray(ptr,_size,[_channel,1]);

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif

      _event_wait_list.push(CL.cl_objects[_event_wait]);

    } 

    try {

      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueWriteImage",[CL.cl_objects[image],blocking_write,_origin,_region,input_row_pitch,_host_ptr,_event_wait_list,_event]);
#endif               
        CL.cl_objects[command_queue].enqueueWriteImage(CL.cl_objects[image],blocking_write,_origin,_region,input_row_pitch,_host_ptr,_event_wait_list,_event);
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueWriteImage",[CL.cl_objects[image],blocking_write,_origin,_region,input_row_pitch,_host_ptr,_event_wait_list]);
#endif             
        CL.cl_objects[command_queue].enqueueWriteImage(CL.cl_objects[image],blocking_write,_origin,_region,input_row_pitch,_host_ptr,_event_wait_list);
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif   
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif

    return webcl.SUCCESS;  
  },

  /**
   * Description
   * @method clEnqueueCopyImage
   * @param {} command_queue
   * @param {} src_image
   * @param {} dst_image
   * @param {} src_origin
   * @param {} dst_origin
   * @param {} region
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueCopyImage: function(command_queue,src_image,dst_image,src_origin,dst_origin,region,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueCopyImage",[command_queue,src_image,dst_image,src_origin,dst_origin,region,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(src_image in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+src_image+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(dst_image in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+dst_image+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif

    var _event_wait_list = [];

    var _src_origin = [];
    var _dest_origin = [];
    var _region = [];

    for (var i = 0; i < 2; i++) {
      _src_origin.push({{{ makeGetValue('src_origin', 'i*4', 'i32') }}});
      _dest_origin.push({{{ makeGetValue('dst_origin', 'i*4', 'i32') }}});
      _region.push({{{ makeGetValue('region', 'i*4', 'i32') }}});            
    }

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif
      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 

    try {

      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyImage",[CL.cl_objects[src_image],CL.cl_objects[dst_image],_src_origin,_dest_origin,_region,_event_wait_list,_event]);
#endif 
        CL.cl_objects[command_queue].enqueueCopyImage(CL.cl_objects[src_image],CL.cl_objects[dst_image],_src_origin,_dest_origin,_region,_event_wait_list,_event);    
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyImage",[CL.cl_objects[src_image],CL.cl_objects[dst_image],_src_origin,_dest_origin,_region,_event_wait_list]);
#endif       
        CL.cl_objects[command_queue].enqueueCopyImage(CL.cl_objects[src_image],CL.cl_objects[dst_image],_src_origin,_dest_origin,_region,_event_wait_list);    
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }
          
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clEnqueueCopyImageToBuffer
   * @param {} command_queue
   * @param {} src_image
   * @param {} dst_buffer
   * @param {} src_origin
   * @param {} region
   * @param {} dst_offset
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueCopyImageToBuffer: function(command_queue,src_image,dst_buffer,src_origin,region,dst_offset,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueCopyImageToBuffer",[command_queue,src_image,dst_buffer,src_origin,region,dst_offset,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(src_image in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+src_image+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(dst_buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+dst_buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    var _event_wait_list = [];

    var _src_origin = [];
    var _region = [];

    for (var i = 0; i < 2; i++) {
      _src_origin.push({{{ makeGetValue('src_origin', 'i*4', 'i32') }}});
      _region.push({{{ makeGetValue('region', 'i*4', 'i32') }}});            
    }

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif

      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 
 
    try {

      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyImageToBuffer",[CL.cl_objects[src_image],CL.cl_objects[dst_buffer],_src_origin,_region,dst_offset,_event_wait_list,_event]);
#endif    
        CL.cl_objects[command_queue].enqueueCopyImageToBuffer(CL.cl_objects[src_image],CL.cl_objects[dst_buffer],_src_origin,_region,dst_offset,_event_wait_list,_event);    
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyImageToBuffer",[CL.cl_objects[src_image],CL.cl_objects[dst_buffer],_src_origin,_region,dst_offset,_event_wait_list]);
#endif          
        CL.cl_objects[command_queue].enqueueCopyImageToBuffer(CL.cl_objects[src_image],CL.cl_objects[dst_buffer],_src_origin,_region,dst_offset,_event_wait_list);    
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clEnqueueCopyBufferToImage
   * @param {} command_queue
   * @param {} src_buffer
   * @param {} dst_image
   * @param {} src_offset
   * @param {} dst_origin
   * @param {} region
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueCopyBufferToImage: function(command_queue,src_buffer,dst_image,src_offset,dst_origin,region,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueCopyBufferToImage",[command_queue,src_buffer,dst_image,src_offset,dst_origin,region,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(src_buffer in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+src_buffer+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(dst_image in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+dst_image+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 

    var _event_wait_list = [];

    var _dest_origin = []; 
    var _region = []; 

    for (var i = 0; i < 2; i++) {
      _dest_origin.push({{{ makeGetValue('dst_origin', 'i*4', 'i32') }}});
      _region.push({{{ makeGetValue('region', 'i*4', 'i32') }}});            
    }

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif

      _event_wait_list.push(CL.cl_objects[_event_wait]);
    } 
  
    try {

      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyBufferToImage",[CL.cl_objects[src_buffer],CL.cl_objects[dst_image],src_offset,_dest_origin,_region,_event_wait_list,_event]);
#endif    
        CL.cl_objects[command_queue].enqueueCopyBufferToImage(CL.cl_objects[src_buffer],CL.cl_objects[dst_image],src_offset,_dest_origin,_region,_event_wait_list,_event);    
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueCopyBufferToImage",[CL.cl_objects[src_buffer],CL.cl_objects[dst_image],src_offset,_dest_origin,_region,_event_wait_list]);
#endif          
        CL.cl_objects[command_queue].enqueueCopyBufferToImage(CL.cl_objects[src_buffer],CL.cl_objects[dst_image],src_offset,_dest_origin,_region,_event_wait_list);    
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clEnqueueMapBuffer
   * @param {} command_queue
   * @param {} buffer
   * @param {} blocking_map
   * @param {} map_flags_i64_1
   * @param {} map_flags_i64_2
   * @param {} offset
   * @param {} cb
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @param {} cl_errcode_ret
   * @return MemberExpression
   */
  clEnqueueMapBuffer__deps: ['clEnqueueReadBuffer'],
  clEnqueueMapBuffer: function(command_queue,buffer,blocking_map,map_flags_i64_1,map_flags_i64_2,offset,cb,num_events_in_wait_list,event_wait_list,event,cl_errcode_ret) {
#if ASSERTIONS       
    // Assume the map_flags is i32 
    assert(map_flags_i64_2 == 0, 'Invalid map flags i64');
#endif
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueMapBuffer",[command_queue,buffer,blocking_map,map_flags_i64_1,offset,cb,num_events_in_wait_list,event_wait_list,event,cl_errcode_ret]);
#endif
#if CL_CHECK_SET_POINTER    
    if (CL.cl_pn_type.length == 0) console.info("/!\\ clEnqueueMapBuffer : you don't call clSetTypePointer");
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_CHECK_SET_POINTER    
      CL.cl_pn_type = [];
#endif        
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_COMMAND_QUEUE', 'i32') }}};
      }
      return 0;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(buffer in CL.cl_objects)) {
#if CL_CHECK_SET_POINTER    
      CL.cl_pn_type = [];
#endif  
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+buffer+"' are not inside the map","");
#endif
      if (cl_errcode_ret != 0) {
        {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_MEM_OBJECT', 'i32') }}};
      }
      return 0;
    }
#endif 

    var mapped_ptr = _malloc(cb);

    // { SIZE , BLOCKING_MAP , OFFSET }
    CL.cl_objects_map[mapped_ptr] = {"size":cb,"blocking":blocking_map,"offset":offset,"mode":map_flags_i64_1};

    if (CL.cl_objects_map[mapped_ptr]["mode"] == 0x1 /*webcl.MAP_READ*/) {

      // Call write buffer .... may be add try ... catch
      _clEnqueueReadBuffer(command_queue,buffer,CL.cl_objects_map[mapped_ptr]["blocking"],CL.cl_objects_map[mapped_ptr]["offset"],CL.cl_objects_map[mapped_ptr]["size"],mapped_ptr,num_events_in_wait_list,event_wait_list,event);
    
    }

#if CL_CHECK_SET_POINTER    
    CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([mapped_ptr],"","");
#endif

    if (cl_errcode_ret != 0) {
      {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.SUCCESS', 'i32') }}};
    }

    return mapped_ptr;
  },

  /**
   * Description
   * @method clEnqueueMapImage
   * @param {} command_queue
   * @param {} image
   * @param {} blocking_map
   * @param {} map_flags_i64_1
   * @param {} map_flags_i64_2
   * @param {} origin
   * @param {} region
   * @param {} image_row_pitch
   * @param {} image_slice_pitch
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @param {} cl_errcode_ret
   * @return MemberExpression
   */
  clEnqueueMapImage: function(command_queue,image,blocking_map,map_flags_i64_1,map_flags_i64_2,origin,region,image_row_pitch,image_slice_pitch,num_events_in_wait_list,event_wait_list,event,cl_errcode_ret) {
#if ASSERTIONS    
    // Assume the map_flags is i32 
    assert(map_flags_i64_2 == 0, 'Invalid map flags i64');
#endif

    console.error("clEnqueueMapImage: Can't be implemented - Differences between WebCL and OpenCL 1.1\n");

    return webcl.INVALID_VALUE; 
  },

  /**
   * Description
   * @method clEnqueueUnmapMemObject
   * @param {} command_queue
   * @param {} memobj
   * @param {} mapped_ptr
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueUnmapMemObject__deps: ['clEnqueueWriteBuffer'],
  clEnqueueUnmapMemObject: function(command_queue,memobj,mapped_ptr,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueUnmapMemObject",[command_queue,memobj,mapped_ptr,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_SET_POINTER    
    if (CL.cl_pn_type.length == 0) console.info("/!\\ clEnqueueUnmapMemObject : you don't call clSetTypePointer");
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_CHECK_SET_POINTER    
      CL.cl_pn_type = [];
#endif        
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(memobj in CL.cl_objects)) {
#if CL_CHECK_SET_POINTER    
      CL.cl_pn_type = [];
#endif        
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+memobj+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif 
//#if CL_CHECK_VALID_OBJECT   
    // If the call is comming from clEnqueueMapImage the Unmap can't work
    if (!(mapped_ptr in CL.cl_objects_map)) {
#if CL_CHECK_SET_POINTER    
      CL.cl_pn_type = [];
#endif       
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"MappedPtr '"+mapped_ptr+"' are not inside the map objects","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
//#endif 

    if (CL.cl_objects_map[mapped_ptr]["mode"] == 0x2 /*webcl.MAP_WRITE*/) {

      // Call write buffer .... may be add try ... catch
      _clEnqueueWriteBuffer(command_queue,memobj,CL.cl_objects_map[mapped_ptr]["blocking"],CL.cl_objects_map[mapped_ptr]["offset"],CL.cl_objects_map[mapped_ptr]["size"],mapped_ptr,num_events_in_wait_list,event_wait_list,event);
    
    }

    // Remove the object from the map
    delete CL.cl_objects[mapped_ptr];

    // Free malloc
    _free(mapped_ptr);

#if CL_CHECK_SET_POINTER    
    CL.cl_pn_type = [];
#endif 
#if CL_GRAB_TRACE
    CL.webclEndStackTrace([mapped_ptr],"","");
#endif

    return mapped_ptr; 
  },

  /**
   * Description
   * @method clEnqueueNDRangeKernel
   * @param {} command_queue
   * @param {} kernel
   * @param {} work_dim
   * @param {} global_work_offset
   * @param {} global_work_size
   * @param {} local_work_size
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueNDRangeKernel: function(command_queue,kernel,work_dim,global_work_offset,global_work_size,local_work_size,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueNDRangeKernel",[command_queue,kernel,work_dim,global_work_offset,global_work_size,local_work_size,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(kernel in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_KERNEL],"WebCLKernel '"+kernel+"' are not inside the map","");
#endif
      return webcl.INVALID_KERNEL;
    }
#endif 

    var _event_wait_list;
    var _local_work_size;

    // \todo need to be remove when webkit will be support null
    /**** **** **** **** **** **** **** ****/
    if (navigator.userAgent.toLowerCase().indexOf('firefox') != -1) {
      _event_wait_list = num_events_in_wait_list > 0 ? [] : null;
      _local_work_size = (local_work_size != 0) ? [] : null;
    } else {
      _event_wait_list = [];
      _local_work_size = [];
    }


    var _global_work_offset = [];
    var _global_work_size = [];
    

    for (var i = 0; i < work_dim; i++) {
      _global_work_size.push({{{ makeGetValue('global_work_size', 'i*4', 'i32') }}});

      if (global_work_offset != 0)
        _global_work_offset.push({{{ makeGetValue('global_work_offset', 'i*4', 'i32') }}});
    
      if (local_work_size != 0)
        _local_work_size.push({{{ makeGetValue('local_work_size', 'i*4', 'i32') }}});
    }

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT   
      if (!(_event_wait in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif 
       
      _event_wait_list.push(CL.cl_objects[_event_wait]);
    }
           
    try { 
      
      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueNDRangeKernel",[CL.cl_objects[kernel],work_dim,_global_work_offset,_global_work_size,_local_work_size,_event_wait_list,_event]);
#endif    
        CL.cl_objects[command_queue].enqueueNDRangeKernel(CL.cl_objects[kernel],work_dim,_global_work_offset,_global_work_size,_local_work_size,_event_wait_list,_event);  
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueNDRangeKernel",[CL.cl_objects[kernel],work_dim,_global_work_offset,_global_work_size,_local_work_size,_event_wait_list]);
#endif          
        CL.cl_objects[command_queue].enqueueNDRangeKernel(CL.cl_objects[kernel],work_dim,_global_work_offset,_global_work_size,_local_work_size,_event_wait_list);  
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS;    

  },

  /**
   * Description
   * @method clEnqueueTask
   * @param {} command_queue
   * @param {} kernel
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueTask__deps: ['clEnqueueNDRangeKernel'],
  clEnqueueTask: function(command_queue,kernel,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueTask",[command_queue,kernel,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {    
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif 
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif 
#if CL_CHECK_VALID_OBJECT   
    if (!(kernel in CL.cl_objects)) {    
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_KERNEL],"WebCLKernel '"+kernel+"' are not inside the map","");
#endif 
      return webcl.INVALID_KERNEL;
    }
#endif 

    var global_work_size = _malloc(4);
    setValue(global_work_size, 1, 'i32');

    var local_work_size = _malloc(4);
    setValue(local_work_size, 1, 'i32');

    var _res = _clEnqueueNDRangeKernel(command_queue,kernel,1,0,global_work_size,local_work_size,num_events_in_wait_list,event_wait_list,event);

    _free(global_work_size);
    _free(local_work_size);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_res],"","");
#endif

    return _res;
  },

  /**
   * Description
   * @method clEnqueueNativeKernel
   * @param {} command_queue
   * @param {} user_func
   * @param {} args
   * @param {} cb_args
   * @param {} num_mem_objects
   * @param {} mem_list
   * @param {} args_mem_loc
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueNativeKernel: function(command_queue,user_func,args,cb_args,num_mem_objects,mem_list,args_mem_loc,num_events_in_wait_list,event_wait_list,event) {
    console.error("clEnqueueNativeKernel: Can't be implemented - Differences between WebCL and OpenCL 1.1\n");

    return webcl.INVALID_VALUE; 
  },

  /**
   * Description
   * @method clEnqueueMarker
   * @param {} command_queue
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueMarker: function(command_queue,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueMarker",[command_queue,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif     

    try { 

      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueMarker",[_event]);
#endif            
        CL.cl_objects[command_queue].enqueueMarker(_event);    
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueMarker",[]);
#endif          
        CL.cl_objects[command_queue].enqueueMarker();    
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS; 
  },

  /**
   * Description
   * @method clEnqueueWaitForEvents
   * @param {} command_queue
   * @param {} num_events
   * @param {} event_list
   * @return MemberExpression
   */
  clEnqueueWaitForEvents: function(command_queue,num_events,event_list) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueWaitForEvents",[command_queue,num_events,event_list]);
#endif
#if CL_CHECK_VALID_OBJECT       
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif
      return webcl.INVALID_COMMAND_QUEUE; 
    }
#endif

    var _events = [];

    for (var i = 0; i < num_events; i++) {
      var _event = {{{ makeGetValue('event_list', 'i*4', 'i32') }}};
#if CL_CHECK_VALID_OBJECT       
      if (!(_event in CL.cl_objects)) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([0,cl_errcode_ret],"WebCLEvent '"+_event+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT; 
      }
#endif
      
      _events.push(CL.cl_objects[_event])

    } 
     
#if CL_GRAB_TRACE
    CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueWaitForEvents",[_events]);
#endif  

    try { 
      
      CL.cl_objects[command_queue].enqueueWaitForEvents(_events);   

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS; 
  },

  /**
   * Description
   * @method clEnqueueBarrier
   * @param {} command_queue
   * @return MemberExpression
   */
  clEnqueueBarrier: function(command_queue) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueBarrier",[command_queue]);
#endif
#if CL_CHECK_VALID_OBJECT       
    if (!(command_queue in CL.cl_objects)) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif
      return webcl.INVALID_COMMAND_QUEUE; 
    }
#endif

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueBarrier",[]);
#endif    
    
    try {
      
      CL.cl_objects[command_queue].enqueueBarrier(); 

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS; 
  },

  /**
   * Description
   * @method clGetExtensionFunctionAddress
   * @param {} func_name
   * @return MemberExpression
   */
  clGetExtensionFunctionAddress: function(func_name) {
    console.error("clGetExtensionFunctionAddress: Not yet implemented\n");
    return webcl.INVALID_VALUE;
  },

  /**
   * Description
   * @method clCreateFromGLBuffer
   * @param {} context
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} bufobj
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateFromGLBuffer: function(context,flags_i64_1,flags_i64_2,bufobj,cl_errcode_ret) {
#if ASSERTIONS    
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');
#endif
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateFromGLBuffer",[context,flags_i64_1,bufobj,cl_errcode_ret]);
#endif
#if CL_CHECK_VALID_OBJECT       
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"WebCLContext '"+context+"' are not inside the map","");
#endif
      return 0; 
    }
#endif
 
    var _id = null;
    var _buffer = null;
    var _flags;

    if (flags_i64_1 & webcl.MEM_READ_WRITE) {
      _flags = webcl.MEM_READ_WRITE;
    } else if (flags_i64_1 & webcl.MEM_WRITE_ONLY) {
      _flags = webcl.MEM_WRITE_ONLY;
    } else if (flags_i64_1 & webcl.MEM_READ_ONLY) {
      _flags = webcl.MEM_READ_ONLY;
    } else {
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"values specified "+flags_i64_1+" in flags are not valid","");
#endif
      return 0; 
    }

#if CL_GRAB_TRACE
    CL.webclCallStackTrace( CL.cl_objects[context]+".createFromGLBuffer",[_flags,GL.buffers[bufobj]]);
#endif   

    try {

      _buffer = CL.cl_objects[context].createFromGLBuffer(_flags,GL.buffers[bufobj]);

    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};

    _id = CL.udid(_buffer);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clCreateFromGLTexture
   * @param {} context
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} target
   * @param {} miplevel
   * @param {} texture
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateFromGLTexture: function(context,flags_i64_1,flags_i64_2,target,miplevel,texture,cl_errcode_ret) {
#if ASSERTIONS    
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');
#endif
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateFromGLTexture",[context,flags_i64_1,target,miplevel,texture,cl_errcode_ret]);
#endif
#if CL_CHECK_VALID_OBJECT       
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"WebCLContext '"+context+"' are not inside the map","");
#endif
      return 0; 
    }
#endif

    var _id = null;
    var _buffer = null;
    var _flags;

    if (flags_i64_1 & webcl.MEM_READ_WRITE) {
      _flags = webcl.MEM_READ_WRITE;
    } else if (flags_i64_1 & webcl.MEM_WRITE_ONLY) {
      _flags = webcl.MEM_WRITE_ONLY;
    } else if (flags_i64_1 & webcl.MEM_READ_ONLY) {
      _flags = webcl.MEM_READ_ONLY;
    } else {
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"values specified "+flags_i64_1+" in flags are not valid","");
#endif
      return 0; 
    }

#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[context]+".createFromGLTexture",[_flags, target, miplevel, GL.textures[texture]]);
#endif      

    try {
      
      _buffer = CL.cl_objects[context].createFromGLTexture(_flags, target, miplevel, GL.textures[texture]);

    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};

    _id = CL.udid(_buffer);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clCreateFromGLTexture2D
   * @param {} context
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} target
   * @param {} miplevel
   * @param {} texture
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateFromGLTexture2D: function(context,flags_i64_1,flags_i64_2,target,miplevel,texture,cl_errcode_ret) {
#if ASSERTIONS    
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');
#endif
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateFromGLTexture2D",[context,flags_i64_1,target,miplevel,texture,cl_errcode_ret]);
#endif
#if CL_CHECK_VALID_OBJECT       
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"WebCLContext '"+context+"' are not inside the map","");
#endif
      return 0; 
    }
#endif

    var _id = null;
    var _buffer = null;
    var _flags;

    if (flags_i64_1 & webcl.MEM_READ_WRITE) {
      _flags = webcl.MEM_READ_WRITE;
    } else if (flags_i64_1 & webcl.MEM_WRITE_ONLY) {
      _flags = webcl.MEM_WRITE_ONLY;
    } else if (flags_i64_1 & webcl.MEM_READ_ONLY) {
      _flags = webcl.MEM_READ_ONLY;
    } else {
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"values specified "+flags_i64_1+" in flags are not valid","");
#endif
      return 0; 
    }


#if CL_GRAB_TRACE
    CL.webclCallStackTrace( CL.cl_objects[context]+".createFromGLTexture",[_flags, target, miplevel, GL.textures[texture]]);
#endif    

    try {

      _buffer = CL.cl_objects[context].createFromGLTexture(_flags, target, miplevel, GL.textures[texture]);

    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};
    
    _id = CL.udid(_buffer);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;
  },

  /**
   * Description
   * @method clCreateFromGLTexture3D
   * @param {} context
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} target
   * @param {} miplevel
   * @param {} texture
   * @param {} cl_errcode_ret
   * @return Literal
   */
  clCreateFromGLTexture3D: function(context,flags_i64_1,flags_i64_2,target,miplevel,texture,cl_errcode_ret) {
#if ASSERTIONS
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');
#endif    
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateFromGLTexture3D",[context,flags_i64_1,target,miplevel,texture,cl_errcode_ret]);
#endif

    console.error("clCreateFromGLTexture3D: Can't be implemented - Differences between WebCL and OpenCL 1.1\n");

    if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};

    return 0;
  },

  /**
   * Description
   * @method clCreateFromGLRenderbuffer
   * @param {} context
   * @param {} flags_i64_1
   * @param {} flags_i64_2
   * @param {} renderbuffer
   * @param {} cl_errcode_ret
   * @return _id
   */
  clCreateFromGLRenderbuffer: function(context,flags_i64_1,flags_i64_2,renderbuffer,cl_errcode_ret) {
#if ASSERTIONS    
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');
#endif
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clCreateFromGLRenderbuffer",[context,flags_i64_1,renderbuffer,cl_errcode_ret]);
#endif
#if CL_CHECK_VALID_OBJECT       
    if (!(context in CL.cl_objects)) {
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_CONTEXT', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"WebCLContext '"+context+"' are not inside the map","");
#endif
      return 0; 
    }
#endif

    var _id = null;
    var _buffer = null;    
    var _flags;

    if (flags_i64_1 & webcl.MEM_READ_WRITE) {
      _flags = webcl.MEM_READ_WRITE;
    } else if (flags_i64_1 & webcl.MEM_WRITE_ONLY) {
      _flags = webcl.MEM_WRITE_ONLY;
    } else if (flags_i64_1 & webcl.MEM_READ_ONLY) {
      _flags = webcl.MEM_READ_ONLY;
    } else {
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', 'webcl.INVALID_VALUE', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"values specified "+flags_i64_1+" in flags are not valid","");
#endif
      return 0; 
    }

#if CL_GRAB_TRACE
      CL.webclCallStackTrace( CL.cl_objects[context]+".createFromGLRenderbuffer",[_flags, GL.renderbuffers[renderbuffer]]);
#endif      
    try {

      _buffer = CL.cl_objects[context].createFromGLRenderbuffer(_flags, GL.renderbuffers[renderbuffer]);

    } catch (e) {
      var _error = CL.catchError(e);
    
      if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '_error', 'i32') }}};
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([0,cl_errcode_ret],"",e.message);
#endif
      return 0; // NULL Pointer
    }

    if (cl_errcode_ret != 0) {{{ makeSetValue('cl_errcode_ret', '0', '0', 'i32') }}};

    _id = CL.udid(_buffer);

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([_id,cl_errcode_ret],"","");
#endif

    return _id;  
  },

  /**
   * Description
   * @method clGetGLObjectInfo
   * @param {} memobj
   * @param {} gl_object_type
   * @param {} gl_object_name
   * @return MemberExpression
   */
  clGetGLObjectInfo: function(memobj,gl_object_type,gl_object_name) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetGLObjectInfo",[memobj,gl_object_type,gl_object_name]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!memobj in CL.cl_objects) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+memobj+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif

#if CL_GRAB_TRACE
    CL.webclCallStackTrace(""+CL.cl_objects[memobj]+".getGLObjectInfo",[]);
#endif        

    try { 

      var _info = CL.cl_objects[memobj].getGLObjectInfo();

      if (gl_object_type != 0) {{{ makeSetValue('gl_object_type', '0', '_info.type', 'i32') }}};
      if (gl_object_name != 0) {{{ makeSetValue('gl_object_name', '0', '_info.glObject', 'i32') }}};  

    } catch (e) {

      var _error = CL.catchError(e);

      if (gl_object_type != 0) {{{ makeSetValue('gl_object_type', '0', '0', 'i32') }}};
      if (gl_object_name != 0) {{{ makeSetValue('gl_object_name', '0', '0', 'i32') }}};

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,gl_object_type,gl_object_name],"",e.message);
#endif
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,gl_object_type,gl_object_name],"","");
#endif

    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clGetGLTextureInfo
   * @param {} memobj
   * @param {} param_name
   * @param {} param_value_size
   * @param {} param_value
   * @param {} param_value_size_ret
   * @return MemberExpression
   */
  clGetGLTextureInfo: function(memobj,param_name,param_value_size,param_value,param_value_size_ret) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clGetGLTextureInfo",[memobj,param_name,param_value_size,param_value,param_value_size_ret]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!memobj in CL.cl_objects) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+memobj+"' are not inside the map","");
#endif
      return webcl.INVALID_MEM_OBJECT;
    }
#endif

#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[memobj]+".getGLTextureInfo",[param_name]);
#endif        

    try {
      
      var _info = CL.cl_objects[memobj].getGLTextureInfo(param_name);

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '_info', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '4', 'i32') }}};

    } catch (e) {
      var _error = CL.catchError(e);

      if (param_value != 0) {{{ makeSetValue('param_value', '0', '0', 'i32') }}};
      if (param_value_size_ret != 0) {{{ makeSetValue('param_value_size_ret', '0', '0', 'i32') }}};
    
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error,param_value,param_value_size_ret],"",e.message);
#endif
      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS,param_value,param_value_size_ret],"","");
#endif
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clEnqueueAcquireGLObjects
   * @param {} command_queue
   * @param {} num_objects
   * @param {} mem_objects
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueAcquireGLObjects: function(command_queue,num_objects,mem_objects,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueAcquireGLObjects",[command_queue,num_objects,mem_objects,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!command_queue in CL.cl_objects) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif
      
    var _event_wait_list = [];
    var _mem_objects = [];

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};

#if CL_CHECK_VALID_OBJECT   
      if (!_event_wait in CL.cl_objects) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif
      
      _event_wait_list.push(CL.cl_objects[_event_wait]);
    }

    for (var i = 0; i < num_objects; i++) {
      var _id = {{{ makeGetValue('mem_objects', 'i*4', 'i32') }}};

#if CL_CHECK_VALID_OBJECT   
      if (!_id in CL.cl_objects) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+_id+"' are not inside the map","");
#endif
        return webcl.INVALID_MEM_OBJECT;
      }
#endif
      
      _mem_objects.push(CL.cl_objects[_id]);
    }

    try { 

      if (event != 0) {
        var _event = new WebCLEvent();
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueAcquireGLObjects",[_mem_objects,_event_wait_list,_event]);
#endif    
        CL.cl_objects[command_queue].enqueueAcquireGLObjects(_mem_objects,_event_wait_list,_event); 
        {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};
      } else {
#if CL_GRAB_TRACE
        CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueAcquireGLObjects",[_mem_objects,_event_wait_list]);
#endif          
        CL.cl_objects[command_queue].enqueueAcquireGLObjects(_mem_objects,_event_wait_list);    
      }

    } catch (e) {
      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS;
  },

  /**
   * Description
   * @method clEnqueueReleaseGLObjects
   * @param {} command_queue
   * @param {} num_objects
   * @param {} mem_objects
   * @param {} num_events_in_wait_list
   * @param {} event_wait_list
   * @param {} event
   * @return MemberExpression
   */
  clEnqueueReleaseGLObjects: function(command_queue,num_objects,mem_objects,num_events_in_wait_list,event_wait_list,event) {
#if CL_GRAB_TRACE
    CL.webclBeginStackTrace("clEnqueueReleaseGLObjects",[command_queue,num_objects,mem_objects,num_events_in_wait_list,event_wait_list,event]);
#endif
#if CL_CHECK_VALID_OBJECT   
    if (!command_queue in CL.cl_objects) {
#if CL_GRAB_TRACE
      CL.webclEndStackTrace([webcl.INVALID_COMMAND_QUEUE],"WebCLCommandQueue '"+command_queue+"' are not inside the map","");
#endif
      return webcl.INVALID_COMMAND_QUEUE;
    }
#endif
      
    var _event_wait_list = [];
    var _mem_objects = [];

    for (var i = 0; i < num_events_in_wait_list; i++) {
      var _event_wait = {{{ makeGetValue('event_wait_list', 'i*4', 'i32') }}};

#if CL_CHECK_VALID_OBJECT   
      if (!_event_wait in CL.cl_objects) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_EVENT],"WebCLEvent '"+_event_wait+"' are not inside the map","");
#endif
        return webcl.INVALID_EVENT;
      }
#endif
      
      _event_wait_list.push(CL.cl_objects[_event_wait]);
    }

    for (var i = 0; i < num_objects; i++) {
      var _id = {{{ makeGetValue('mem_objects', 'i*4', 'i32') }}};

#if CL_CHECK_VALID_OBJECT   
      if (!_id in CL.cl_objects) {
#if CL_GRAB_TRACE
        CL.webclEndStackTrace([webcl.INVALID_MEM_OBJECT],"WebCLBuffer '"+_id+"' are not inside the map","");
#endif
        return webcl.INVALID_MEM_OBJECT;
      }
#endif
      
      _mem_objects.push(CL.cl_objects[_id]);
    }

    try { 

        if (event != 0) {
          var _event = new WebCLEvent();
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueReleaseGLObjects",[_mem_objects,_event_wait_list,_event]);
#endif            
          CL.cl_objects[command_queue].enqueueReleaseGLObjects(_mem_objects,_event_wait_list,_event);    
          {{{ makeSetValue('event', '0', 'CL.udid(_event)', 'i32') }}};  
        } else {
#if CL_GRAB_TRACE
          CL.webclCallStackTrace(""+CL.cl_objects[command_queue]+".enqueueReleaseGLObjects",[_mem_objects,_event_wait_list]);
#endif  
          CL.cl_objects[command_queue].enqueueReleaseGLObjects(_mem_objects,_event_wait_list);      
        }

    } catch (e) {

      var _error = CL.catchError(e);

#if CL_GRAB_TRACE
      CL.webclEndStackTrace([_error],"",e.message);
#endif

      return _error;
    }

#if CL_GRAB_TRACE
    CL.webclEndStackTrace([webcl.SUCCESS],"","");
#endif
    
    return webcl.SUCCESS;
  },

};

autoAddDeps(LibraryOpenCL, '$CL');
mergeInto(LibraryManager.library, LibraryOpenCL);

