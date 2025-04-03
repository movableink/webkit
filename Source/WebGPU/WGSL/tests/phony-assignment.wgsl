// RUN: %not %wgslc | %check

@compute @workgroup_size(1)
fn main() {
  // CHECK-L: function 'f' does not return a value
  _ = f();
}

fn f() { }
