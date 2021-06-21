Module["loadSplitModule"] = function(_prop, deferred, imports) {
    console.log('Custom handler for loading split module.');
    return instantiateSync(deferred, imports);
}