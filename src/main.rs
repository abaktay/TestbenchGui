use std::{
    fs::File,
    io::{self, Write},
    sync::{Arc, Mutex},
    thread,
    time::Duration,
};

use eframe::egui;
use serialport;
use uart::{file::*, packet::TelemetryData, throttle::Commands, throttle::ThrottlePacket};

mod gui;
mod uart;

use gui::TelemetryApp;

const RX_SIZE: usize = 42;

fn main() {
    println!("Enter the device address: ");
    let mut port_name = String::new();
    io::stdin()
        .read_line(&mut port_name)
        .expect("Failed to read line");

    // println!("{}", port_name);
    let port = match serialport::new(port_name.trim(), 115200)
        .timeout(Duration::from_millis(500))
        .open()
    {
        Ok(port) => Arc::new(Mutex::new(port)),
        Err(e) => {
            eprintln!("ERROR: {}", e);
            return;
        }
    };
    let dir = path_generator().unwrap();

    println!("{}", dir);
    let mut fd: File = csv_starter(dir).unwrap();

    let telemetry = TelemetryData::new();
    let telemetry_arc = Arc::new(Mutex::new(telemetry));

    let throttle_packet = ThrottlePacket::new(Commands::Set);
    let throttle_arc = Arc::new(Mutex::new(throttle_packet));

    /*
        RX THREAD SECTION
    */
    let port_rx = Arc::clone(&port);
    let tele_rx = Arc::clone(&telemetry_arc);
    let rx_handle = thread::spawn(move || {
        let mut buf = [0u8; RX_SIZE]; // make it const

        let mut ch1 = 0.0;
        let mut ch2 = 0.0;
        let mut ch3 = 0.0;

        loop {
            // First acquires the lock of the port for receiving input
            // Then acquires the lock of TelemetryData to update it
            if let Ok(mut port_guard) = port_rx.lock() {
                match port_guard.read_exact(buf.as_mut_slice()) {
                    Ok(_) => {
                        // Updates TelemetryData
                        if let Ok(mut guard) = tele_rx.lock() {
                            guard.process(buf);

                            ch1 = guard.adc1_ch1;
                            ch2 = guard.adc1_ch2;
                            ch3 = guard.adc1_ch3;
                        }
                    }
                    Err(e) => {
                        eprintln!("RX Error occurred {}", e);
                    }
                }
            }
            writeln!(fd, "{:<10.2},{:<10.2},{:<10.2}", ch1, ch2, ch3,).unwrap();

            thread::sleep(Duration::from_millis(5));
        }
    });

    /*
        TX THREAD SECTION
    */
    let port_tx = Arc::clone(&port);
    let throttle_tx = Arc::clone(&throttle_arc);
    let tx_handle = thread::spawn(move || {
        loop {
            // First acquires the lock of TX port
            if let Ok(mut port_guard) = port_tx.lock() {
                if let Ok(tx_guard) = throttle_tx.lock() {
                    // Sends the packet after acquiring throttle packet
                    match port_guard.write(tx_guard.prep_packet().as_slice()) {
                        Ok(_) => {
                            match tx_guard {
                                _ => {}
                            }
                            let _ = port_guard.flush();
                        }
                        Err(e) => {
                            eprintln!("TX Error occurred {}", e);
                        }
                    };
                }
            }
            thread::sleep(Duration::from_millis(10));
        }
    });

    /*
        GUI THREAD SECTION
    */

    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([1200.0, 800.0])
            .with_title("TESTBENCH"),
        ..Default::default()
    };

    let app = TelemetryApp::new(Arc::clone(&telemetry_arc), Arc::clone(&throttle_arc));

    if let Err(e) = eframe::run_native(
        "Telemetry Control Panel",
        options,
        Box::new(|_cc| Box::new(app)),
    ) {
        eprintln!("GUI Error: {}", e);
    }

    if tx_handle.is_finished() {
        let _ = tx_handle.join();
    }

    if rx_handle.is_finished() {
        let _ = rx_handle.join();
    }
}
