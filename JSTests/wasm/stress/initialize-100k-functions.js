//@ requireOptions("--useDollarVM=1")

import * as assert from "../assert.js"

async function main() {
    let bytes = read('initialize-100k-functions.wasm', 'binary');
    await WebAssembly.compile(bytes).then((module) => {
        if (MemoryFootprint().peak > 100000000) {
            $vm.abort()
        }
        return 0;
    });
}

await assert.asyncTest(main());

