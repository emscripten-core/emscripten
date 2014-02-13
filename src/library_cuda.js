var LibraryCUDA = {  
  $CU__deps: ['$GL','$CL'],
  $CU: {
    
  },

  cuInit: function(Flags) {
    console.error("cuInit: Not yet implemented\n");
  },

  cuDriverGetVersion: function(driverVersion) {
    console.error("cuDriverGetVersion: Not yet implemented\n");
  },

  cuDeviceGet: function(device,ordinal) {
    console.error("cuDeviceGet: Not yet implemented\n");
  },

  cuDeviceGetCount: function(count) {
    console.error("cuDeviceGetCount: Not yet implemented\n");
  },

  cuDeviceGetName: function(name,len,dev) {
    console.error("cuDeviceGetName: Not yet implemented\n");
  },

  cuDeviceComputeCapability: function(major,minor,dev) {
    console.error("cuDeviceComputeCapability: Not yet implemented\n");
  },

  cuDeviceTotalMem: function(bytes,dev) {
    console.error("cuDeviceTotalMem: Not yet implemented\n");
  },

  cuDeviceGetProperties: function(prop,dev) {
    console.error("cuDeviceGetProperties: Not yet implemented\n");
  },

  cuDeviceGetAttribute: function(pi,attrib,dev) {
    console.error("cuDeviceGetAttribute: Not yet implemented\n");
  },

  cuCtxCreate: function(pctx,flags,dev) {
    console.error("cuCtxCreate: Not yet implemented\n");
  },

  cuCtxDestroy: function(ctx) {
    console.error("cuCtxDestroy: Not yet implemented\n");
  },

  cuCtxAttach: function(pctx,flags) {
    console.error("cuCtxAttach: Not yet implemented\n");
  },

  cuCtxDetach: function(ctx) {
    console.error("cuCtxDetach: Not yet implemented\n");
  },

  cuCtxPushCurrent: function(ctx) {
    console.error("cuCtxPushCurrent: Not yet implemented\n");
  },

  cuCtxPopCurrent: function(pctx) {
    console.error("cuCtxPopCurrent: Not yet implemented\n");
  },

  cuCtxGetDevice: function(device) {
    console.error("cuCtxGetDevice: Not yet implemented\n");
  },

  cuCtxSynchronize: function() {
    console.error("cuCtxSynchronize: Not yet implemented\n");
  },

  cuModuleLoad: function(module,fname) {
    console.error("cuModuleLoad: Not yet implemented\n");
  },

  cuModuleLoadData: function(module,image) {
    console.error("cuModuleLoadData: Not yet implemented\n");
  },

  cuModuleLoadDataEx: function(module,image,numOptions,options,optionValues) {
    console.error("cuModuleLoadDataEx: Not yet implemented\n");
  },

  cuModuleLoadFatBinary: function(module,fatCubin) {
    console.error("cuModuleLoadFatBinary: Not yet implemented\n");
  },

  cuModuleUnload: function(hmod) {
    console.error("cuModuleUnload: Not yet implemented\n");
  },

  cuModuleGetFunction: function(hfunc,hmod,name) {
    console.error("cuModuleGetFunction: Not yet implemented\n");
  },

  cuModuleGetGlobal: function(dptr,bytes,hmod,name) {
    console.error("cuModuleGetGlobal: Not yet implemented\n");
  },

  cuModuleGetTexRef: function(pTexRef,hmod,name) {
    console.error("cuModuleGetTexRef: Not yet implemented\n");
  },

  cuMemGetInfo: function(free,total) {
    console.error("cuMemGetInfo: Not yet implemented\n");
  },

  cuMemAlloc: function(dptr,bytesize) {
    console.error("cuMemAlloc: Not yet implemented\n");
  },

  cuMemAllocPitch: function(dptr,pPitch,WidthInBytes,Height,ElementSizeBytes) {
    console.error("cuMemAllocPitch: Not yet implemented\n");
  },

  cuMemFree: function(dptr) {
    console.error("cuMemFree: Not yet implemented\n");
  },

  cuMemGetAddressRange: function(pbase,psize,dptr) {
    console.error("cuMemGetAddressRange: Not yet implemented\n");
  },

  cuMemAllocHost: function(pp,bytesize) {
    console.error("cuMemAllocHost: Not yet implemented\n");
  },

  cuMemFreeHost: function(p) {
    console.error("cuMemFreeHost: Not yet implemented\n");
  },

  cuMemHostAlloc: function(pp,bytesize,Flags) {
    console.error("cuMemHostAlloc: Not yet implemented\n");
  },

  cuMemHostGetDevicePointer: function(pdptr,p,Flags) {
    console.error("cuMemHostGetDevicePointer: Not yet implemented\n");
  },

  cuMemHostGetFlags: function(pFlags,p) {
    console.error("cuMemHostGetFlags: Not yet implemented\n");
  },

  cuMemcpyHtoD: function(dstDevice,srcHost,ByteCount) {
    console.error("cuMemcpyHtoD: Not yet implemented\n");
  },

  cuMemcpyDtoH: function(dstHost,srcDevice,ByteCount) {
    console.error("cuMemcpyDtoH: Not yet implemented\n");
  },

  cuMemcpyDtoD: function(dstDevice,srcDevice,ByteCount) {
    console.error("cuMemcpyDtoD: Not yet implemented\n");
  },

  cuMemcpyDtoA: function(dstArray,dstIndex,srcDevice,ByteCount) {
    console.error("cuMemcpyDtoA: Not yet implemented\n");
  },

  cuMemcpyAtoD: function(dstDevice,hSrc,SrcIndex,ByteCount) {
    console.error("cuMemcpyAtoD: Not yet implemented\n");
  },

  cuMemcpyHtoA: function(dstArray,dstIndex,pSrc,ByteCount) {
    console.error("cuMemcpyHtoA: Not yet implemented\n");
  },

  cuMemcpyAtoH: function(dstHost,srcArray,srcIndex,ByteCount) {
    console.error("cuMemcpyAtoH: Not yet implemented\n");
  },

  cuMemcpyAtoA: function(dstArray,dstIndex,srcArray,srcIndex,ByteCount) {
    console.error("cuMemcpyAtoA: Not yet implemented\n");
  },

  cuMemcpy2D: function(pCopy) {
    console.error("cuMemcpy2D: Not yet implemented\n");
  },

  cuMemcpy2DUnaligned: function(pCopy) {
    console.error("cuMemcpy2DUnaligned: Not yet implemented\n");
  },

  cuMemcpy3D: function(pCopy) {
    console.error("cuMemcpy3D: Not yet implemented\n");
  },

  cuMemcpyHtoDAsync: function(dstDevice,srcHost,ByteCount,hStream) {
    console.error("cuMemcpyHtoDAsync: Not yet implemented\n");
  },

  cuMemcpyDtoHAsync: function(dstHost,srcDevice,ByteCount,hStream) {
    console.error("cuMemcpyDtoHAsync: Not yet implemented\n");
  },

  cuMemcpyDtoDAsync: function(dstDevice,srcDevice,ByteCount,hStream) {
    console.error("cuMemcpyDtoDAsync: Not yet implemented\n");
  },

  cuMemcpyHtoAAsync: function(dstArray,dstIndex,pSrc,ByteCount,hStream) {
    console.error("cuMemcpyHtoAAsync: Not yet implemented\n");
  },

  cuMemcpyAtoHAsync: function(dstHost,srcArray,srcIndex,ByteCount,hStream) {
    console.error("cuMemcpyAtoHAsync: Not yet implemented\n");
  },

  cuMemcpy2DAsync: function(pCopy,hStream) {
    console.error("cuMemcpy2DAsync: Not yet implemented\n");
  },

  cuMemcpy3DAsync: function(pCopy,hStream) {
    console.error("cuMemcpy3DAsync: Not yet implemented\n");
  },

  cuMemsetD8: function(dstDevice,uc,N) {
    console.error("cuMemsetD8: Not yet implemented\n");
  },

  cuMemsetD16: function(dstDevice,us,N) {
    console.error("cuMemsetD16: Not yet implemented\n");
  },

  cuMemsetD32: function(dstDevice,ui,N) {
    console.error("cuMemsetD32: Not yet implemented\n");
  },

  cuMemsetD2D8: function(dstDevice,dstPitch,uc,Width,Height) {
    console.error("cuMemsetD2D8: Not yet implemented\n");
  },

  cuMemsetD2D16: function(dstDevice,dstPitch,us,Width,Height) {
    console.error("cuMemsetD2D16: Not yet implemented\n");
  },

  cuMemsetD2D32: function(dstDevice,dstPitch,ui,Width,Height) {
    console.error("cuMemsetD2D32: Not yet implemented\n");
  },

  cuFuncSetBlockShape: function(hfunc,x,y,z) {
    console.error("cuFuncSetBlockShape: Not yet implemented\n");
  },

  cuFuncSetSharedSize: function(hfunc,bytes) {
    console.error("cuFuncSetSharedSize: Not yet implemented\n");
  },

  cuFuncGetAttribute: function(pi,attrib,hfunc) {
    console.error("cuFuncGetAttribute: Not yet implemented\n");
  },

  cuFuncSetCacheConfig: function(hfunc,config) {
    console.error("cuFuncSetCacheConfig: Not yet implemented\n");
  },

  cuArrayCreate: function(pHandle,pAllocateArray) {
    console.error("cuArrayCreate: Not yet implemented\n");
  },

  cuArrayGetDescriptor: function(pArrayDescriptor,hArray) {
    console.error("cuArrayGetDescriptor: Not yet implemented\n");
  },

  cuArrayDestroy: function(hArray) {
    console.error("cuArrayDestroy: Not yet implemented\n");
  },

  cuArray3DCreate: function(pHandle,pAllocateArray) {
    console.error("cuArray3DCreate: Not yet implemented\n");
  },

  cuArray3DGetDescriptor: function(pArrayDescriptor,hArray) {
    console.error("cuArray3DGetDescriptor: Not yet implemented\n");
  },

  cuTexRefCreate: function(pTexRef) {
    console.error("cuTexRefCreate: Not yet implemented\n");
  },

  cuTexRefDestroy: function(hTexRef) {
    console.error("cuTexRefDestroy: Not yet implemented\n");
  },

  cuTexRefSetArray: function(hTexRef,hArray,Flags) {
    console.error("cuTexRefSetArray: Not yet implemented\n");
  },

  cuTexRefSetAddress: function(ByteOffset,hTexRef,dptr,bytes) {
    console.error("cuTexRefSetAddress: Not yet implemented\n");
  },

  cuTexRefSetAddress2D: function(hTexRef,desc,dptr,Pitch) {
    console.error("cuTexRefSetAddress2D: Not yet implemented\n");
  },

  cuTexRefSetFormat: function(hTexRef,fmt,NumPackedComponents) {
    console.error("cuTexRefSetFormat: Not yet implemented\n");
  },

  cuTexRefSetAddressMode: function(hTexRef,dim,am) {
    console.error("cuTexRefSetAddressMode: Not yet implemented\n");
  },

  cuTexRefSetFilterMode: function(hTexRef,fm) {
    console.error("cuTexRefSetFilterMode: Not yet implemented\n");
  },

  cuTexRefSetFlags: function(hTexRef,Flags) {
    console.error("cuTexRefSetFlags: Not yet implemented\n");
  },

  cuTexRefGetAddress: function(pdptr,hTexRef) {
    console.error("cuTexRefGetAddress: Not yet implemented\n");
  },

  cuTexRefGetArray: function(phArray,hTexRef) {
    console.error("cuTexRefGetArray: Not yet implemented\n");
  },

  cuTexRefGetAddressMode: function(pam,hTexRef,dim) {
    console.error("cuTexRefGetAddressMode: Not yet implemented\n");
  },

  cuTexRefGetFilterMode: function(pfm,hTexRef) {
    console.error("cuTexRefGetFilterMode: Not yet implemented\n");
  },

  cuTexRefGetFormat: function(pFormat,pNumChannels,hTexRef) {
    console.error("cuTexRefGetFormat: Not yet implemented\n");
  },

  cuTexRefGetFlags: function(pFlags,hTexRef) {
    console.error("cuTexRefGetFlags: Not yet implemented\n");
  },

  cuParamSetSize: function(hfunc,numbytes) {
    console.error("cuParamSetSize: Not yet implemented\n");
  },

  cuParamSeti: function(hfunc,offset,value) {
    console.error("cuParamSeti: Not yet implemented\n");
  },

  cuParamSetf: function(hfunc,offset,value) {
    console.error("cuParamSetf: Not yet implemented\n");
  },

  cuParamSetv: function(hfunc,offset,ptr,numbytes) {
    console.error("cuParamSetv: Not yet implemented\n");
  },

  cuParamSetTexRef: function(hfunc,texunit,hTexRef) {
    console.error("cuParamSetTexRef: Not yet implemented\n");
  },

  cuLaunch: function(f) {
    console.error("cuLaunch: Not yet implemented\n");
  },

  cuLaunchGrid: function(f,grid_width,grid_height) {
    console.error("cuLaunchGrid: Not yet implemented\n");
  },

  cuLaunchGridAsync: function(f,grid_width,grid_height,hStream) {
    console.error("cuLaunchGridAsync: Not yet implemented\n");
  },

  cuEventCreate: function(phEvent,Flags) {
    console.error("cuEventCreate: Not yet implemented\n");
  },

  cuEventRecord: function(hEvent,hStream) {
    console.error("cuEventRecord: Not yet implemented\n");
  },

  cuEventQuery: function(hEvent) {
    console.error("cuEventQuery: Not yet implemented\n");
  },

  cuEventSynchronize: function(hEvent) {
    console.error("cuEventSynchronize: Not yet implemented\n");
  },

  cuEventDestroy: function(hEvent) {
    console.error("cuEventDestroy: Not yet implemented\n");
  },

  cuEventElapsedTime: function(pMilliseconds,hStart,hEnd) {
    console.error("cuEventElapsedTime: Not yet implemented\n");
  },

  cuStreamCreate: function(phStream,Flags) {
    console.error("cuStreamCreate: Not yet implemented\n");
  },

  cuStreamQuery: function(hStream) {
    console.error("cuStreamQuery: Not yet implemented\n");
  },

  cuStreamSynchronize: function(hStream) {
    console.error("cuStreamSynchronize: Not yet implemented\n");
  },

  cuStreamDestroy: function(hStream) {
    console.error("cuStreamDestroy: Not yet implemented\n");
  },

  cuGraphicsUnregisterResource: function(resource) {
    console.error("cuGraphicsUnregisterResource: Not yet implemented\n");
  },

  cuGraphicsSubResourceGetMappedArray: function(pArray,resource,arrayIndex,mipLevel) {
    console.error("cuGraphicsSubResourceGetMappedArray: Not yet implemented\n");
  },

  cuGraphicsResourceGetMappedPointer: function(pDevPtr,pSize,resource) {
    console.error("cuGraphicsResourceGetMappedPointer: Not yet implemented\n");
  },

  cuGraphicsResourceSetMapFlags: function(resource,flags) {
    console.error("cuGraphicsResourceSetMapFlags: Not yet implemented\n");
  },
 
  cuGraphicsMapResources: function(count,resources,hStream) {
    console.error("cuGraphicsMapResources: Not yet implemented\n");
  },

  cuGraphicsUnmapResources: function(count,resources,hStream) {
    console.error("cuGraphicsUnmapResources: Not yet implemented\n");
  },

  cuGetExportTable: function(ppExportTable,pExportTableId) {
    console.error("cuGetExportTable: Not yet implemented\n");
  },

  cuGLCtxCreate: function(pCtx,Flags,device) {
    console.error("cuGLCtxCreate: Not yet implemented\n");
  },

  cuGraphicsGLRegisterBuffer: function(pCudaResource,GLubuffer,Flags) {
    console.error("cuGraphicsGLRegisterBuffer: Not yet implemented\n");
  },

  cuGraphicsGLRegisterImage: function(pCudaResource,GLuimage,target,Flags) {
    console.error("cuGraphicsGLRegisterImage: Not yet implemented\n");
  },

};

autoAddDeps(LibraryCUDA, '$CU');
mergeInto(LibraryManager.library, LibraryCUDA);



