onrtctransform = (event) => {
    let getFrame = false;
    let cloneFrame = false;
    const transformer = event.transformer;
    transformer.options.port.onmessage = (event) => {
console.log(event.data);
        if (event.data === "getFrame")
            getFrame = true;
        else if (event.data === "startCloningFrame")
            cloneFrame = true;
        else if (event.data === "stopCloningFrame")
            cloneFrame = false;
    };

    transformer.options.port.postMessage("started");
    transformer.reader = transformer.readable.getReader();
    transformer.writer = transformer.writable.getWriter();

    function process(transformer)
    {
        transformer.reader.read().then(chunk => {
            if (chunk.done)
                return;
            let frame = chunk.value;
            if (getFrame) {
                getFrame = false;
                transformer.options.port.postMessage(frame);
            }
            if (cloneFrame) {
                if (frame instanceof RTCEncodedVideoFrame)
                    frame = new RTCEncodedVideoFrame(frame);
                else if (frame instanceof RTCEncodedAudioFrame)
                    frame = new RTCEncodedAudioFrame(frame);
            }
            transformer.writer.write(frame);
            process(transformer);
        });
    }

    process(transformer);
};
self.postMessage("registered");
