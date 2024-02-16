if (!ENVIRONMENT_IS_PTHREAD) {
  if (ENVIRONMENT_IS_NODE) {
    process.on('uncaughtException', (e) => {
      if (e.message.includes('rejected!')) {
        console.log('passed');
      }
    });
  } else {
    addEventListener('error', (e) => {
      if (e.message === 'Uncaught rejected!') {
        console.log('passed');
      }
    });
  }
}
