Module["loadSplitModule"] = function(deferred, imports, prop) {
    console.log('Custom handler for loading split module.');    
    console.log('Called with placeholder ', prop);
    
    return instantiateSync(deferred, imports);
}
