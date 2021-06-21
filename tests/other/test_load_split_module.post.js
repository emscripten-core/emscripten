Module["loadSplitModule"] = function(deferred, imports, prop) {
    console.log('Custom handler for loading split module.');    
    if (prop && !isNaN(parseInt(prop))) {
        console.log('Called with placeholder');
        console.log(prop);
    }
    return instantiateSync(deferred, imports);
}