import { Emlite } from "../src/emlite.js";

async function main() {
    const emlite = new Emlite();
    const bytes = await emlite.readFile(new URL("../bin/freestanding/eval.wasm", import.meta.url));
    let wasm = await WebAssembly.compile(bytes);
    let instance = await WebAssembly.instantiate(wasm, {
        env: emlite.env,
    });
    emlite.setExports(instance.exports);
    // if you have another exported function marked with EMLITE_USED, you can get it in the instance exports
    instance.exports.main();
}

await main();