// Import the Emscripten glue code
importScripts('ppm_web.js');

const moduleConfig = {
    // Redirect C++ stdout to the main thread
    print: (text) => {
        postMessage({ type: 'LOG', message: "[C++] " + text });
    },
    // Redirect C++ stderr to the main thread
    printErr: (text) => {
        postMessage({ type: 'ERROR', message: "[C++ Error] " + text });
    }
};

let wasmModule = null;
let processor = null;

// Initialize WASM
createModule(moduleConfig).then(instance => {
    wasmModule = instance;
    processor = new wasmModule.ImageProcessor();
    // Notify main thread we are ready
    postMessage({ type: 'READY' });
});

onmessage = async function(e) {
    const { type, payload } = e.data;

    if (!processor) {
        console.error("WASM not ready yet");
        return;
    }

    if (type === 'LOAD_IMAGE') {
        try {
            // 1. Allocate memory in WASM heap
            const len = payload.byteLength;
            const ptr = wasmModule._malloc(len);
            
            // 2. Copy data to WASM
            wasmModule.HEAPU8.set(new Uint8Array(payload), ptr);

            // 3. Decode image
            const success = processor.loadImage(ptr, len);
            
            // 4. Free temp buffer
            wasmModule._free(ptr);

            if (success) {
                const w = processor.getWidth();
                const h = processor.getHeight();
                // Send back dimensions to update UI
                postMessage({ type: 'IMAGE_LOADED', width: w, height: h });
                sendImageToMain(); // Send the initial image to render
            } else {
                postMessage({ type: 'ERROR', message: 'Failed to load image (stbi error)' });
            }
        } catch (err) {
            postMessage({ type: 'ERROR', message: err.message });
        }

    } else if (type === 'APPLY_FILTER') {
        const { filterType, intensity } = payload;
        
        try {
            // This call BLOCKS, but it's okay because we are in a Worker!
            processor.applyFilter(intensity, filterType);
            
            sendImageToMain();
            postMessage({ type: 'FILTER_DONE' });
        } catch (err) {
            postMessage({ type: 'ERROR', message: err.message });
        }
    }
};

function sendImageToMain() {
    const w = processor.getWidth();
    const h = processor.getHeight();
    const ptr = processor.getPixelDataPtr();

    // Create a COPY of the pixels to send to main thread
    // We cannot send the WASM shared buffer directly, so we copy.
    const pixelData = new Uint8ClampedArray(wasmModule.HEAPU8.buffer, ptr, w * h * 4);
    
    // Copy to a new buffer that can be transferred or cloned
    const resultBuffer = new Uint8ClampedArray(pixelData);

    postMessage({ 
        type: 'RENDER', 
        pixels: resultBuffer, 
        width: w, 
        height: h 
    }, [resultBuffer.buffer]); // Transferable optimization
}
