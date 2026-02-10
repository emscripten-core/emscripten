#[no_mangle]
pub extern "C" fn say_hello() {
  println!("Hello from rust!");
}
