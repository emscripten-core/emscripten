Module['onRuntimeInitialized'] = async () => {
  try {
    let delayedThrowResult = Module["delayed_throw"]();
    assert(delayedThrowResult instanceof Promise);
    let err = await delayedThrowResult.then(() => '', err => err.message);
    assert(err === 'my message', `"${err}" doesn't contain the expected error`);

    let fooResult = Module["foo"]();
    assert(fooResult instanceof Promise);
    fooResult = await fooResult;
    assert(fooResult === 10);

    let barInstancePromise = new Module["Bar"]();
    assert(barInstancePromise instanceof Promise);
    let barInstance = await barInstancePromise;
    assert(barInstance instanceof Module["Bar"]);
    assert(barInstance.x === 20);

    let barMethodResult = barInstance.method();
    assert(barMethodResult instanceof Promise);
    assert(await barMethodResult === 30);

    assert(barInstance.method_without_async_calls() === 40);
    assert(barInstance.void_method_without_async_calls() === undefined);

    barMethodResult = barInstance.void_method_with_async_calls();
    assert(barMethodResult instanceof Promise);
    assert(await barMethodResult === undefined);

    let barStaticMethodResult = Module["Bar"].static_method();
    assert(barStaticMethodResult instanceof Promise);
    assert(await barStaticMethodResult === 50);

    if (ASSERTIONS) {
      let err = '';
      try {
        barInstance.method();
        barInstance.method();
      } catch (e) {
        err = e.message;
      }
      assert(err.includes('Assertion failed: Cannot have multiple async operations in flight at once)'), `"${err}" doesn't contain the assertion error`);
    }

    console.log('ok');
  } catch (e) {
    console.log('error: ' + e.stack);
  }
};
