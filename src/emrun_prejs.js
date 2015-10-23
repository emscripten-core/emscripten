// Route URL GET parameters to argc+argv
if (typeof window === "object") {
  Module['arguments'] = window.location.search.substr(1).trim().split('&');
  // If no args were passed arguments = [''], in which case kill the single empty string.
  if (!Module['arguments'][0])
    Module['arguments'] = [];
}