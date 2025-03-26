import moduleFactory from './test_emit_tsd_sync.js'

const module = moduleFactory();
module._fooVoid();
module._fooInt(7, 8);
