Module["loadSplitModule"] = async function(deferred, imports, prop) {
    console.log('Custom handler for loading split module.');

    return instantiateAsync(null, deferred, imports);
}
