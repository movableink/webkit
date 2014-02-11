description(
"This tests that (x|0) case is handled properly by the DFG."
);

var result = 0;
for (var i = 0; i < 15000; ++i) {
    if ((result|0) !== result) {
        testFailed("Should not be reached (result|0) !== result) at iteration " + i);
    }
}

