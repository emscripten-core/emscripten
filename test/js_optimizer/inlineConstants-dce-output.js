export const EXPORTED = 3;

function handle(cmd) {
  if (cmd === 1) return 10;
  if (cmd === 2) return 20;
  return 0;
}

export function run(cmd) {
  return handle(cmd) + 3;
}
