function shouldThrow(func, errorMessage) {
    var errorThrown = false;
    var error = null;
    try {
        func();
    } catch (e) {
        errorThrown = true;
        error = e;
    }
    if (!errorThrown)
        throw new Error('not thrown');
    if (String(error) !== errorMessage)
        throw new Error(`bad error: ${String(error)}`);
}

shouldThrow(() => {
    var wasm_code = new Uint8Array([0,97,115,109,1,0,0,0,1,5,1,96,0,1,127,3,2,1,0,4,4,1,112,0,1,6,6,1,127,0,65,0,11,7,8,1,4,109,97,105,110,0,0,9,7,1,0,35,0,11,1,0,10,6,1,4,0,65,1,11,0,14,4,110,97,109,101,1,7,1,0,4,109,97,105,110,]);
    var wasm_module = new WebAssembly.Module(wasm_code);
}, `CompileError: WebAssembly.Module doesn't parse at byte 49: get_global import kind index 0 is non-import (evaluating 'new WebAssembly.Module(wasm_code)')`);
