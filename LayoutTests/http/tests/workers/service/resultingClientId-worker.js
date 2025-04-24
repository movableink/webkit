onmessage = e => {
    if (e.data === "getClientId") {
        e.source.postMessage(self.clientId);
        return;
    }
    e.source.postMessage(self.resultingClientId);
};

onfetch = e => {
    self.clientId = e.clientId;
    if (e.resultingClientId)
        self.resultingClientId = e.resultingClientId;

    const text = `<html><body><script>
        onload = async () => {
            await fetch("/");
            history.back();
        };
        </script></body></html>`;
    const headers = [
       ["Cache-Control", "no-cache, no-store, must-revalidate, max-age=0"],
       ["Content-Type", "text/html"],
    ];
    if (e.request.url.includes("with-coop"))
        headers.push(["Cross-Origin-Opener-Policy", "same-origin"]);

    e.respondWith(new Response(text, { headers }));
}
