use wasm_bindgen::prelude::*;

#[wasm_bindgen]
pub struct Greeter {
    greeting: String,
}

#[wasm_bindgen]
impl Greeter {
    #[wasm_bindgen(constructor)]
    pub fn new(greeting: String) -> Greeter {
        Greeter { greeting }
    }

    pub fn greet(&self, name: String) -> String {
        format!("{}, {}!", self.greeting, name)
    }
}

fn main() {
    // Matches the emscripten idiom: main runs automatically on init.
    println!("main ran");
}
