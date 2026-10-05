const CMD_LOAD = 1;
const CMD_RUN = 2;
const UNUSED = 99;
export const EXPORTED = 3;

function handle(cmd) {
  if (cmd === CMD_LOAD) return 10;
  if (cmd === CMD_RUN) return 20;
  return 0;
}

export function run(cmd) {
  return handle(cmd) + EXPORTED;
}
