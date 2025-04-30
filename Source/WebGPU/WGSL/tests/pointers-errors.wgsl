// RUN: %not %wgslc | %check

// CHECK-L: only pointers in <storage> address space may specify an access mode
fn f1(x: ptr<function, i32, read>) { }
