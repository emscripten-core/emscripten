import init, { _used_externally } from './library.mjs';

await init();

console.log(_used_externally());
console.log('done');
