// RUN: %metal main 2>&1 | %check

@compute @workgroup_size(1)
fn main() {
  // ⚠️ -- the emoji ensures the file will be parsed as uchar
  // CHECK-L: 1.1754943508222875e-38
  let f32min = 0x1p-126;
  // CHECK-L: 3.4028234663852886e+38
  let f32max = 0x1.fffffep+127;
}
