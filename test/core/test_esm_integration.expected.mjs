// The wasm module must be imported here first before the support file
// in order to avoid issues with circular dependencies.
import * as unused from './hello_world.wasm';
export { default, _foo, _main, err, stringToNewUTF8 } from './hello_world.support.mjs';
