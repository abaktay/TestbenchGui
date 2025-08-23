use std::{
    io::{self, Write},
    sync::{Arc, Mutex},
    thread,
    time::Duration,
};

use serialport::*;

fn main() {
    match available_ports() {
        Ok(mut ports) => {
            ports.sort_by_key(|i| i.port_name.clone());

            match ports.len() {
                0 => println!("Found no ports."),
                1 => println!("Found 1 port."),
                n => println!("Found {} ports.", n),
            };

            for p in ports {
                println!("\t\t{}", p.port_name);
            }
        }
        Err(_) => {
            println!("zort");
        }
    }

    let port_name = "/dev/ttyACM0";

    let port = match serialport::new(port_name, 115200)
        .timeout(Duration::from_secs(5))
        .open()
    {
        Ok(port) => Arc::new(Mutex::new(port)),
        Err(e) => {
            eprintln!("ERROR: {}", e);
            return;
        }
    };

    let port_rx = Arc::clone(&port);
    let rx_handle = thread::spawn(move || {
        let mut buf = [0u8; 8];

        loop {
            if let Ok(mut port_guard) = port_rx.lock() {
                match port_guard.read_exact(buf.as_mut_slice()) {
                    Ok(_) => {
                        // println!("Received bytes: {} ", n);
                        io::stdout().write_all(&buf).unwrap();
                    }
                    Err(e) => {
                        eprintln!("Error occurred {}", e);
                    }
                };

                thread::sleep(Duration::from_millis(250));
            }
        }
    });

    let port_tx = Arc::clone(&port);
    let tx_handle = thread::spawn(move || {
        let to_send = [1u8; 5];

        loop {
            if let Ok(mut port_guard) = port_tx.lock() {
                println!("GOT THE LOCK");
                match port_guard.write(to_send.as_slice()) {
                    Ok(_) => {
                        // println!("Sent bytes: {} ", n);
                        println!("WOOOO");
                        let _ = port_guard.flush();
                    }
                    Err(e) => {
                        eprintln!("Error occurred {}", e);
                    }
                };
                thread::sleep(Duration::from_millis(250));
            }
        }
    });

    rx_handle.join().unwrap();
    tx_handle.join().unwrap();
}
