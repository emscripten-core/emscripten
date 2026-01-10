// The wasm module must be imported here first before the support file
// in order to avoid issues with circular dependencies.
import * as unused from './hello_world.wasm';
export { default, _foo, _main, err, stringToNewUTF8 } from './hello_world.support.mjs';

// When run as the main module under node, create the module directly.  This will
// execute any startup code along with main (if it exists).
import init from './hello_world.support.mjs';
const isNode = globalThis.process?.versions?.node && globalThis.process?.type != 'renderer';
if (isNode) {
  const url = await import('node:url');
  const isMainModule = url.pathToFileURL(process.argv[1]).href === import.meta.url;
  if (isMainModule) await init();
}
