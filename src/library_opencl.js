//"use strict";

var LibraryOpenCL = {
  $CL__deps: ['$Browser'],
  $CL: {
    address_space: {GENERAL:0, GLOBAL:1, LOCAL:2, CONSTANT:4, PRIVATE:8},
    data_type: {FLOAT:16,INT:32,UINT:64},
    device_infos : {},
    ctx: [],
    webcl_mozilla: 0,
    webcl_webkit: 0,
    ctx_clean: 0,
    cmdQueue: [],
    cmdQueue_clean: 0,
    programs: [],
    programs_clean: 0,
    kernels: [],
    kernels_name: [],
    kernels_sig: {},
    kernels_clean: 0,
    buffers: [],
    buffers_clean: 0,
    platforms: [],
    devices: [],
    errorMessage: "Unfortunately your system does not support WebCL. " +
                    "Make sure that you have both the OpenCL driver " +
                    "and the WebCL browser extension installed.",
      
    checkWebCL: function() {
      // If we already check is not useful to do this again
      if (CL.webcl_webkit == 1 || CL.webcl_mozilla == 1) {
        return 0;
      }
              
      // Look is the browser is comaptible
      var isWebkit = 'webkitRequestAnimationFrame' in window;
      var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
      
      if (!isWebkit && !isFirefox) {
        console.error("This current browser is not compatible with WebCL implementation !!! \n");
        console.error("Use WebKit Samsung or Firefox Nokia plugin\n");            
        return -1;
      }
      
      // Look is the browser have WebCL implementation
      if (window.WebCL == undefined || isWebkit) {
        if (typeof(webcl) === "undefined") {
          console.error("This browser has not WebCL implementation !!! \n");
          console.error("Use WebKit Samsung or Firefox Nokia plugin\n");            
          return -1;
        } else {
          window.WebCL = webcl
        }
      }
      
      // Init Device info
      CL.device_infos = {
        0x1000:[WebCL.CL_DEVICE_TYPE,WebCL.DEVICE_TYPE],
        0x1001:[WebCL.CL_DEVICE_VENDOR_ID,WebCL.DEVICE_VENDOR_ID],
        0x1002:[WebCL.CL_DEVICE_MAX_COMPUTE_UNITS,WebCL.DEVICE_MAX_COMPUTE_UNITS],
        0x1003:[WebCL.CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,WebCL.DEVICE_MAX_WORK_ITEM_DIMENSIONS],      
        0x1004:[WebCL.CL_DEVICE_MAX_WORK_GROUP_SIZE,WebCL.DEVICE_MAX_WORK_GROUP_SIZE],
        0x1016:[WebCL.CL_DEVICE_IMAGE_SUPPORT,WebCL.DEVICE_IMAGE_SUPPORT],
        0x1030:[WebCL.CL_DEVICE_EXTENSIONS,WebCL.DEVICE_EXTENSIONS],
        0x102B:[WebCL.CL_DEVICE_NAME,WebCL.DEVICE_NAME],
        0x102C:[WebCL.CL_DEVICE_VENDOR,WebCL.DEVICE_VENDOR],
        0x102D:[WebCL.CL_DRIVER_VERSION,WebCL.DRIVER_VERSION],
        0x102E:[WebCL.CL_DEVICE_PROFILE,WebCL.DEVICE_PROFILE],
        0x102F:[WebCL.CL_DEVICE_VERSION,WebCL.DEVICE_VERSION]            
      };
      
      CL.webcl_webkit = isWebkit == true ? 1 : 0;
      CL.webcl_mozilla = isFirefox == true ? 1 : 0;
  
#if OPENCL_DEBUG
      var browser = (CL.webcl_mozilla == 1) ? "Mozilla" : "Webkit";
      console.info("Webcl implemented for "+browser);
#endif
      
      return 0;
    },
    
    // Check if the data inside ptr are float or int, it's dirty and need to find something more clean but
    // clCreateBuffer need Float32Array or Uint32Array
    // clEnqueueWriteBuffer need Float32Array or Uint32Array
    // clEnqueueReadBuffer need Float32Array or Uint32Array
    isFloat: function(ptr,size) {
      
      console.error("CL.isFloat not must be called any more ... use the parse of kernel string !!! \n");
      console.error("But may be the kernel source is not yet parse !!! \n");
            
      var v_int = {{{ makeGetValue('ptr', '0', 'i32') }}}; 
      var v_float = {{{ makeGetValue('ptr', '0', 'float') }}}; 
              
      // If the value is 0
      if ( v_int == 0 ) {

        // If is an array
        if (size > 1) {
          v_int = {{{ makeGetValue('ptr', 'size - 1', 'i32') }}}; 
          v_float = {{{ makeGetValue('ptr', 'size - 1', 'float') }}};     
        } else { 
          // Use float by default 
          return 1;
        }                
      }

      // If we read int and is float we have a very big value 1e8
      if (Math.abs(v_int) > 100000000) {
        return 1;
      }

      return 0;      
    },
    
    parseKernel: function(kernelstring) {
      
      // Experimental parse of Kernel
      // Search kernel function like __kernel ... NAME ( p1 , p2 , p3)  
      // Step 1 : Search __kernel
      // Step 2 : Search kernel name (before the open brace)
      // Step 3 : Search brace '(' and ')'
      // Step 4 : Split all inside the brace by ',' after removing all space
      // Step 5 : For each parameter search Adress Space and Data Type
      //
      // --------------------------------------------------------------------
      //
      // \note Work only with one kernel ....
                  
      var kernel_struct = {};
    
      kernelstring = kernelstring.replace(/\n/g, " ");
      kernelstring = kernelstring.replace(/\r/g, " ");
      kernelstring = kernelstring.replace(/\t/g, " ");
      
      // Search kernel function __kernel 
      var kernel_start = kernelstring.indexOf("__kernel");

      while (kernel_start >= 0) {

        kernelstring = kernelstring.substr(kernel_start,kernelstring.length-kernel_start);
      
        var brace_start = kernelstring.indexOf("(");
        var brace_end = kernelstring.indexOf(")");  
      
        var kernels_name = "";
        // Search kernel Name
        for (var i = brace_start - 1; i >= 0 ; i--) {
          var chara = kernelstring.charAt(i);

          if (chara == ' ' && kernels_name.length > 0) {
            break;
          } else if (chara != ' ') {
            kernels_name = chara + kernels_name;
          }
        }
      
        kernelsubstring = kernelstring.substr(brace_start + 1,brace_end - brace_start - 1);
        kernelsubstring = kernelsubstring.replace(/\ /g, "");
      
        var kernel_parameter = kernelsubstring.split(",");

        kernelstring = kernelstring.substr(brace_end);
        
        var parameter = new Array(kernel_parameter.length)
        for (var i = 0; i < kernel_parameter.length; i ++) {

          var value = 0;
          var string = kernel_parameter[i]
        
          // Adress space
          // __global, __local, __constant, __private. 
          if (string.indexOf("__local") >= 0 ) {
            value |= CL.address_space.LOCAL;
          }
          
          // Data Type
          // float, uchar, unsigned char, uint, unsigned int, int. 
          if (string.indexOf("float") >= 0 ) {
            value |= CL.data_type.FLOAT;
          } else if (string.indexOf("uchar") >= 0 ) {
            value |= CL.data_type.UINT;
          } else if (string.indexOf("unsigned char") >= 0 ) {
            value |= CL.data_type.UINT;
          } else if (string.indexOf("uint") >= 0 ) {
            value |= CL.data_type.UINT;
          } else if (string.indexOf("unsigned int") >= 0 ) {
            value |= CL.data_type.UINT;
          } else if (string.indexOf("int") >= 0 ) {
            value |= CL.data_type.INT;
          }
          
          parameter[i] = value;
        }
        
        kernel_struct[kernels_name] = parameter;
        
        kernel_start = kernelstring.indexOf("__kernel");
      }
      
#if OPENCL_DEBUG
      for (var name in kernel_struct) {
        console.info("Kernel NAME : " + name);      
        console.info("Kernel PARAMETER NUM : "+kernel_struct[name].length);
      }
#endif 
    
      return kernel_struct;
    },
    
    getDeviceName: function(type) {
      switch (type) {
        case 2 : return "CPU_DEVICE";
        case 4 : return "GPU_DEVICE";
        default : return "UNKNOW_DEVICE";
      }
    },
    
    getAllDevices: function(platform) {
      var res = [];
            
      if (platform >= CL.platforms.length || platform < 0 ) {
#if OPENCL_DEBUG
          console.error("getAllDevices: Invalid platform : "+plat);
#endif
          return res; 
      }

      if (CL.webcl_mozilla == 1) {
        res = CL.platforms[platform].getDeviceIDs(WebCL.CL_DEVICE_TYPE_ALL);
      } else {
        //res = CL.platforms[platform].getDevices(WebCL.DEVICE_TYPE_ALL);
        res = res.concat(CL.platforms[platform].getDevices(WebCL.DEVICE_TYPE_GPU));
        res = res.concat(CL.platforms[platform].getDevices(WebCL.DEVICE_TYPE_CPU));  
      }    

#if OPENCL_DEBUG
      console.info("CL.getAllDevices: : "+res.length);
#endif
  
      return res;
    },
                
    // Log and return the value error exception
    catchError: function(name,e) {
      var str=""+e;
      var n=str.lastIndexOf(" ");
      var error = str.substr(n+1,str.length-n-2);
#if OPENCL_DEBUG
      console.error("CATCH: "+name+": "+e);
#endif
      return error;
    },
  },
  
  clGetPlatformIDs: function(num_entries,platform_ids,num_platforms) {
    
    if (CL.checkWebCL() < 0) {
      console.error(CL.errorMessage);
      return -1;/*WEBCL_NOT_FOUND*/;
    }
        
    try { 
      
      // Get the platform
      var platforms = (CL.webcl_mozilla == 1) ? WebCL.getPlatformIDs() : WebCL.getPlatforms();
      
      // If platforms is not NULL, the num_entries must be greater than zero.
      // If both num_platforms and platforms are NULL.
      if ( (num_entries == 0 && platform_ids) != 0 || (num_platforms == 0 && platforms.length == 0) ) {
        return -30;/*CL_INVALID_VALUE*/
      }
            
      if (platform_ids == 0) {
        // If num_platforms is not null, we put the value inside
        {{{ makeSetValue('num_platforms', '0', 'platforms.length', 'i32') }}} /* Num of devices */;
                
        return 0;/*CL_SUCCESS*/
      } 
      
#if OPENCL_DEBUG
      console.info("clGetPlatformID: Platforms:");
#endif      
      for (var i = 0; i < platforms.length; i++) {
        // The number of OpenCL platforms returned is the mininum of the value specified by num_entries or the number of OpenCL platforms available.
        if (num_entries != 0 && i >= num_entries) break;
        
        CL.platforms.push(platforms[i]);
        
#if OPENCL_DEBUG
        var plat = platforms[i];
        var name = (CL.webcl_mozilla == 1) ? plat.getPlatformInfo (WebCL.CL_PLATFORM_NAME) : "Not Visible"/*plat.getInfo (WebCL.PLATFORM_NAME)*/;
        var vendor = (CL.webcl_mozilla == 1) ? plat.getPlatformInfo (WebCL.CL_PLATFORM_VENDOR) : "Not Visible"/*plat.getInfo (WebCL.PLATFORM_VENDOR)*/;
        var version = (CL.webcl_mozilla == 1) ? plat.getPlatformInfo (WebCL.CL_PLATFORM_VERSION) : plat.getInfo (WebCL.PLATFORM_VERSION);
        var extensions = (CL.webcl_mozilla == 1) ? plat.getPlatformInfo (WebCL.CL_PLATFORM_EXTENSIONS) : "Not Visible"/*plat.getInfo (WebCL.PLATFORM_EXTENSIONS)*/;
        var profile = (CL.webcl_mozilla == 1) ? plat.getPlatformInfo (WebCL.CL_PLATFORM_PROFILE) : plat.getInfo (WebCL.PLATFORM_PROFILE);
        console.info("\t"+i+": name: " + name);              
        console.info("\t"+i+": vendor: " + vendor);              
        console.info("\t"+i+": version: " + version);
        console.info("\t"+i+": profile: " + profile);
        console.info("\t"+i+": extensions: " + extensions);
#endif

      }

      // If num_platforms is not null, we put the value inside
      if (num_platforms != 0) {
        {{{ makeSetValue('num_platforms', '0', 'CL.platforms.length', 'i32') }}} /* Num of devices */;
      }
      
      // Add indices in array platforms (+1) for don't have platforms with id == 0
      for (var i = 0; i < CL.platforms.length; i++) {
        {{{ makeSetValue('platform_ids', 'i*4', 'i+1', 'i32') }}};
      }
      
      return 0;/*CL_SUCCESS*/
      
    } catch (e) {
      return CL.catchError("clGetPlatformID",e);
    }
  },

  clGetPlatformInfo: function(platform, param, param_value_size, param_value, param_value_size_ret) {
    var plat = platform - 1;
    if (plat >= CL.platforms.length || plat < 0 ) {
#if OPENCL_DEBUG
        console.error("clGetPlatformInfo: Invalid platform : "+plat);
#endif
        return -32; /* CL_INVALID_PLATFORM */ 
    }
    
    try { 
        
      var value;
          
      switch (param) {
        case(0x0900)/*CL_PLATFORM_PROFILE*/:
          value = CL.platforms[plat].getPlatformInfo(WebCL.CL_PLATFORM_PROFILE);  
          break;
        case(0x0901)/*CL_PLATFORM_VERSION*/:
          value = CL.platforms[plat].getPlatformInfo(WebCL.CL_PLATFORM_VERSION);  
          break;
        case(0x0902)/*CL_PLATFORM_NAME*/:
          value = CL.platforms[plat].getPlatformInfo(WebCL.CL_PLATFORM_NAME);  
          break;
        case(0x0903)/*CL_PLATFORM_VENDOR*/:
          value = CL.platforms[plat].getPlatformInfo(WebCL.CL_PLATFORM_VENDOR);  
          break;
        case(0x0904)/*CL_PLATFORM_EXTENSIONS*/:
          value = CL.platforms[plat].getPlatformInfo(WebCL.CL_PLATFORM_EXTENSIONS); 
          break;
        default:
#if OPENCL_DEBUG
          console.error("clGetPlatformInfo : Param not yet implemented or unknow : "+param);
#endif
          return -30; /* CL_INVALID_VALUE */           
      }
      
      if (param_value != 0) {
        writeStringToMemory(value, param_value);
      }
      
      {{{ makeSetValue('param_value_size_ret', '0', 'value.length', 'i32') }}};
            
      return 0; /*CL_SUCCESS*/
      
    } catch (e) {
      return CL.catchError("clGetPlatformInfo",e);
    }
  },

  clGetDeviceIDs: function(platform, device_type_i64_1, device_type_i64_2, num_entries, devices_ids, num_devices) {
    
    if (CL.checkWebCL() < 0) {
      console.error(CL.errorMessage);
      return -1;/*WEBCL_NOT_FOUND*/;
    }
    
    // Assume the device type is i32 
    assert(device_type_i64_2 == 0, 'Invalid flags i64');

    try { 

      // If platform is NULL, the behavior is implementation-defined
      if (platform == 0 && CL.platforms.length == 0) {
      
          // Get the platform
          var platforms = (CL.webcl_mozilla == 1) ? WebCL.getPlatformIDs() : WebCL.getPlatforms();
        
          if (platforms.length > 0) {
            CL.platforms.push(platforms[0]);
            
          } else {
#if OPENCL_DEBUG
            console.error("clGetDeviceIDs: Invalid platform : "+platform);
#endif
            return -32; /* CL_INVALID_PLATFORM */ 
          }      
      } else {              
        platform -= 1;
      }
      
      var alldev = CL.getAllDevices(platform);
      
      // If devices_ids is not NULL, the num_entries must be greater than zero.
      if ((num_entries == 0 && device_type_i64_1 == 0) || (alldev.length == 0 && device_type_i64_1 == 0)) {
#if OPENCL_DEBUG
        console.error("clGetDeviceIDs: Invalid value : "+num_entries);
#endif
        return -30;/*CL_INVALID_VALUE*/
      }
      
      if ( alldev.length > 0 && device_type_i64_1 == 0) {
#if OPENCL_DEBUG
        console.error("clGetDeviceIDs: Invalid device type : "+device_type_i64_1);
#endif
        return -31;/*CL_INVALID_DEVICE_TYPE*/
      }
      
      var map = {};
      var mapcount = 0;
    
      
      for (var i = 0 ; i < alldev.length; i++ ) {
        var type = (CL.webcl_mozilla == 1) ? alldev[i].getDeviceInfo(WebCL.CL_DEVICE_TYPE) : alldev[i].getInfo(WebCL.DEVICE_TYPE);

        if (type == device_type_i64_1 || device_type_i64_1 == -1) { 
           var name = (CL.webcl_mozilla == 1) ? alldev[i].getDeviceInfo(WebCL.CL_DEVICE_NAME) : CL.getDeviceName(type);
           map[name] = alldev[i];
           mapcount ++;
        }    
      }
            
      if (mapcount == 0) {
        var alldev = CL.getAllDevices(platform);
        for (var i = 0 ; i < alldev.length; i++) {
          var name = (CL.webcl_mozilla == 1) ? alldev[i].getDeviceInfo(WebCL.CL_DEVICE_NAME) : /*alldev[i].getInfo(WebCL.DEVICE_NAME) ;*/CL.getDeviceName(alldev[i].getInfo(WebCL.DEVICE_TYPE));
          map[name] = alldev[i];
          mapcount ++;
        }       
      }
      
      if (devices_ids == 0) {
        
        {{{ makeSetValue('num_devices', '0', 'mapcount', 'i32') }}} /* Num of devices */;
          
        return 0;/*CL_SUCCESS*/
      }
      
#if OPENCL_DEBUG
      console.info("clGetDeviceIDs: Devices:");
#endif
      for (var name in map) {
        CL.devices.push(map[name]);
#if OPENCL_DEBUG
        console.info("\t"+CL.devices.length-1+": name: " + name);
#endif
      }
    
      if (CL.devices.length == 0 ) {
        return -31;/*CL_INVALID_DEVICE_TYPE*/
      }

      if (num_entries > 0 && CL.devices.length > num_entries) {
        return -30;/*CL_INVALID_VALUE*/
      }

      // If devices is not null, we put the value inside
      if (num_devices != 0) {
        {{{ makeSetValue('num_devices', '0', 'CL.devices.length', 'i32') }}} /* Num of devices */;
      }

      // Add indices in array devices (+1) for don't have devices with id == 0
      for (var i = 0; i < CL.devices.length; i++) {
        {{{ makeSetValue('devices_ids', 'i*4', 'i+1', 'i32') }}};
      }
                    
      return 0;/*CL_SUCCESS*/
    } catch (e) {
      return CL.catchError("clGetDeviceIDs",e);
    }
  },
  
  clGetContextInfo: function(context, param_name, param_value_size, param_value, param_value_size_ret) {
    var ctx = context - 1;
    if (ctx >= CL.ctx.length || ctx < 0 ) {
#if OPENCL_DEBUG
        console.error("clGetContextInfo: Invalid context : "+ctx);
#endif
        return -34; /* CL_INVALID_CONTEXT */ 
    }
  
    try {
      var res;
      var size;
            
      switch (param_name) {
        case (0x1081) /* CL_CONTEXT_DEVICES */:
          res = (CL.webcl_mozilla == 1) ? CL.ctx[ctx].getContextInfo(WebCL.CL_CONTEXT_DEVICES) : CL.ctx[ctx].getInfo(WebCL.CONTEXT_DEVICES) ;

          // Must verify if size of device is same as param_value°size
          if (param_value != 0) {
            for (var i = 0 ; i < res.length; i++) {
              CL.devices.push(res[i]);
              {{{ makeSetValue('param_value', 'i*4', 'CL.devices.length', 'i32') }}};
            }
          }
          size = res.length * 4;
          break;
        case (0x1082) /* CL_CONTEXT_PROPERTIES */:
          res = (CL.webcl_mozilla == 1) ? CL.ctx[ctx].getContextInfo(WebCL.CL_CONTEXT_PROPERTIES) : CL.ctx[ctx].getInfo(WebCL.CONTEXT_PROPERTIES) ;
          // \todo add in param_value the properties list
          size = res.length * 4;          
          break;
        case (0x1080) /* CL_CONTEXT_REFERENCE_COUNT */:
          res = CL.ctx[ctx].getContextInfo(WebCL.CL_CONTEXT_REFERENCE_COUNT); // return cl_uint
          size = 1;
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          break;
        default:
#if OPENCL_DEBUG
          console.error("clGetContextInfo : Param not yet implemented or unknow : "+param_name);
#endif
          return -30; /* CL_INVALID_VALUE */ 
      };
      
      if (param_value_size < size && param_value != 0) {
#if OPENCL_DEBUG
        console.error("clGetContextInfo : Size of param_value "+pvs+" is less than size compute "+size);
#endif
        return -30; /* CL_INVALID_VALUE */              
      }
      
      {{{ makeSetValue('param_value_size_ret', '0', 'size', 'i32') }}};
                      
      return 0;/*CL_SUCCESS*/
    } catch (e) {
      return CL.catchError("clGetContextInfo",e);
    }    
  },
  
  clGetDeviceInfo: function(device, param_name, param_value_size, param_value, param_value_size_ret) {
    var idx = device - 1;

    if (idx >= CL.devices.length || idx < 0 ) {
#if OPENCL_DEBUG
      console.error("clGetDeviceInfo: Invalid device : "+idx);
#endif
      return -33; /* CL_INVALID_DEVICE */  
    }    

    var res;
    var size = 0;
    
    var info = CL.device_infos[param_name];
    if (info != undefined) {
      // Return string
      if (
        (param_name == 0x1030) || /* CL_DEVICE_EXTENSIONS */
        (param_name == 0x102B) || /* CL_DEVICE_NAME       */
        (param_name == 0x102C) || /* CL_DEVICE_VENDOR     */
        (param_name == 0x102D) || /* CL_DRIVER_VERSION    */
        (param_name == 0x102F) || /* CL_DEVICE_VERSION    */
        (param_name == 0x102E)    /* CL_DEVICE_PROFILE    */
      ) {
        try {
          res = (CL.webcl_mozilla == 1) ? CL.devices[idx].getDeviceInfo(info[0]) : CL.devices[idx].getInfo(info[1]);
        } catch (e) {
          CL.catchError("clGetContextInfo",e);
          res = "Not Visible";
        }    
 
        writeStringToMemory(res, param_value);
        size = res.length;
      } 
      // Return int
      else {
        try {
          res = (CL.webcl_mozilla == 1) ? CL.devices[idx].getDeviceInfo(info[0]) : CL.devices[idx].getInfo(info[1]);
        } catch (e) {
          CL.catchError("clGetContextInfo",e);
          res = O;
        }   
        
        {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
        size = 1;
      }
    } else {
#if OPENCL_DEBUG
      console.error("clGetContextInfo: Unknow param info : "+param_name);
#endif
      {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
      size = 1;
    }
    
    {{{ makeSetValue('param_value_size_ret', '0', 'size', 'i32') }}};
              
    return 0;/*CL_SUCCESS*/  
  },

  clCreateContext: function(properties, num_devices, devices, pfn_notify, user_data, errcode_ret) {
    if (CL.platforms.length == 0) {
#if OPENCL_DEBUG
      console.error("clCreateContext: Invalid platform");
#endif
      {{{ makeSetValue('errcode_ret', '0', '-32', 'i32') }}} /* CL_INVALID_PLATFORM */;
      return 0; // Null pointer    
    }

    try {
      var prop = [];
      
      if (properties != 0) {
        var i = 0;
        while(1) {
          var readprop = {{{ makeGetValue('properties', 'i*4', 'i32') }}};
          if (readprop == 0) break;
          
          switch(readprop) {
            case (4228) /*CL_CONTEXT_PLATFORM*/ :
              // property platform
              prop.push(WebCL.CL_CONTEXT_PLATFORM);
              i++;
              // get platform id
              readprop = {{{ makeGetValue('properties', 'i*4', 'i32') }}} - 1;
              if (readprop >= CL.platforms.length || readprop < 0 ) {
#if OPENCL_DEBUG
                console.error("clCreateContext: Invalid context : "+ctx);
#endif
                {{{ makeSetValue('errcode_ret', '0', '-32', 'i32') }}} /* CL_INVALID_PLATFORM */;
                return 0; // Null pointer    
              } else {
                prop.push(CL.platforms[readprop]);
              }             
            break;
            default:
#if OPENCL_DEBUG
              console.error("clCreateContext : Param not yet implemented or unknow : "+param_name);
#endif
              {{{ makeSetValue('errcode_ret', '0', '-30', 'i32') }}} /* CL_INVALID_VALUE */;
              return 0; // Null pointer    

          }
          i++;  
        }        
      }
      
      if (prop.length == 0) {
        prop = [WebCL.CL_CONTEXT_PLATFORM, CL.platforms[0]];     
      }

      if (num_devices > CL.devices.length || CL.devices.length == 0) {
#if OPENCL_DEBUG
        console.error("clCreateContext: Invalid num devices : "+num_devices);
#endif
        {{{ makeSetValue('errcode_ret', '0', '-33', 'i32') }}} /* CL_INVALID_DEVICE */;  
        return 0;
      }

      // \todo will be better to use the devices list in parameter ...
      var devices_tab = [];
      for (var i = 0; i < num_devices; i++) {
        devices_tab[i] = CL.devices[i];
      } 
    
      // Use default platform
      if (CL.webcl_mozilla == 1) {
        CL.ctx.push(WebCL.createContext(prop, devices_tab/*[CL.devices[0],CL.devices[1]]*/));  
      } else {
        CL.ctx.push(WebCL.createContext({platform: prop[1], devices: devices_tab, deviceType: devices_tab[0].getInfo(WebCL.DEVICE_TYPE), shareGroup: 1, hint: null}));
      }
      

      // Return the pos of the context +1
      return CL.ctx.length;
    } catch (e) {    
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateContext",e)', 'i32') }}};
      return 0; // Null pointer    
    }
  },

  clCreateContextFromType: function(properties, device_type_i64_1, device_type_i64_2, pfn_notify, private_info, cb, user_data, user_data, errcode_ret) {
    
    if (CL.platforms.length == 0) {
#if OPENCL_DEBUG
      console.error("clCreateContextFromType: Invalid platform");
#endif
      {{{ makeSetValue('errcode_ret', '0', '-32', 'i32') }}} /* CL_INVALID_PLATFORM */;
      return 0; // Null pointer    
    }
    
    // Assume the device type is i32 
    assert(device_type_i64_2 == 0, 'Invalid flags i64');
    
    try {
      
      var prop = [];
      var plat = -1;
      
      if (properties != 0) {
        var i = 0;
        while(1) {
          var readprop = {{{ makeGetValue('properties', 'i*4', 'i32') }}};
          if (readprop == 0) break;
          
          switch(readprop) {
            case (4228) /*CL_CONTEXT_PLATFORM*/ :
              // property platform
              prop.push(WebCL.CL_CONTEXT_PLATFORM);
              i++;
              // get platform id
              readprop = {{{ makeGetValue('properties', 'i*4', 'i32') }}} - 1;
              if (readprop >= CL.platforms.length || readprop < 0 ) {
#if OPENCL_DEBUG
                console.error("clCreateContextFromType: Invalid context : "+ctx);
#endif
                {{{ makeSetValue('errcode_ret', '0', '-32', 'i32') }}} /* CL_INVALID_PLATFORM */;
                return 0; // Null pointer    
              } else {
                plat = readprop;
                prop.push(CL.platforms[readprop]);
              }             
            break;
            default:
#if OPENCL_DEBUG
              console.error("clCreateContextFromType : Param not yet implemented or unknow : "+param_name);
#endif
              {{{ makeSetValue('errcode_ret', '0', '-30', 'i32') }}} /* CL_INVALID_VALUE */;
              return 0; // Null pointer    

          }
          i++;  
        }        
      }
      
      if (prop.length == 0) {
        prop = [WebCL.CL_CONTEXT_PLATFORM, CL.platforms[0]];
        plat = 0;   
      }
          
      // \todo en faire une function si le device n'existe pas
      var alldev = CL.getAllDevices(plat);
      var mapcount = 0;
    
      for (var i = 0 ; i < alldev.length; i++ ) {
        var type = (CL.webcl_mozilla == 1) ? alldev[i].getDeviceInfo(WebCL.CL_DEVICE_TYPE) : alldev[i].getInfo(WebCL.DEVICE_TYPE);
        if (type == device_type_i64_1 || device_type_i64_1 == -1) {
           mapcount ++;
        }        
      }

      if (CL.webcl_mozilla == 1) {
        if (mapcount >= 1) {        
          CL.ctx.push(WebCL.createContextFromType(prop, device_type_i64_1));
        } else {
          // Use default platform
          CL.ctx.push(WebCL.createContextFromType(prop, WebCL.CL_DEVICE_TYPE_DEFAULT));
        }
      } else {
        if (mapcount >= 1) {
          var contextProperties = {platform: null, devices: null, deviceType: device_type_i64_1, shareGroup: 0, hint: null};
          CL.ctx.push(WebCL.createContext(contextProperties));
        } else {
          CL.ctx.push(WebCL.createContext());
        }
      }
    
      // Return the pos of the context +1
      return CL.ctx.length;
      
    } catch (e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateContextFromType",e)', 'i32') }}};
      return 0; // Null pointer    
    }
  },
  
  clCreateCommandQueue: function(context, devices, properties, errcode_ret) {
    var ctx = context - 1;
    if (ctx >= CL.ctx.length || ctx < 0 ) {
#if OPENCL_DEBUG
        console.error("clCreateCommandQueue: Invalid context : "+ctx);
#endif
        {{{ makeSetValue('errcode_ret', '0', '-34', 'i32') }}} /* CL_INVALID_CONTEXT */;
        return 0; // Null pointer    
    }
      
    try {
          
      var idx = devices;//{{{ makeGetValue('devices', '0', 'i32') }}};
    
      if (idx == 0) {
        // Create a command-queue on the first device available if idx == 0
        console.error("\\todo clCreateCommandQueue() : idx = 0 : Need work on that ")
        var devices = CL.getAllDevices(0);
        CL.devices.push(devices[0]);
      }
      
      idx = idx - 1;
    
      if (idx >= CL.devices.length || idx < 0 ) {
#if OPENCL_DEBUG
        console.error("clCreateCommandQueue: Invalid device : "+idx);
#endif
        {{{ makeSetValue('errcode_ret', '0', '-33', 'i32') }}} /* CL_INVALID_DEVICE */;  
        return 0; // Null pointer    
      }

      // \todo set the properties 
      CL.cmdQueue.push(CL.ctx[ctx].createCommandQueue(CL.devices[idx], 0));

      // Return the pos of the queue +1
      return CL.cmdQueue.length;
    } catch (e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateCommandQueue",e)', 'i32') }}};
      return 0; // Null pointer    
    }
  },

  clCreateProgramWithSource: function(context, count, strings, lengths, errcode_ret) {
    var ctx = context - 1;
    if (ctx >= CL.ctx.length || ctx < 0 ) {
#if OPENCL_DEBUG
      console.error("clCreateProgramWithSource: Invalid context : "+ctx);
#endif
      {{{ makeSetValue('errcode_ret', '0', '-34', 'i32') }}} /* CL_INVALID_CONTEXT */;
      return 0; // Null pointer    
    }

    var sourceIdx = {{{ makeGetValue('strings', '0', 'i32') }}}
    var kernel = Pointer_stringify(sourceIdx); 
  
    CL.kernels_sig = CL.parseKernel(kernel);
  
    try {
      // \todo set the properties 
      if (CL.webcl_mozilla == 1) {
        CL.programs.push(CL.ctx[ctx].createProgramWithSource(kernel));
      } else {
        CL.programs.push(CL.ctx[ctx].createProgram(kernel));
      }
      
      // Return the pos of the queue +1
      return CL.programs.length;
    } catch (e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateProgramWithSource",e)', 'i32') }}};
      return 0; // Null pointer
    }
  },
  
  clCreateProgramWithBinary: function(context,num_devices,device_list,lengths,binaries,binary_status,errcode_ret) {
    // Can't create program with binary
    console.warning("Can't create program with binary");
    return 0; // Null pointer    
  },
  
  clBuildProgram: function(program, num_devices, device_list, options, pfn_notify, user_data) {
    var prog = program - 1;
    if (prog >= CL.programs.length || prog < 0 ) {
#if OPENCL_DEBUG
      console.error("clBuildProgram: Invalid program : "+prog);
#endif
      return -44; /* CL_INVALID_PROGRAM */
    }
    
    try {
      if (num_devices > CL.devices.length || CL.devices.length == 0) {
#if OPENCL_DEBUG
        console.error("clBuildProgram: Invalid num devices : "+num_devices);
#endif
        return -33; /* CL_INVALID_DEVICE */;  
      }

      var devices_tab = [];

      if (num_devices == 0 || device_list == 0) {
        devices_tab[0] = CL.devices[0];
      } else {
        for (var i = 0; i < num_devices; i++) {
          var idx = {{{ makeGetValue('device_list', 'i*4', 'i32') }}} - 1;
          devices_tab[i] = CL.devices[idx];
        }
      }    

      var opt = "";//Pointer_stringify(options);

      if (CL.webcl_mozilla == 1) {
        CL.programs[prog].buildProgram (devices_tab, opt);
      } else { 
        CL.programs[prog].build(devices_tab, opt);
      }
      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clBuildProgram",e);
    }
  },

  clGetProgramBuildInfo: function(program, device, param_name, param_value_size, param_value, param_value_size_ret) {
    var prog = program - 1;
    if (prog >= CL.programs.length || prog < 0 ) {
#if OPENCL_DEBUG
      console.error("clGetProgramBuildInfo: Invalid program : "+prog);
#endif
      return -44; /* CL_INVALID_PROGRAM */
    }          

    // \todo the type is a number but why i except have a Array ??? Will must be an array ???
    // var idx = {{{ makeGetValue('device', '0', 'i32') }}} - 1;
    var idx = device - 1;
    
    if (idx >= CL.devices.length || idx < 0 ) {
#if OPENCL_DEBUG
      console.error("clGetProgramBuildInfo: Invalid device : "+idx);
#endif
      return -33; /* CL_INVALID_DEVICE */  
    }

    try {
      var res = "";
      switch (param_name) {
        case 0x1181 /*CL_PROGRAM_BUILD_STATUS*/:
        res = CL.programs[prog].getProgramBuildInfo (CL.devices[idx], WebCL.CL_PROGRAM_BUILD_STATUS);
        break;
      case 0x1182 /*CL_PROGRAM_BUILD_OPTIONS*/:
        res = CL.programs[prog].getProgramBuildInfo (CL.devices[idx], WebCL.CL_PROGRAM_BUILD_OPTIONS);
        break;
      case 0x1183 /*CL_PROGRAM_BUILD_LOG*/:
        res = CL.programs[prog].getProgramBuildInfo (CL.devices[idx], WebCL.CL_PROGRAM_BUILD_LOG);
        break;
      };

      {{{ makeSetValue('param_value_size_ret', '0', 'res.length', 'i32') }}};
      writeStringToMemory(res,param_value);

      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clGetProgramBuildInfo",e);
    }
  },
  
  clGetProgramInfo: function(program, param_name, param_value_size, param_value, param_value_size_ret) {
    var prog = program - 1;
    if (prog >= CL.programs.length || prog < 0 ) {
#if OPENCL_DEBUG
      console.error("clGetProgramInfo: Invalid program : "+prog);
#endif
      return -44; /* CL_INVALID_PROGRAM */
    }           

    try {
      switch (param_name) {
        case 0x1160 /*CL_PROGRAM_REFERENCE_COUNT*/:
          var res = CL.programs[prog].getProgramInfo (WebCL.CL_PROGRAM_REFERENCE_COUNT); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          {{{ makeSetValue('param_value_size_ret', '0', '1', 'i32') }}};
          break;
        case 0x1162 /*CL_PROGRAM_NUM_DEVICES*/:
          var res = CL.programs[prog].getProgramInfo (WebCL.CL_PROGRAM_NUM_DEVICES);
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          {{{ makeSetValue('param_value_size_ret', '0', '1', 'i32') }}};
          break;    
        case 0x1164 /*CL_PROGRAM_SOURCE*/:
          res = CL.programs[prog].getProgramInfo (WebCL.CL_PROGRAM_SOURCE);
          {{{ makeSetValue('param_value_size_ret', '0', 'res.length', 'i32') }}};
          writeStringToMemory(res,param_value);
          break;
        // case 0x1165 /*CL_PROGRAM_BINARY_SIZES*/:
        //   res = CL.programs[prog].getProgramInfo (WebCL.CL_PROGRAM_BINARY_SIZES);
        //   break;
        case 0x1163 /*CL_PROGRAM_DEVICES*/:
          res = CL.programs[prog].getProgramInfo (WebCL.CL_PROGRAM_DEVICES);
          {{{ makeSetValue('param_value_size_ret', '0', 'res.length', 'i32') }}};
          for (var i = 0; i <res.length; i++) {
            CL.devices.push(res[i]);
            {{{ makeSetValue('param_value', 'i*4', 'CL.devices.length', 'i32') }}};
          }
          
          break;
        
        default:
#if OPENCL_DEBUG
          console.error("clGetProgramInfo : Param not yet implemented or unknow : "+param_name);
#endif
          return -30; /* CL_INVALID_VALUE */ 
      };

      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clGetProgramInfo",e);
    }
  },

  clCreateKernel: function(program, kernels_name, errcode_ret) {
    var prog = program - 1;
    if (prog >= CL.programs.length || prog < 0 ) {
#if OPENCL_DEBUG
      console.error("clCreateKernel: Invalid program : "+prog);
#endif
      {{{ makeSetValue('errcode_ret', '0', '-44', 'i32') }}};
      return 0; // Null pointer   
    }           

    try {

      var name = Pointer_stringify(kernels_name);
      CL.kernels.push(CL.programs[prog].createKernel(name));

      // Add the name of the kernel for search the kernel sig after...
      CL.kernels_name.push(name);
      
#if OPENCL_DEBUG
      console.info("Kernel '"+name+"', has "+CL.kernels_sig[name]+" parameters !!!!");
#endif

      // Return the pos of the queue +1
      return CL.kernels.length;
    } catch (e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateKernel",e)', 'i32') }}};
      return 0; // Null pointer    
    }
  },

  clCreateImage2D: function (context, flags_i64_1, flags_i64_2, image_format, image_width, image_height, image_row_pitch, host_ptr, errcode_ret) {
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');

    var ctx = context - 1;
    if (ctx >= CL.ctx.length || ctx < 0 ) {
#if OPENCL_DEBUG
      console.error("clCreateImage2D: Invalid context : "+ctx);
#endif
      {{{ makeSetValue('errcode_ret', '0', '-34', 'i32') }}} /* CL_INVALID_CONTEXT */;
      return 0; // Null pointer    
    }
    
    try {
      switch (flags_i64_1) {
          
        default:
#if OPENCL_DEBUG
          console.error("clCreateImage2D: flag not yet implemented "+flags_i64_1);
#endif
          {{{ makeSetValue('errcode_ret', '0', '-30', 'i32') }}} /* CL_INVALID_VALUE */;
          return 0;
      };

      {{{ makeSetValue('errcode_ret', '0', '0', 'i32') }}} /* CL_SUCCESS */;

      return CL.buffers.length;
    } catch(e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateImage2D",e)', 'i32') }}};
      return 0;
    }
    
  },
  
  
  clCreateBuffer: function(context, flags_i64_1, flags_i64_2, size, host_ptr, errcode_ret) {
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');

    var ctx = context - 1;
    if (ctx >= CL.ctx.length || ctx < 0 ) {
#if OPENCL_DEBUG
      console.error("clCreateBuffer: Invalid context : "+ctx);
#endif
      {{{ makeSetValue('errcode_ret', '0', '-34', 'i32') }}} /* CL_INVALID_CONTEXT */;
      return 0; // Null pointer    
    }

    try {
      switch (flags_i64_1) {
        case (1 << 0) /* CL_MEM_READ_WRITE */:
          var macro = (CL.webcl_mozilla == 1) ? WebCL.CL_MEM_READ_WRITE : WebCL.MEM_READ_WRITE;
          CL.buffers.push(CL.ctx[ctx].createBuffer(macro,size));
          break;
        case (1 << 1) /* CL_MEM_WRITE_ONLY */:
          var macro = (CL.webcl_mozilla == 1) ? WebCL.CL_MEM_WRITE_ONLY : WebCL.MEM_WRITE_ONLY;
          CL.buffers.push(CL.ctx[ctx].createBuffer(macro,size));
          break;
        case (1 << 2) /* CL_MEM_READ_ONLY */:
          var macro = (CL.webcl_mozilla == 1) ? WebCL.CL_MEM_READ_ONLY : WebCL.MEM_READ_ONLY;
          CL.buffers.push(CL.ctx[ctx].createBuffer(macro,size));
          break;
        case (((1 << 2)|(1 << 5))) /* CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR */:
          if (host_ptr == 0) {
#if OPENCL_DEBUG
            console.error("clCreateBuffer: CL_MEM_COPY_HOST_PTR can't be use with null host_ptr parameter");
#endif
            {{{ makeSetValue('errcode_ret', '0', '-37', 'i32') }}} /* CL_INVALID_HOST_PTR */;
            return 0;     
          }
          
          var vector;
          var isFloat = 0;
          var isUint = 0;
          var isInt = 0;
          var buff = CL.buffers.length;
          
          if (CL.kernels_name.length > 0) {
            // \warning experimental stuff
          
            console.info("/!\\ clCreateBuffer: Need to find how detect the array type");
            var name = CL.kernels_name[0];
            console.info("/!\\ clCreateBuffer: use '"+name+"' kernel name ...");
            var sig = CL.kernels_sig[name];
            var type = sig[buff];
    
            if (type & CL.data_type.FLOAT) {
              isFloat = 1;
            } 
            if (type & CL.data_type.UINT) {
              isUint = 1;
            } 
            if (type & CL.data_type.INT) {
              isInt = 1;
            }
          }

          if (CL.webcl_webkit == -1) {
            vector = new ArrayBuffer(size / ArrayBuffer.BYTES_PER_ELEMENT);
          } else {
            if ( isFloat == 0 && isUint == 0 && isInt == 0 ) {
              isFloat = CL.isFloat(host_ptr,size); 
              if (isFloat) {
                vector = new Float32Array(size / Float32Array.BYTES_PER_ELEMENT);
              } else {
                vector = new Int32Array(size / Int32Array.BYTES_PER_ELEMENT);
              }
            } else {        
              if (isFloat) {
                vector = new Float32Array(size / Float32Array.BYTES_PER_ELEMENT);
              } else if (isUint) {
                vector = new Uint32Array(size / Uint32Array.BYTES_PER_ELEMENT);
              } else if (isInt) {
                vector = new Int32Array(size / Int32Array.BYTES_PER_ELEMENT);
              } else {
#if OPENCL_DEBUG
                console.error("clCreateBuffer: Unknow ouptut type : "+sig[buff]);
#endif
              }
            }
          }
          
#if OPENCL_DEBUG          
          if (isFloat) {
            console.info("/!\\ clCreateBuffer: use FLOAT output type ...");
          } else if (isUint) {
            console.info("/!\\ clCreateBuffer: use UINT output type ...");
          } else if (isInt) {
            console.info("/!\\ clCreateBuffer: use INT output type ...");
          } 
#endif
          
          for (var i = 0; i < (size / 4); i++) {
            if (isFloat) {
              vector[i] = {{{ makeGetValue('host_ptr', 'i*4', 'float') }}};
            } else {
              vector[i] = {{{ makeGetValue('host_ptr', 'i*4', 'i32') }}};
            }
          }
        
#if OPENCL_DEBUG         
          console.info(vector);
#endif
          
          if (CL.webcl_webkit == -1) {
              CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.MEM_READ_ONLY | WebCL.MEM_COPY_HOST_PTR, size, vector));
          } else {
            if (CL.webcl_webkit == 1) {
              CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.MEM_READ_ONLY,size));
            } else{
              CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.CL_MEM_READ_ONLY,size));              
            }
            
            if (CL.cmdQueue.length == 0) {
#if OPENCL_DEBUG
              console.error("clCreateBuffer: Invalid command queue : "+CL.cmdQueue.length);
#endif
              {{{ makeSetValue('errcode_ret', '0', '-36', 'i32') }}} /* CL_INVALID_COMMAND_QUEUE */;
              return 0;
            }

            if (CL.buffers.length == 0) {
#if OPENCL_DEBUG
              console.error("clCreateBuffer: Invalid buffers : "+CL.buffers.length);
#endif
              {{{ makeSetValue('errcode_ret', '0', '-38', 'i32') }}} /* CL_INVALID_MEM_OBJECT */;
              return 0;
            }    
          
            CL.cmdQueue[CL.cmdQueue.length-1].enqueueWriteBuffer(CL.buffers[CL.buffers.length-1], 1, 0, size, vector , []);    
          }
 
          break;
        default:
#if OPENCL_DEBUG
          console.error("clCreateBuffer: flag not yet implemented "+flags_i64_1);
#endif
          {{{ makeSetValue('errcode_ret', '0', '-30', 'i32') }}} /* CL_INVALID_VALUE */;
          return 0;
      };

      {{{ makeSetValue('errcode_ret', '0', '0', 'i32') }}} /* CL_SUCCESS */;

      return CL.buffers.length;
    } catch(e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateBuffer",e)', 'i32') }}};
      return 0;
    }
  },

  clCreateSubBuffer: function (buffer, flags_i64_1, flags_i64_2, buffer_create_type, buffer_create_info, errcode_ret) {
    // Assume the flags is i32 
    assert(flags_i64_2 == 0, 'Invalid flags i64');

    var buff = buffer - 1;
    if (buff >= CL.buffers.length || buff < 0 ) {
#if OPENCL_DEBUG
      console.error("clCreateSubBuffer: Invalid buffer : "+buff);
#endif
      {{{ makeSetValue('errcode_ret', '0', '-38', 'i32') }}} /* CL_INVALID_MEM_OBJECT */;
      return 0; // Null pointer    
    }

    try {
      
      var aBufferRegion = {};
      if (buffer_create_type == 0x1220 /*CL_BUFFER_CREATE_TYPE_REGION*/) {
        var o = {{{ makeGetValue('buffer_create_info', '0', 'i32') }}};
        var s = {{{ makeGetValue('buffer_create_info', '4', 'i32') }}};
        aBufferRegion={origin:o,size:s};
      }      

      switch (flags_i64_1) {
        case (1 << 0) /* CL_MEM_READ_WRITE */:
          CL.buffers.push(CL.buffers[buff].createSubBuffer(WebCL.CL_MEM_READ_WRITE,aBufferRegion));
          break;
        case (1 << 1) /* CL_MEM_WRITE_ONLY */:
          CL.buffers.push(CL.buffers[buff].createSubBuffer(WebCL.CL_MEM_WRITE_ONLY,aBufferRegion));
          break;
        case (1 << 2) /* CL_MEM_READ_ONLY */:
          CL.buffers.push(CL.buffers[buff].createSubBuffer(WebCL.CL_MEM_READ_ONLY,aBufferRegion));
          break;
        case (1 << 3) /* CL_MEM_USE_HOST_PTR */:
          CL.buffers.push(CL.buffers[buff].createSubBuffer(WebCL.CL_MEM_USE_HOST_PTR,aBufferRegion));
          break;
        case (1 << 4) /* CL_MEM_ALLOC_HOST_PTR */:
          CL.buffers.push(CL.buffers[buff].createSubBuffer(WebCL.CL_MEM_ALLOC_HOST_PTR,aBufferRegion));
          break;
        case (1 << 5) /* CL_MEM_COPY_HOST_PTR */:
          CL.buffers.push(CL.buffers[buff].createSubBuffer(WebCL.CL_MEM_COPY_HOST_PTR,aBufferRegion));
          break;
        default:
#if OPENCL_DEBUG
          console.error("clCreateSubBuffer: flag not yet implemented "+flags_i64_1);
#endif
          {{{ makeSetValue('errcode_ret', '0', '-30', 'i32') }}} /* CL_INVALID_VALUE */;
          return 0;
      };
      
      {{{ makeSetValue('errcode_ret', '0', '0', 'i32') }}} /* CL_SUCCESS */;

      return CL.buffers.length;
      
    } catch(e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateSubBuffer",e)', 'i32') }}};
      return 0;
    }
  },
     
  clEnqueueWriteBuffer: function(command_queue, buffer, blocking_write, offset, size, ptr, num_events_in_wait_list, event_wait_list, event) {
    var queue = command_queue - 1;
    if (queue >= CL.cmdQueue.length || queue < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueWriteBuffer: Invalid command queue : "+queue);
#endif

      return -36; /* CL_INVALID_COMMAND_QUEUE */
    }

    var buff = buffer - 1;
    if (buff >= CL.buffers.length || buff < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueWriteBuffer: Invalid command queue : "+buff);
#endif

      return -38; /* CL_INVALID_MEM_OBJECT */
    }
    
    var vector;
    var isFloat = 0;
    var isUint = 0;
    var isInt = 0;

    if (CL.kernels_name.length > 0) {
      // \warning experimental stuff
    
      console.info("/!\\ clEnqueueWriteBuffer: Need to find how detect the array type");
      var name = CL.kernels_name[0];
      console.info("/!\\ clEnqueueWriteBuffer: use '"+name+"' kernel name ...");
      var sig = CL.kernels_sig[name];
      var type = sig[buff];

      if (type & CL.data_type.FLOAT) {
        isFloat = 1;
      } 
      if (type & CL.data_type.UINT) {
        isUint = 1;
      } 
      if (type & CL.data_type.INT) {
        isInt = 1;
      }
    }

    if ( isFloat == 0 && isUint == 0 && isInt == 0 ) {
      isFloat = CL.isFloat(ptr,size); 
      if (isFloat) {
        vector = new Float32Array(size / Float32Array.BYTES_PER_ELEMENT);
        console.info("/!\\ clEnqueueWriteBuffer: use FLOAT output type ...");
      } else {
        vector = new Int32Array(size / Int32Array.BYTES_PER_ELEMENT);
        console.info("/!\\ clEnqueueWriteBuffer: use INT output type ...");        
      }
    } else {        
      if (isFloat) {
        vector = new Float32Array(size / Float32Array.BYTES_PER_ELEMENT);
        console.info("/!\\ clEnqueueWriteBuffer: use FLOAT output type ...");
      } else if (isUint) {
        vector = new Uint32Array(size / Uint32Array.BYTES_PER_ELEMENT);
        console.info("/!\\ clEnqueueWriteBuffer: use UINT output type ...");
      } else if (isInt) {
        vector = new Int32Array(size / Int32Array.BYTES_PER_ELEMENT);
        console.info("/!\\ clEnqueueWriteBuffer: use INT output type ...");
      } else {
#if OPENCL_DEBUG
        console.error("clEnqueueWriteBuffer: Unknow ouptut type : "+sig[buff]);
#endif
      }
    }

    for (var i = 0; i < (size / 4); i++) {
      if (isFloat) {
        vector[i] = {{{ makeGetValue('ptr', 'i*4', 'float') }}};
      } else {
        vector[i] = {{{ makeGetValue('ptr', 'i*4', 'i32') }}};
      }
    }
 
#if OPENCL_DEBUG         
    console.info(vector);
#endif
    
    try {
      CL.cmdQueue[queue].enqueueWriteBuffer (CL.buffers[buff], blocking_write, offset, size, vector , []);

      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clEnqueueWriteBuffer",e);
    }
  },
  
  clEnqueueMapBuffer: function(command_queue, buffer, blocking_map, map_flags_1, map_flags_2, offset, size, num_events_in_wait_list, event_wait_list, event, errcode_ret) {
    // Assume the flags is i32 
    assert(map_flags_2 == 0, 'Invalid flags i64');
    
    var queue = command_queue - 1;
    if (queue >= CL.cmdQueue.length || queue < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueMapBuffer: Invalid command queue : "+queue);
#endif
      {{{ makeSetValue('errcode_ret', '0', '-36', 'i32') }}} /* CL_INVALID_COMMAND_QUEUE */;
      return 0;
    }

    var buff = buffer - 1;
    if (buff >= CL.buffers.length || buff < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueMapBuffer: Invalid buffer mem : "+buff);
#endif
      {{{ makeSetValue('errcode_ret', '0', '-38', 'i32') }}} /* CL_INVALID_MEM_OBJECT */;
      return 0;
    }

    try {
         
      var adata = new Array();
         
      switch (map_flags_1) {
        case(1<<0):
          CL.cmdQueue[queue].enqueueMapBuffer	 (CL.buffers[buff], blocking_map, WebCL.CL_MAP_WRITE, offset, size, [], adata);
          break;        
        case(1<<1):
          CL.cmdQueue[queue].enqueueMapBuffer	 (CL.buffers[buff], blocking_map, WebCL.CL_MAP_WRITE, offset, size, [], adata);
          break;        
        default:
#if OPENCL_DEBUG
          console.error("clEnqueueMapBuffer: flag not yet implemented "+map_flags_1);
#endif
          {{{ makeSetValue('errcode_ret', '0', '-30', 'i32') }}} /* CL_INVALID_VALUE */;
          return 0;
      }
                
      {{{ makeSetValue('errcode_ret', '0', '0', 'i32') }}} /* CL_SUCCESS */;

      return adata;
      
    } catch(e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clEnqueueMapBuffer",e)', 'i32') }}};
      return 0;
    }
  },
  
  clEnqueueMarker: function(command_queue,event) {

    var queue = command_queue - 1;
    if (queue >= CL.cmdQueue.length || queue < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueMarker: Invalid command queue : "+queue);
#endif
      {{{ makeSetValue('errcode_ret', '0', '-36', 'i32') }}} /* CL_INVALID_COMMAND_QUEUE */;
      return 0;
    }
    
    try {
      
      CL.cmdQueue[queue].enqueueMarker();

      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clEnqueueMarker",e);
    }
  },

  clSetKernelArg: function(kernel, arg_index, arg_size, arg_value) {
    var ker = kernel - 1;
    if (ker >= CL.kernels.length || ker < 0 ) {
#if OPENCL_DEBUG
      console.error("clSetKernelArg: Invalid kernel : "+ker);
#endif

      return -48; /* CL_INVALID_KERNEL */
    }
                        
    try {  
                
      var name = CL.kernels_name[ker];
            
      // \todo problem what is arg_value is buffer or just value ??? hard to say ....
      // \todo i suppose the arg_index correspond with the order of the buffer creation if is 
      // not inside the buffers array size we take the value
      
      if (CL.kernels_sig[name].length <= 0 && arg_index > CL.kernels_sig[name].length) {
#if OPENCL_DEBUG
        console.error("clSetKernelArg: Invalid signature : "+CL.kernels_sig[name].length);
#endif
        return -1; /* CL_FAILED */
      }
      
      var sig = CL.kernels_sig[name];
      var type = sig[arg_index];
      
      // \todo this syntax give a very bad crash ... why ??? (type & CL.data_type.FLOAT) ? 1 : 0;
      var isFloat = 0;
      var isLocal = 0;    
      
      if (type&CL.data_type.FLOAT) {
        isFloat = 1;
      } 
      
      if (type&CL.address_space.LOCAL) {
        isLocal = 1;
      }
      
      var value;
      if (isLocal) {
#if OPENCL_DEBUG
        console.info("clSetKernelArg 'local': "+arg_index+" - size : "+arg_size);
#endif        
        ( CL.webcl_mozilla == 1 ) ? CL.kernels[ker].setKernelArgLocal(arg_index,arg_size) : CL.kernels[ker].setArg(arg_index,arg_size,WebCLKernelArgumentTypes.LOCAL_MEMORY_SIZE);
      } else if (arg_size > 4) {
        value = new Array(arg_size/4);
        
        for (var i = 0; i < arg_size/4; i++) {
        
          if (isFloat) {
            value[i] = {{{ makeGetValue('arg_value', 'i*4', 'float') }}};   
          } else {
            value[i] = {{{ makeGetValue('arg_value', 'i*4', 'i32') }}};
          }
        }
        
        var type;
        if ( CL.webcl_webkit == 1 ) {
          if (arg_size/4 == 2)
            type = WebCLKernelArgumentTypes.VEC2;
          if (arg_size/4 == 3)
            type = WebCLKernelArgumentTypes.VEC3;
          if (arg_size/4 == 4)
            type = WebCLKernelArgumentTypes.VEC4;
        }

        if (isFloat) {    
          //CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.FLOAT_V)
          ( CL.webcl_mozilla == 1 ) ? CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.FLOAT_V) : CL.kernels[ker].setArg(arg_index,value,WebCLKernelArgumentTypes.FLOAT | type);
        } else {          
          //CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.INT_V)
          ( CL.webcl_mozilla == 1 ) ? CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.INT_V) : CL.kernels[ker].setArg(arg_index,value,WebCLKernelArgumentTypes.INT | type);
        } 
        
      } else {     
        if (isFloat) {
          value = {{{ makeGetValue('arg_value', '0', 'float') }}};
        } else {
          value = {{{ makeGetValue('arg_value', '0', 'i32') }}};
        }
        
        if (arg_index >= 0 && arg_index < CL.buffers.length) {
          ( CL.webcl_mozilla == 1 ) ? CL.kernels[ker].setKernelArg(arg_index,CL.buffers[arg_index]) : CL.kernels[ker].setArg(arg_index,CL.buffers[arg_index]);
        } else {
          if (isFloat) { 
            ( CL.webcl_mozilla == 1 ) ? CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.FLOAT) : CL.kernels[ker].setArg(arg_index,value,WebCLKernelArgumentTypes.FLOAT);
          } else {
            ( CL.webcl_mozilla == 1 ) ? CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.INT) : CL.kernels[ker].setArg(arg_index,value,WebCLKernelArgumentTypes.INT);
          }            
        }        
      }
        
      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clSetKernelArg",e);
    }
  },

  clGetKernelWorkGroupInfo: function(kernel, devices, param_name, param_value_size, param_value, param_value_size_ret) {
    var ker = kernel - 1;
    if (ker >= CL.kernels.length || ker < 0 ) {
#if OPENCL_DEBUG
      console.error("clGetKernelWorkGroupInfo: Invalid kernel : "+ker);
#endif

      return -48; /* CL_INVALID_KERNEL */
    }

    // \todo the type is a number but why i except have a Array ??? Will must be an array ???
    var idx = 0;//{{{ makeGetValue('devices', '0', 'i32') }}} - 1;

    if (idx >= CL.devices.length || idx < 0 ) {
#if OPENCL_DEBUG
      console.error("clGetKernelWorkGroupInfo: Invalid device : "+idx);
#endif
      return -33; /* CL_INVALID_DEVICE */  
    }

    try {        
      var res;
      switch (param_name) {
        case (0x11B0) /* CL_KERNEL_WORK_GROUP_SIZE */:
          if (CL.webcl_mozilla == 1) {
            res = CL.kernels[ker].getKernelWorkGroupInfo(CL.devices[idx],WebCL.CL_KERNEL_WORK_GROUP_SIZE);
          } else {
            res = CL.kernels[ker].getWorkGroupInfo(CL.devices[idx],WebCL.KERNEL_WORK_GROUP_SIZE);
          }
        break;
      case (0x11B1) /*    CL_KERNEL_COMPILE_WORK_GROUP_SIZE    */:
        if (CL.webcl_mozilla == 1) {
          res = CL.kernels[ker].getKernelWorkGroupInfo(CL.devices[idx],WebCL.CL_KERNEL_COMPILE_WORK_GROUP_SIZE);
        } else {
          res = CL.kernels[ker].getWorkGroupInfo(CL.devices[idx],WebCL.KERNEL_COMPILE_WORK_GROUP_SIZE);
        }
        break;
      case (0x11B2) /*    CL_KERNEL_LOCAL_MEM_SIZE    */:
        if (CL.webcl_mozilla == 1) {
          res = CL.kernels[ker].getKernelWorkGroupInfo(CL.devices[idx],WebCL.CL_KERNEL_LOCAL_MEM_SIZE);
        } else {
          res = CL.kernels[ker].getWorkGroupInfo(CL.devices[idx],WebCL.CL_KERNEL_LOCAL_MEM_SIZE);
        }
        break;
      };

      {{{ makeSetValue('param_value', '0', 'res', 'i32') }}}

      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clGetKernelWorkGroupInfo",e);
    }
  },
  
  clEnqueueUnmapMemObject: function(command_queue, buffer, ptr, num_events_in_wait_list, event_wait_list, event) {
    var queue = command_queue - 1;
    if (queue >= CL.cmdQueue.length || queue < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueUnmapMemObject: Invalid command queue : "+queue);
#endif

      return -36; /* CL_INVALID_COMMAND_QUEUE */
    }
    
    var buff = buffer - 1;
    if (buff >= CL.buffers.length || buff < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueMapBuffer: Invalid buffer mem : "+buff);
#endif
      return -38;  /* CL_INVALID_MEM_OBJECT */
    }
    
    try {
      
      CL.cmdQueue[queue].enqueueUnnmapBuffer(CL.buffers[buff], ptr, []);
  
      return 0;/*CL_SUCCESS*/
      
    } catch(e) {
      return CL.catchError("clEnqueueMapBuffer",e);
    }    
  },

  clEnqueueNDRangeKernel: function(command_queue, kernel, work_dim, global_work_offset, global_work_size, local_work_size, num_events_in_wait_list, event_wait_list, event) {
    var queue = command_queue - 1;
    if (queue >= CL.cmdQueue.length || queue < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueNDRangeKernel: Invalid command queue : "+queue);
#endif

      return -36; /* CL_INVALID_COMMAND_QUEUE */
    }

    var ker = kernel - 1;
    if (ker >= CL.kernels.length || ker < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueNDRangeKernel: Invalid kernel : "+ker);
#endif

      return -48; /* CL_INVALID_KERNEL */
    }
    
    var value_local_work_size;
    var value_global_work_size;
    
    if (CL.webcl_mozilla == 1) {
      value_local_work_size = [];
      value_global_work_size = [];
    } else {
      value_local_work_size = new Int32Array(work_dim);
      value_global_work_size = new Int32Array(work_dim);
    }
    
    for (var i = 0 ; i < work_dim; i++) {
      value_local_work_size[i] = {{{ makeGetValue('local_work_size', 'i*4', 'i32') }}};
      value_global_work_size[i] = {{{ makeGetValue('global_work_size', 'i*4', 'i32') }}};
    }

#if 0
    var global = "";
    var local = "";
    for (var i = 0 ; i < work_dim; i++){
      global += value_global_work_size[i];
      local += value_local_work_size[i];
      if (i != work_dim -1) {
        global += " , ";
        local += " , ";
      }
    }

    console.info("Global [ "+ global +" ]")
    console.info("Local [ "+ local +" ]")
#endif
  
    // empty “localWS” array because give some trouble on CPU mode with mac
    // value_local_work_size = [];  

    try {

      // \todo how add some event inside the array
      if (CL.webcl_mozilla == 1) {
        CL.cmdQueue[queue].enqueueNDRangeKernel(CL.kernels[ker],work_dim,/*global_work_offset*/[],value_global_work_size,value_local_work_size,[]);
      } else {
        CL.cmdQueue[queue].enqueueNDRangeKernel(CL.kernels[ker], /*global_work_offset*/ null, value_global_work_size, value_local_work_size);
      }
      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clEnqueueNDRangeKernel",e);
    }
  },
  
  clWaitForEvents: function(num_events, event_list) {
    try {
      //var list = [];
      // \todo how grab the event ??
      //for (var i = 0 ; i < num_events; i++) {
      //  list.push({{{ makeGetValue('event_list', 'i*4', 'i32') }}})        
      //}
      //WebCL.waitForEvents(list);
 
      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clWaitForEvents",e);
    }
  },

  clFinish: function(command_queue) {
    var queue = command_queue - 1;
    if (queue >= CL.cmdQueue.length || queue < 0 ) {
#if OPENCL_DEBUG
      console.error("clFinish: Invalid command queue : "+queue);
#endif

      return -36; /* CL_INVALID_COMMAND_QUEUE */
    }

    try {
      CL.cmdQueue[queue].finish(); //Finish all the operations
 
      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clFinish",e);
    }
  },

  clEnqueueReadBuffer: function(command_queue, buffer, blocking_read, offset, size, results, num_events_in_wait_list, event_wait_list, event) {
    var queue = command_queue - 1;
    if (queue >= CL.cmdQueue.length || queue < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueReadBuffer: Invalid command queue : "+queue);
#endif

      return -36; /* CL_INVALID_COMMAND_QUEUE */
    }

    var buff = buffer - 1;
    if (buff >= CL.buffers.length || buff < 0 ) {
#if OPENCL_DEBUG
      console.error("clEnqueueReadBuffer: Invalid buffer : "+buff);
#endif
      return -38; /* CL_INVALID_MEM_OBJECT */
    }

    try {
      
      var vector;
      var isFloat = 0;
      var isUint = 0;
      var isInt = 0;

      if (CL.kernels_name.length > 0) {
        // \warning experimental stuff
      
        console.info("/!\\ clEnqueueReadBuffer: Need to find how detect the array type");
        var name = CL.kernels_name[0];
        console.info("/!\\ clEnqueueReadBuffer: use '"+name+"' kernel name ...");
        var sig = CL.kernels_sig[name];
        var type = sig[buff];

        if (type & CL.data_type.FLOAT) {
          isFloat = 1;
        } 
        if (type & CL.data_type.UINT) {
          isUint = 1;
        } 
        if (type & CL.data_type.INT) {
          isInt = 1;
        }
      }

      if (isFloat) {
        vector = new Float32Array(size / Float32Array.BYTES_PER_ELEMENT);
        console.info("/!\\ clEnqueueReadBuffer: use FLOAT output type ...");
      } else if (isUint) {
        vector = new Uint32Array(size / Uint32Array.BYTES_PER_ELEMENT);
        console.info("/!\\ clEnqueueReadBuffer: use UINT output type ...");
      } else if (isInt) {
        vector = new Int32Array(size / Int32Array.BYTES_PER_ELEMENT);
        console.info("/!\\ clEnqueueReadBuffer: use INT output type ...");
      } else {
#if OPENCL_DEBUG
        console.error("clEnqueueReadBuffer: Unknow ouptut type : "+sig[buff]);
#endif
      }
       
      console.info("clEnqueueReadBuffer - Pos : "+buff);
        
      CL.cmdQueue[queue].enqueueReadBuffer (CL.buffers[buff], blocking_read == 1 ? true : false, offset, size, vector, []);

      for (var i = 0; i < (size / 4); i++) {
        if (isFloat) {
          {{{ makeSetValue('results', 'i*4', 'vector[i]', 'float') }}};  
        } else {
          {{{ makeSetValue('results', 'i*4', 'vector[i]', 'i32') }}};  
        }         
      }
    
#if OPENCL_DEBUG
      console.info(vector);
#endif
      
      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clEnqueueReadBuffer",e);
    }
  },

  clReleaseMemObject: function(memobj) {            
    var buff = memobj - 1 - CL.buffers_clean;
    if (buff >= CL.buffers.length || buff < 0 ) {
#if OPENCL_DEBUG
      console.error("clReleaseMemObject: Invalid Memory Object : "+buff);
#endif

      return -38; /* CL_INVALID_MEM_OBJECT */
    }

    CL.buffers.splice(buff, 1);

    if (CL.buffers.length == 0) {
      CL.buffers_clean = 0;
    } else {
      CL.buffers_clean++;
    }

#if OPENCL_DEBUG
    console.info("clReleaseMemObject: Release Memory Object : "+buff);
#endif

    return 0;/*CL_SUCCESS*/
  },

  clReleaseProgram: function(program) {
    var prog = program - 1 - CL.programs_clean;
    if (prog >= CL.programs.length || prog < 0 ) {
#if OPENCL_DEBUG
      console.error("clReleaseProgram: Invalid program : "+prog);
#endif
      return -44; /* CL_INVALID_PROGRAM */
    }           

    CL.programs.splice(prog, 1);
    if (CL.programs.length == 0) {
      CL.programs_clean = 0;
    } else {
      CL.programs_clean++;
    }
#if OPENCL_DEBUG
    console.info("clReleaseProgram: Release program : "+prog);
#endif

    return 0;/*CL_SUCCESS*/
  },

  clReleaseKernel: function(kernel) {
    var ker = kernel - 1 - CL.kernels_clean;
    if (ker >= CL.kernels.length || ker < 0 ) {
#if OPENCL_DEBUG
      console.error("clReleaseKernel: Invalid kernel : "+ker);
#endif

      return -48; /* CL_INVALID_KERNEL */
    }

    CL.kernels.splice(ker, 1);
    if (CL.kernels.length == 0) {
      CL.kernels_clean = 0;
    } else {
      CL.kernels_clean++;
    }    
#if OPENCL_DEBUG
    console.info("clReleaseKernel: Release kernel : "+ker);
#endif

    return 0;/*CL_SUCCESS*/
  },

  clReleaseCommandQueue: function(command_queue) {
    var queue = command_queue - 1 - CL.cmdQueue_clean;
    if (queue >= CL.cmdQueue.length || queue < 0 ) {
#if OPENCL_DEBUG
      console.error("clReleaseCommandQueue: Invalid command queue : "+queue);
#endif

      return -36; /* CL_INVALID_COMMAND_QUEUE */
    }

    CL.cmdQueue.splice(queue, 1);
    if (CL.cmdQueue.length == 0) {
      CL.cmdQueue_clean = 0;
    } else {
      CL.cmdQueue_clean++;
    }
#if OPENCL_DEBUG
    console.info("clReleaseCommandQueue: Release command queue : "+queue);
#endif

    return 0;/*CL_SUCCESS*/
  },

  clReleaseContext: function(context) {
    var ctx = context - 1 - CL.ctx_clean;
    if (ctx >= CL.ctx.length || ctx < 0 ) {
#if OPENCL_DEBUG
      console.error("clReleaseContext: Invalid context : "+ctx);
#endif

      return -34; /* CL_INVALID_CONTEXT */
    }        

    CL.ctx.splice(ctx, 1);
    if (CL.ctx.length == 0) {
      CL.ctx_clean = 0;
    } else {
      CL.ctx_clean++;
    }
#if OPENCL_DEBUG
    console.info("clReleaseContext: Release context : "+ctx);
#endif

    return 0;/*CL_SUCCESS*/
  },
};

autoAddDeps(LibraryOpenCL, '$CL');
mergeInto(LibraryManager.library, LibraryOpenCL);

