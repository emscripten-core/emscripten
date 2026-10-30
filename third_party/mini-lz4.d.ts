export interface CompressedPackage {
  data: Uint8Array | null;
  cachedOffset: number;
  cachedIndexes: number[];
  cachedChunks: (Uint8Array | null)[];
  offsets: number[];
  sizes: number[];
  successes: number[];
}
export function compressPackage(data: ArrayBuffer, verify?: boolean): CompressedPackage;
