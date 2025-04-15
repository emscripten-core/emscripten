// The wasm module must be imported here first before the support file
// in order to avoid issues with circular dependencies.
import * as unused from './hello_world.wasm';
export { default, _foo, _main, err, stringToNewUTF8 } from './hello_world.support.mjs';

// When run as the main module under node, execute main directly here
import init from './hello_world.support.mjs';
const isNode = typeof process == 'object' && typeof process.versions == 'object' && typeof process.versions.node == 'string' && process.type != 'renderer';
if (isNode) {
  const url = await import('url');
  const isMainModule = url.pathToFileURL(process.argv[1]).href === import.meta.url;
  if (isMainModule) await init();
}
