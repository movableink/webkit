// RUN: %not %wgslc | %check

// CHECK-L: f16 type used without f16 extension enabled
fn main() -> f16 {
    var x: f16;

    // CHECK-L: f16 type used without f16 extension enabled
    x = vec2h().x;

    // CHECK-L: f16 type used without f16 extension enabled
    x = vec3h().x;

    // CHECK-L: f16 type used without f16 extension enabled
    x = vec4h().x;

    // CHECK-L: f16 type used without f16 extension enabled
    x = mat2x2h()[0].x;

    return x;
}
