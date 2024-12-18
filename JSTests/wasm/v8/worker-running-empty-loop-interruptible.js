//@ requireOptions("--useBBQJIT=1", "--useWebAssemblyLLInt=1", "--webAssemblyLLIntTiersUpToBBQ=1")
//@ skip
// Failure:
// Exception: ReferenceError: Can't find variable: Worker
//  global code@worker-running-empty-loop-interruptible.js:32:26

// Copyright 2022 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

load("wasm-module-builder.js");

const builder = new WasmModuleBuilder();
// void main() { for (;;) {} }
builder.addFunction('main', kSig_v_v).addBody([
  kExprLoop, kWasmVoid, kExprBr, 0, kExprEnd
]).exportFunc();
const module = builder.toModule();

function workerCode() {
  onmessage = function(module) {
    // print('[worker] Creating instance.');
    let instance = new WebAssembly.Instance(module);
    // print('[worker] Reporting start.');
    postMessage('start');
    // print('[worker] Running main.');
    instance.exports.main();
  };
}

// print('[main] Starting worker.');
const worker = new Worker(workerCode, {type: 'function'});
// print('[main] Sending module.');
worker.postMessage(module);
assertEquals('start', worker.getMessage());
// print('[main] Terminating worker and waiting for it to finish.');
worker.terminateAndWait();
// print('[main] All done.');
