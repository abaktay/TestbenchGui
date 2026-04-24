## Requirements
- Rust (install from https://rust-lang.org/) 

## Running:
Open the directory of the project in terminal and run:

```
cargo run --release
```

It'll ask the address of UART connection.

**Linux:** `/dev/ttyUSB0` (the number may vary, e.g. `/dev/ttyUSB1`, or if using Nucleo board `/dev/ttyACMX`)
 
**Windows:** Check Device Manager for the correct COM port.

