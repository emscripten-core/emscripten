import init, { _foo as foo } from "./modularize_static.mjs";
await init();
foo();
