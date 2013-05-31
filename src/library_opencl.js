//"use strict";

var LibraryOpenCL = {
  $CL__deps: ['$Browser'],
  $CL: {
    ctx: [],
    ctx_clean: 0,
    cmdQueue: [],
    cmdQueue_clean: 0,
    programs: [],
    programs_clean: 0,
    kernels: [],
    kernels_clean: 0,
    buffers: [],
    buffers_clean: 0,
    platforms: [],
    devices: [],
    sig: [],
    errorMessage: "Unfortunately your system does not support WebCL. " +
                    "Make sure that you have both the OpenCL driver " +
                    "and the WebCL browser extension installed.",
      
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
    if (window.WebCL == undefined) {
      console.log(CL.errorMessage);
      return -1;/*CL_DEVICE_NOT_FOUND*/;
    }
    
    try { 
      
      // Get the platform
      var platforms = WebCL.getPlatformIDs();
      
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
        var name = plat.getPlatformInfo (WebCL.CL_PLATFORM_NAME);
        var vendor = plat.getPlatformInfo (WebCL.CL_PLATFORM_VENDOR);
        var version = plat.getPlatformInfo (WebCL.CL_PLATFORM_VERSION);
        var extensions = plat.getPlatformInfo (WebCL.CL_PLATFORM_EXTENSIONS);
        console.info("\t"+i+": name: " + name);              
        console.info("\t"+i+": vendor: " + vendor);              
        console.info("\t"+i+": version: " + version);
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
    if (window.WebCL == undefined) {
      console.log(CL.errorMessage);
      return -1;/*CL_DEVICE_NOT_FOUND*/;
    }

    // Assume the device type is i32 
    assert(device_type_i64_2 == 0, 'Invalid flags i64');

    try { 

      // If platform is NULL, the behavior is implementation-defined
      if (platform == 0 && CL.platforms.length == 0) {
          var platforms = WebCL.getPlatformIDs();
          
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
      
      var alldev = CL.platforms[platform].getDeviceIDs(WebCL.CL_DEVICE_TYPE_ALL);
           
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
        var type = alldev[i].getDeviceInfo(WebCL.CL_DEVICE_TYPE);
        if (type == device_type_i64_1 || device_type_i64_1 == -1) {
           var name = alldev[i].getDeviceInfo(WebCL.CL_DEVICE_NAME);
           map[name] = alldev[i];
           mapcount ++;
        }        
      }
                  
      if (mapcount == 0) {
        alldev = CL.platforms[platform].getDeviceIDs(WebCL.CL_DEVICE_TYPE_DEFAULT);
        for (var i = 0 ; i < alldev.length; i++) {
          var name = alldev[i].getDeviceInfo(WebCL.CL_DEVICE_NAME);
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
          res = CL.ctx[ctx].getContextInfo(WebCL.CL_CONTEXT_DEVICES);
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
          res = CL.ctx[ctx].getContextInfo(WebCL.CL_CONTEXT_PROPERTIES);
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

    try {
      var res;
      var size;
      switch (param_name) {
        case (0x100D) /* CL_DEVICE_ADDRESS_BITS */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_ADDRESS_BITS); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;
        case (0x1028) /* CL_DEVICE_COMPILER_AVAILABLE */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_COMPILER_AVAILABLE); // return cl_bool
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;
        case (0x1027) /* CL_DEVICE_AVAILABLE */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_AVAILABLE); // return cl_bool
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;
        case (0x1026) /* CL_DEVICE_ENDIAN_LITTLE */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_ENDIAN_LITTLE); // return cl_bool
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;
        case (0x1024) /* CL_DEVICE_ERROR_CORRECTION_SUPPORT */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_ERROR_CORRECTION_SUPPORT); // return cl_bool
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;   
        case (0x1030) /* CL_DEVICE_EXTENSIONS */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_EXTENSIONS); // return string
          size = res.length;
          writeStringToMemory(res, param_value);
          break;
        case (0x101E) /* CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;   
        case (0x1016) /* CL_DEVICE_IMAGE_SUPPORT*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_IMAGE_SUPPORT); // return true or false
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break; 
        case (0x1011) /* CL_DEVICE_IMAGE2D_MAX_WIDTH*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_IMAGE2D_MAX_WIDTH); // return size_t
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;   
        case (0x1012) /* CL_DEVICE_IMAGE2D_MAX_HEIGHT*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_IMAGE2D_MAX_HEIGHT); // return size_t
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;   
        case (0x1013) /* CL_DEVICE_IMAGE3D_MAX_WIDTH*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_IMAGE3D_MAX_WIDTH); // return size_t
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;   
        case (0x1014) /* CL_DEVICE_IMAGE3D_MAX_HEIGHT*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_IMAGE3D_MAX_HEIGHT); // return size_t
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;    
        case (0x1015) /* CL_DEVICE_IMAGE3D_MAX_DEPTH*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_IMAGE3D_MAX_DEPTH); // return size_t
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;         
        case (0x100C) /* CL_DEVICE_MAX_CLOCK_FREQUENCY */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_CLOCK_FREQUENCY); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;
        case (0x1002) /* CL_DEVICE_MAX_COMPUTE_UNITS */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_COMPUTE_UNITS); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;   
        case (0x1021) /* CL_DEVICE_MAX_CONSTANT_ARGS */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_CONSTANT_ARGS); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;     
               
        case (0x1017) /* CL_DEVICE_MAX_PARAMETER_SIZE*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_PARAMETER_SIZE); // return size_t
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break; 
        case (0x100E) /* CL_DEVICE_MAX_READ_IMAGE_ARGS*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_READ_IMAGE_ARGS); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break; 
        case (0x1018) /* CL_DEVICE_MAX_SAMPLERS*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_SAMPLERS); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;     
        case (0x1004) /* CL_DEVICE_MAX_WORK_GROUP_SIZE*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_WORK_GROUP_SIZE); // return size_t
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break; 
        case (0x1003) /* CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;    
        case (0x100F) /* CL_DEVICE_MAX_WRITE_IMAGE_ARGS*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_WRITE_IMAGE_ARGS); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;    
        case (0x1019) /* CL_DEVICE_MEM_BASE_ADDR_ALIGN*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MEM_BASE_ADDR_ALIGN); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;                  
        case (0x101A) /* CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;              
        case (0x102B) /* CL_DEVICE_NAME */:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_NAME); // return string
          size = res.length;
          writeStringToMemory(res, param_value);
          break;
        case (0x1031) /* CL_DEVICE_PLATFORM*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_PLATFORM); // return cl_platform_id 
          // \todo how return the good platform inside the tab ?????
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;     
        case (0x1006) /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;     
        case (0x1007) /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;     
        case (0x1008) /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;     
        case (0x1009) /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;     
        case (0x100A) /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;     
        case (0x100B) /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break; 
//          case (0x1034) /* CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF*/:
//           res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF); // return cl_uint
//           {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
//           size = 1;
//           break;         
//         case (0x1036) /* CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR*/:
//           res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR); // return cl_uint
//           {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
//           size = 1;
//           break;     
//         case (0x1037) /* CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT*/:
//           res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT); // return cl_uint
//           {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
//           size = 1;
//           break;     
//         case (0x1038) /* CL_DEVICE_NATIVE_VECTOR_WIDTH_INT*/:
//           res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_NATIVE_VECTOR_WIDTH_INT); // return cl_uint
//           {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
//           size = 1;
//           break;     
//         case (0x1039) /* CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG*/:
//           res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG); // return cl_uint
//           {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
//           size = 1;
//           break;     
//         case (0x103A) /* CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT*/:
//           res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT); // return cl_uint
//           {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
//           size = 1;
//           break;     
//         case (0x103B) /* CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE*/:
//           res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE); // return cl_uint
//           {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
//           size = 1;
//           break;    
//         case (0x103C) /* CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF*/:
//           res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF); // return cl_uint
//           {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
//           size = 1;
//           break;             
        case (0x1001) /* CL_DEVICE_VENDOR_ID*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_VENDOR_ID); // return cl_uint
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;       
        case (0x102C) /* CL_DEVICE_VENDOR*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_VENDOR); // return string
          writeStringToMemory(res, param_value);
          size = res.length;
          break;
        case (0x1000) /* CL_DEVICE_TYPE*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_TYPE); // return cl_device i64
          // \todo return the type with i32 is wrong ????? seems ok with result but not really sure !!!!
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;
        case (0x1004) /* CL_DEVICE_MAX_WORK_GROUP_SIZE*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_WORK_GROUP_SIZE); // return size_t
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;
        case (0x102D) /* CL_DRIVER_VERSION*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DRIVER_VERSION); // return string
          writeStringToMemory(res, param_value);
          size = res.length;
          break;   
        case (0x102F) /* CL_DEVICE_VERSION*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_VERSION); // return string
          writeStringToMemory(res, param_value);
          size = res.length;
          break;   
        case (0x102E) /* CL_DEVICE_PROFILE*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_PROFILE); // return string
          writeStringToMemory(res, param_value);
          size = res.length;
          break;  
        case (0x1010) /* CL_DEVICE_MAX_MEM_ALLOC_SIZE*/:
          res = CL.devices[idx].getDeviceInfo(WebCL.CL_DEVICE_MAX_MEM_ALLOC_SIZE); // return cl_ulong
          {{{ makeSetValue('param_value', '0', 'res', 'i32') }}};
          size = 1;
          break;
          
        default:
#if OPENCL_DEBUG
          console.error("clGetDeviceInfo : Param not yet implemented or unknow : "+param_name);
#endif
          return -30; /* CL_INVALID_VALUE */ 
      };
      
      {{{ makeSetValue('param_value_size_ret', '0', 'size', 'i32') }}};
                
      return 0;/*CL_SUCCESS*/
    } catch (e) {
      return CL.catchError("clGetDeviceInfo",e);
    }    
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
      CL.ctx.push(WebCL.createContext(prop, devices_tab/*[CL.devices[0],CL.devices[1]]*/));

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
      var alldev = CL.platforms[plat].getDeviceIDs(WebCL.CL_DEVICE_TYPE_ALL);
      var mapcount = 0;
    
      for (var i = 0 ; i < alldev.length; i++ ) {
        var type = alldev[i].getDeviceInfo(WebCL.CL_DEVICE_TYPE);
        if (type == device_type_i64_1 || device_type_i64_1 == -1) {
           mapcount ++;
        }        
      }
          
      if (mapcount >= 1) {        
        CL.ctx.push(WebCL.createContextFromType(prop, device_type_i64_1));
      } else {
        // Use default platform
        CL.ctx.push(WebCL.createContextFromType(prop, WebCL.CL_DEVICE_TYPE_DEFAULT));
      }

      // Return the pos of the context +1
      return CL.ctx.length;
      
    } catch (e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateContext",e)', 'i32') }}};
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
        var devices = CL.platforms[platform].getDeviceIDs(WebCL.CL_DEVICE_TYPE_DEFAULT);
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
    
    // Experimental parse of kernel for have the different type of the kernel (input and output)
    var start_kernel = kernel.indexOf("__kernel");
    var kernel_sub_part = kernel.substr(start_kernel,kernel.length - start_kernel);
    var start_kernel_brace = kernel_sub_part.indexOf("(");
    var close_kernel_brace = kernel_sub_part.indexOf(")");
    kernel_sub_part = kernel_sub_part.substr(start_kernel_brace + 1,close_kernel_brace - start_kernel_brace);
    kernel_sub_part = kernel_sub_part.replace(/\n/g, "");
    
    var kernel_sub_part_split = kernel_sub_part.split(",");
    for (var i = 0; i < kernel_sub_part_split.length; i++) {
      
      if (kernel_sub_part_split[i].contains("float4") ||
         (kernel_sub_part_split[i].contains("float") && kernel_sub_part_split[i].contains("*"))) {
           
        console.info("Kernel Parameter "+i+" typeof is float4 or float* ("+WebCL.types.FLOAT_V+")");
        CL.sig[i] = WebCL.types.FLOAT_V;
        
      } else if (kernel_sub_part_split[i].contains("float")) {
        
        console.info("Kernel Parameter "+i+" typeof is float ("+WebCL.types.FLOAT+")");
        CL.sig[i] = WebCL.types.FLOAT;    
                
      } else if (kernel_sub_part_split[i].contains("uchar4") ||
                (kernel_sub_part_split[i].contains("unsigned") && kernel_sub_part_split[i].contains("char") && kernel_sub_part_split[i].contains("*")) ||
                (kernel_sub_part_split[i].contains("unsigned") && kernel_sub_part_split[i].contains("int") && kernel_sub_part_split[i].contains("*"))) {
                  
        console.info("Kernel Parameter "+i+" typeof is uchar4 or unsigned char* or unsigned int * ("+WebCL.types.UINT_V+")");
        CL.sig[i] = WebCL.types.UINT_V;
        
      } else if (kernel_sub_part_split[i].contains("unsigned") && kernel_sub_part_split[i].contains("int")) {
        
        console.info("Kernel Parameter "+i+" typeof is unsigned int ("+WebCL.types.UINT+")");
        CL.sig[i] = WebCL.types.UINT;        
        
      } else if (kernel_sub_part_split[i].contains("int") && kernel_sub_part_split[i].contains("*")) {
        
        console.info("Kernel Parameter "+i+" typeof is int * ("+WebCL.types.INT_V+")");
        CL.sig[i] = WebCL.types.INT_V;    
        
      } else if (kernel_sub_part_split[i].contains("int")) {
        
        console.info("Kernel Parameter "+i+" typeof is int ("+WebCL.types.INT+")");
        CL.sig[i] = WebCL.types.INT;    
        
      } else {
        console.error("Unknow type of parameter : "+kernel_sub_part_split[i]);        
      }
    }
    
    try {
      // \todo set the properties 
      CL.programs.push(CL.ctx[ctx].createProgramWithSource(kernel));

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
        // \todo will be better to use the devices list in parameter ...
        for (var i = 0; i < num_devices; i++) {
          devices_tab[i] = CL.devices[i];
        }
      }    

      var opt = "";

      CL.programs[prog].buildProgram (devices_tab, opt);

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
    var idx = {{{ makeGetValue('device', '0', 'i32') }}} - 1;

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

  clCreateKernel: function(program, kernel_name, errcode_ret) {
    var prog = program - 1;
    if (prog >= CL.programs.length || prog < 0 ) {
#if OPENCL_DEBUG
      console.error("clCreateKernel: Invalid program : "+prog);
#endif
      {{{ makeSetValue('errcode_ret', '0', '-44', 'i32') }}};
      return 0; // Null pointer   
    }           

    try {
      //console.log("kernel_name : "+Pointer_stringify(kernel_name));
      var name = Pointer_stringify(kernel_name);
      CL.kernels.push(CL.programs[prog].createKernel(name));

      // Return the pos of the queue +1
      return CL.kernels.length;
    } catch (e) {
      {{{ makeSetValue('errcode_ret', '0', 'CL.catchError("clCreateKernel",e)', 'i32') }}};
      return 0; // Null pointer    
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
          CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.CL_MEM_READ_WRITE,size));
          break;
        case (1 << 1) /* CL_MEM_WRITE_ONLY */:
          CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.CL_MEM_WRITE_ONLY,size));
          break;
        case (1 << 2) /* CL_MEM_READ_ONLY */:
          CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.CL_MEM_READ_ONLY,size));
          break;
        case (1 << 3) /* CL_MEM_USE_HOST_PTR */:
          CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.CL_MEM_USE_HOST_PTR,size));
          break;
        case (1 << 4) /* CL_MEM_ALLOC_HOST_PTR */:
          CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.CL_MEM_ALLOC_HOST_PTR,size));
          break;
        case (1 << 5) /* CL_MEM_COPY_HOST_PTR */:
          CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.CL_MEM_COPY_HOST_PTR,size));
          break;
        case (((1 << 2)|(1 << 5))) /* CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR */:
          if (host_ptr == 0) {
#if OPENCL_DEBUG
            console.error("clCreateBuffer: CL_MEM_COPY_HOST_PTR can't be use with null host_ptr parameter");
#endif
            {{{ makeSetValue('errcode_ret', '0', '-37', 'i32') }}} /* CL_INVALID_HOST_PTR */;
            return 0;     
          }
          
          CL.buffers.push(CL.ctx[ctx].createBuffer(WebCL.CL_MEM_READ_ONLY,size));
          
          // CL_MEM_COPY_HOST_PTR Doesn't work we pass element via enqueue buffer
          
          if (CL.cmdQueue.length == 0) {
#if OPENCL_DEBUG
            console.error("clCreateBuffer: Invalid command queue : "+CL.cmdQueue.length);
#endif
            {{{ makeSetValue('errcode_ret', '0', '-36', 'i32') }}} /* CL_INVALID_COMMAND_QUEUE */;
            return 0;
          }

          if (CL.buffers.length == 0) {
#if OPENCL_DEBUG
            console.error("clCreateBuffer: Invalid command queue : "+CL.buffers.length);
#endif
            {{{ makeSetValue('errcode_ret', '0', '-38', 'i32') }}} /* CL_INVALID_MEM_OBJECT */;
            return 0;
          }
          
          if (CL.sig.length == 0 || CL.buffers-1 > CL.sig.length) {
#if OPENCL_DEBUG
            console.error("clCreateBuffer: Invalid signature : "+buff);
#endif
            return -1; /* CL_FAILED */     
          }

          var isFloat = 0;
          var vector;    
             
          if (CL.sig[CL.buffers.length-1] == WebCL.types.FLOAT_V) {
            vector = new Float32Array(size / 4);
            isFloat = 1;
          } else if (CL.sig[CL.buffers.length-1] == WebCL.types.UINT_V) {
            vector = new Uint32Array(size / 4);
          } else if (CL.sig[CL.buffers.length-1] == WebCL.types.INT_V) {
            vector = new Int32Array(size / 4);
          } else {
#if OPENCL_DEBUG
            console.error("clCreateBuffer: Unknow ouptut type : "+CL.sig[CL.buffers.length-1]);
#endif
          }
  
          for (var i = 0; i < (size / 4); i++) {
            if (isFloat) {
              vector[i] = {{{ makeGetValue('host_ptr', 'i*4', 'float') }}};
            } else {
              vector[i] = {{{ makeGetValue('host_ptr', 'i*4', 'i32') }}};
            }
          }
    
          CL.cmdQueue[CL.cmdQueue.length-1].enqueueWriteBuffer(CL.buffers[CL.buffers.length-1], 1, 0, size, vector , []);    

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
          console.error("clCreateBuffer: flag not yet implemented "+flags_i64_1);
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

    var isFloat = 0;
    var vector;    
    
    if (CL.sig.length == 0 || buff > CL.sig.length) {
      isFloat = CL.isFloat(ptr,size); 
      if (isFloat) {
        vector = new Float32Array(size / 4);
      } else {
        vector = new Int32Array(size / 4);
      }
    } else {        
      if (CL.sig[buff] == WebCL.types.FLOAT_V) {
        vector = new Float32Array(size / 4);
        isFloat = 1;
      } else if (CL.sig[buff] == WebCL.types.UINT_V) {
        vector = new Uint32Array(size / 4);
      } else if (CL.sig[buff] == WebCL.types.INT_V) {
        vector = new Int32Array(size / 4);
      } else {
#if OPENCL_DEBUG
        console.error("clEnqueueWriteBuffer: Unknow ouptut type : "+CL.sig[buff]);
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

  clSetKernelArg: function(kernel, arg_index, arg_size, arg_value) {
    var ker = kernel - 1;
    if (ker >= CL.kernels.length || ker < 0 ) {
#if OPENCL_DEBUG
      console.error("clSetKernelArg: Invalid kernel : "+ker);
#endif

      return -48; /* CL_INVALID_KERNEL */
    }
                        
    try {  
          
      // \todo problem what is arg_value is buffer or just value ??? hard to say ....
      // \todo i suppose the arg_index correspond with the order of the buffer creation if is 
      // not inside the buffers array size we take the value
      
      var isFloat = 0;
      if (CL.sig.length > 0 && arg_index < CL.sig.length) {
        isFloat = ( CL.sig[arg_index] == WebCL.types.FLOAT_V ) || ( CL.sig[arg_index] == WebCL.types.FLOAT ) 
      } else {
#if OPENCL_DEBUG
        console.error("clSetKernelArg: Invalid signature : "+CL.sig.length);
#endif
        return -1; /* CL_FAILED */
      }
        
      var isNull = ({{{ makeGetValue('arg_value', '0', 'i32') }}} == 0);

      var value;
      if (isNull == 1) {
        CL.kernels[ker].setKernelArgLocal(arg_index,arg_size);
      } else if (arg_size > 4) {
        value = [];
        
        for (var i = 0; i < arg_size/4; i++) {
        
          if (isFloat == 1) {
            value[i] = {{{ makeGetValue('arg_value', 'i*4', 'float') }}};   
          } else {
            value[i] = {{{ makeGetValue('arg_value', 'i*4', 'i32') }}};
          }
        }
        if (isFloat == 1) {
          CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.FLOAT_V);
        } else {
          CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.INT_V);
        }      
      } else {     
        if (isFloat == 1) {
          value = {{{ makeGetValue('arg_value', '0', 'float') }}};
        } else {
          value = {{{ makeGetValue('arg_value', '0', 'i32') }}};
        }
        
        if (arg_index >= 0 && arg_index < CL.buffers.length) {
          CL.kernels[ker].setKernelArg(arg_index,CL.buffers[arg_index]);
        } else {
          if (isFloat == 1) { 
            CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.FLOAT);
          } else {
            CL.kernels[ker].setKernelArg(arg_index,value,WebCL.types.INT);
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
        res = CL.kernels[ker].getKernelWorkGroupInfo(CL.devices[idx],WebCL.CL_KERNEL_WORK_GROUP_SIZE);
        break;
      case (0x11B1) /*    CL_KERNEL_COMPILE_WORK_GROUP_SIZE    */:
        res = CL.kernels[ker].getKernelWorkGroupInfo(CL.devices[idx],WebCL.CL_KERNEL_COMPILE_WORK_GROUP_SIZE);
        break;
      case (0x11B2) /*    CL_KERNEL_LOCAL_MEM_SIZE    */:
        res = CL.kernels[ker].getKernelWorkGroupInfo(CL.devices[idx],WebCL.CL_KERNEL_LOCAL_MEM_SIZE);
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
      return CL.catchError("clEnqueueNDRangeKernel",e);
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

    var value_local_work_size = [];
    var value_global_work_size = [];
    
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
    value_local_work_size = [];  

    try {

      // \todo how add some event inside the array
      CL.cmdQueue[queue].enqueueNDRangeKernel(CL.kernels[ker],work_dim,[],value_global_work_size,value_local_work_size,[]);

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

      if (CL.sig.length == 0 || buff > CL.sig.length) {
#if OPENCL_DEBUG
        console.error("clEnqueueReadBuffer: Invalid signature : "+buff);
#endif
        return -1; /* CL_FAILED */     
      }

      var isFloat = 0;
      var vector;    
        
      if (CL.sig[buff] == WebCL.types.FLOAT_V) {
        vector = new Float32Array(size / 4);
        isFloat = 1;
      } else if (CL.sig[buff] == WebCL.types.UINT_V) {
        vector = new Uint32Array(size / 4);
      } else if (CL.sig[buff] == WebCL.types.INT_V) {
        vector = new Int32Array(size / 4);
      } else {
#if OPENCL_DEBUG
        console.error("clEnqueueReadBuffer: Unknow ouptut type : "+CL.sig[buff]);
#endif
        return -1; /* CL_FAILED */     
      }

      CL.cmdQueue[queue].enqueueReadBuffer (CL.buffers[buff], blocking_read == 1 ? true : false, offset, size, vector, []);

      for (var i = 0; i < (size / 4); i++) {
        if (isFloat) {
          {{{ makeSetValue('results', 'i*4', 'vector[i]', 'float') }}};  
        } else {
          {{{ makeSetValue('results', 'i*4', 'vector[i]', 'i32') }}};  
        }         
      }

      return 0;/*CL_SUCCESS*/
    } catch(e) {
      return CL.catchError("clEnqueueReadBuffer",e);
    }
  },

  clReleaseMemObject: function(memobj) {
    var buff = memobj - 1 - CL.buffers_clean;
    if (buff >= CL.buffers.length || buff < 0 ) {
#if OPENCL_DEBUG
      console.error("clReleaseMemObject: Invalid command queue : "+buff);
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
    console.message("clReleaseMemObject: Release Memory Object : "+buff);
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
    console.message("clReleaseProgram: Release program : "+prog);
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
    console.message("clReleaseKernel: Release kernel : "+ker);
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
    console.message("clReleaseCommandQueue: Release command queue : "+queue);
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
    console.message("clReleaseContext: Release context : "+ctx);
#endif

    return 0;/*CL_SUCCESS*/
  },
};

autoAddDeps(LibraryOpenCL, '$CL');
mergeInto(LibraryManager.library, LibraryOpenCL);

