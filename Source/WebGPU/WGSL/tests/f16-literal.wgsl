// RUN: %not %wgslc | %check

fn main() -> f16 {
    // CHECK-L: f16 literal used without f16 extension enabled
    var x = 1h;
}
