EXPORTED_RUNTIME_METHODS.add("myFunc");

addToLibrary({
  $myFunc__postset: 'console.log("myFunc included")',
  $myFunc: () => {
    console.log("myFunc called");
  },
})
