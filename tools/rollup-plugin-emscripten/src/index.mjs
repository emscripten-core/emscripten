import assert from 'node:assert/strict';
import path from 'path';
import {execSync} from 'child_process';
import fs from 'fs';
import tmp from 'tmp';
import which from 'which';

// Cleanup temp files on exit.
tmp.setGracefulCleanup();

/**
 * Rollup plugin that enables tree shaking of Wasm libraries produced by
 * Emscripten. Currently only works with libraries produced with
 * `-sMODULARIZE=instance`.
 *
 * @param {Object} pluginOptions
 * @param {string} pluginOptions.input - The name of the JS file emitted from
 *   Emscripten.
 * @param {string} [pluginOptions.wasmMetaDCE] The path to the wasm-metadce
 *   binary (defaults to wasm-metadce in path).
 */
export default async function emscriptenPlugin(pluginOptions = {}) {
  assert(pluginOptions.input, 'Emscripten plugin requires input name.');
  pluginOptions.wasmMetaDCE ??= await which('wasm-metadce');
  const inputPath = path.parse(pluginOptions.input);

  let exportInfo;
  return {
    name: 'emscripten',
    moduleParsed(moduleInfo) {
      if (path.basename(moduleInfo.id) === pluginOptions.input) {
        // Collect info about the exports that will be needed later to map
        // the ESM export name back to the Wasm export name.
        const exportNames = getEsmExportNames(moduleInfo.ast);
        exportInfo = getExportInfo(moduleInfo.ast, exportNames);
      }
    },
    async generateBundle(options, chunkMap) {
      assert(options.dir, 'Emscripten plugin requires dir option.');
      assert(exportInfo, 'Failed to find Emscripten output.');
      for (const [_file, chunk] of Object.entries(chunkMap)) {
        for (const [id, module] of Object.entries(chunk.modules)) {
          if (path.basename(id) === pluginOptions.input) {
            const tmpWasm = tmp.tmpNameSync({postfix: '.wasm'});
            const wasmFile = path.join(path.dirname(id), `${inputPath.name}.wasm`);
            const wasmExports = await getUsedExports(
              this,
              module,
              wasmFile,
              exportInfo,
              new Set(module.removedExports),
            );
            await runDCE(wasmExports, wasmFile, pluginOptions.wasmMetaDCE, tmpWasm);
            this.emitFile({
              type: 'asset',
              source: fs.readFileSync(tmpWasm),
              name: 'Emscripten Rollup WASM Asset',
              fileName: `${inputPath.name}.wasm`,
            });
          }
        }
      }
    },
  };
}

async function getUsedExports(rollup, module, wasmFilename, exportInfo, removedEsmExports) {
  // Collect all the current exports from the wasm file.
  // TODO: would it be better to use binaryen to do this?
  const wasmBuffer = fs.readFileSync(wasmFilename);
  let wasmModule = await WebAssembly.compile(wasmBuffer);
  const wasmExports = new Set();
  for (const wasmExport of WebAssembly.Module.exports(wasmModule)) {
    wasmExports.add(wasmExport.name);
  }

  // From the list of exports that rollup removed, see if the export has a
  // corresponding wasm export.
  const potentiallyRemovedExports = new Map();
  for (const esmExport of removedEsmExports) {
    if (exportInfo.has(esmExport)) {
      potentiallyRemovedExports.set(esmExport, exportInfo.get(esmExport));
    }
  }
  // Rollup may have removed some ESM exports that are still used internally.
  // Check to see if they are actually unused.
  const unusedExports = getUnusedExports(rollup.parse(module.code), potentiallyRemovedExports);

  // Remove each export that is no longer used.
  for (const wasmExport of wasmExports) {
    if (unusedExports.has(wasmExport)) {
      wasmExports.delete(wasmExport);
    }
  }
  return wasmExports;
}

async function runDCE(wasmExports, wasmFilename, wasmMetaDcePath, outputFile) {
  // Create the wasm-metadce graph.
  const graph = [
    {
      name: 'outside',
      reaches: Array.from(wasmExports),
      root: true,
    },
  ];
  for (const exportName of wasmExports) {
    graph.push({
      name: exportName,
      export: exportName,
    });
  }
  const graphFile = tmp.tmpNameSync({postfix: '.json'});
  fs.writeFileSync(graphFile, JSON.stringify(graph));
  execSync(`${wasmMetaDcePath} ${wasmFilename} --graph-file ${graphFile} -o ${outputFile}`);
}

function visitChildren(node, callback) {
  for (const key in node) {
    if (node.hasOwnProperty(key)) {
      const value = node[key];
      if (value && typeof value === 'object' && value.type) {
        callback(value);
      } else if (Array.isArray(value)) {
        value.forEach(callback);
      }
    }
  }
}

// Traverse an AST and collect all of the ESM exports and their aliases (if applicable).
// Returns a Map where localName => exportedName.
// e.g. export { foo as _foo }; is foo => _foo
function getEsmExportNames(ast) {
  const exportNames = new Map();

  function walk(node) {
    switch (node.type) {
      case 'ExportNamedDeclaration':
        if (node.declaration) {
          // For `export const foo = 1;` or `export function bar() {}`
          if (node.declaration.type === 'VariableDeclaration') {
            for (const declaration of node.declaration.declarations) {
              exportNames.set(declaration.id.name, declaration.id.name);
            }
          } else if (node.declaration.type === 'FunctionDeclaration') {
            exportNames.set(node.declaration.id.name, node.declaration.id.name);
          }
        } else if (node.specifiers) {
          // For `export { foo, bar as baz } from 'module';`
          for (const specifier of node.specifiers) {
            exportNames.set(specifier.local.name, specifier.exported.name);
          }
        }
        break;
    }
    // Exports are only allowed at the top level, so no need to continue walking
    // the tree.
  }

  visitChildren(ast, walk);
  return exportNames;
}

// Traverse the AST and search for the wasm exports to map them back to the
// ESM exports.
// Returns a Map(esmExportName => { wasmName, varName, localName })
function getExportInfo(ast, variableNames) {
  const result = new Map();

  function walk(node) {
    if (node.type === 'VariableDeclarator' && node.init) {
      const init = node.init;
      if (init.type === 'AssignmentExpression') {
        if (variableNames.has(init.left.name) && init.right) {
          // For `var _foo =  __exp__foo = createExportWrapper('foo', 0);`
          if (
            init.right.type === 'CallExpression' &&
            init.right.callee.name === 'createExportWrapper' &&
            init.right.arguments.length === 2 &&
            init.right.arguments[0].type === 'Literal' &&
            typeof init.right.arguments[0].value === 'string'
          ) {
            result.set(variableNames.get(init.left.name), {
              localName: init.left.name,
              wasmName: init.right.arguments[0].value,
              varName: node.id.name,
            });
          }
          // For `var _foo =  __exp__foo = wasmExports['foo'];`
          else if (
            init.right.type === 'MemberExpression' &&
            init.right.object.name === 'wasmExports' &&
            init.right.property.type === 'Literal' &&
            typeof init.right.property.value === 'string'
          ) {
            result.set(variableNames.get(init.left.name), {
              localName: init.left.name,
              wasmName: init.right.property.value,
              varName: node.id.name,
            });
          }
        }
      }
    }
    visitChildren(node, walk);
  }

  walk(ast);
  return result;
}

// Walk the AST and look for places that the exports still may be used.
function getUnusedExports(ast, exports) {
  const unusedExports = new Set();
  const varExports = new Map();
  for (const value of exports.values()) {
    varExports.set(value.varName, value);
    unusedExports.add(value.wasmName);
  }

  function walk(node) {
    // Check if the export is called e.g. _foo().
    if (
      node.type === 'CallExpression' &&
      node.callee.type === 'Identifier' &&
      varExports.has(node.callee.name)
    ) {
      unusedExports.delete(varExports.get(node.callee.name).wasmName);
    }
    // Check if export is accessed e.g. _foo.call()
    if (
      node.type === 'MemberExpression' &&
      node.object.type === 'Identifier' &&
      varExports.has(node.object.name)
    ) {
      unusedExports.delete(varExports.get(node.object.name).wasmName);
    }
    visitChildren(node, walk);
  }

  walk(ast);

  return unusedExports;
}
