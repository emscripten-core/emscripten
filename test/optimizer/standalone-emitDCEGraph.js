// In standalone mode we might emit this in the JS support code, but the funcs
// are not emitted (since the build can't depend on JS). This test verifies
// metadce doesn't fail on such a thing (but, if the JS is actually used and
// this call reached, it will fail, of course, as that's not compatible with
// standalone mode). 
dynCall('v', 0)

var wasmImports = {};
