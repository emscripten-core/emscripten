addToLibrary({
  // Test asynchronously calling a function on the main thread.
  report_result__proxy: 'async',
  report_result__sig: 'viii',
  report_result: function(param1, param2, param3) {
    if (typeof ENVIRONMENT_IS_WORKER !== 'undefined' && ENVIRONMENT_IS_WORKER) {
      console.error('This function should be getting called on the main thread!');
    }
    console.log('got ' + param1 + ' ' + param2 + ' ' + param3);
    __ReportResult(param1 + param2 * param3);
  }
});
