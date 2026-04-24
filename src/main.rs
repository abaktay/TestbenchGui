use std::{
    fs::File,
    io::{self, Write},
    sync::{Arc, Mutex},
    thread,
    time::Duration,
};

use eframe::egui;
use serialport;
use uart::{packet::TelemetryData, throttle::Commands, throttle::ThrottlePacket};

mod gui;
mod uart;

use gui::TelemetryApp;

use crate::uart::file::{csv_starter, path_generator};

fn main() {
    println!("Enter the device address: ");
    let mut port_name = String::new();
    io::stdin()
        .read_line(&mut port_name)
        .expect("Failed to read line");

    let port_rx = serialport::new(port_name.trim(), 115200)
        .timeout(Duration::from_millis(500))
        .open()
        .unwrap();
    let mut port_tx = serialport::new(port_name.trim(), 115200)
        .timeout(Duration::from_millis(500))
        .open()
        .unwrap();

    // log files

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
    // let port_rx = Arc::clone(&port);
    let tele_rx = Arc::clone(&telemetry_arc);
    let rx_handle = thread::spawn(move || {
        let mut port_rx = port_rx;
        let mut buf = [0u8; 42];
        loop {
            match port_rx.read_exact(&mut buf) {
                Ok(_) => {
                    // port is not locked at all here, process freely
                    if let Ok(mut guard) = tele_rx.lock() {
                        guard.process(buf);
                        write!(
                        fd,
                        "{:<10.2},{:<10.2},{:<10.2},{:<10.2},{:<10.2},{:<10.2},{:<10.2},{:<10.2},{:<10}\n",
                        guard.adc1_ch1,
                        guard.adc1_ch2,
                        guard.adc1_ch3,
                        guard.adc2,
                        guard.adc3,
                        guard.voltage,
                        guard.current,
                        guard.charge,
                        guard.timestamp
                    )
                    .unwrap();
                    }
                }
                Err(e) => {
                    eprintln!("RX Error: {}", e);
                }
            }
            thread::sleep(Duration::from_millis(5));
        }
    });

    /*
        TX THREAD SECTION
    */
    let throttle_tx = Arc::clone(&throttle_arc);
    let tx_handle = thread::spawn(move || loop {
        if let Ok(tx_guard) = throttle_tx.lock() {
            match port_tx.write(tx_guard.prep_packet().as_slice()) {
                Ok(_) => {
                    let _ = port_tx.flush();
                }
                Err(e) => {
                    eprintln!("TX Error: {}", e);
                }
            }
        }
        thread::sleep(Duration::from_millis(10));
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
