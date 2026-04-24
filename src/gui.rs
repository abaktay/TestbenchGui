use eframe::egui;
use std::sync::{Arc, Mutex};
use std::time::Duration;

use crate::uart::{
    packet::TelemetryData,
    throttle::{Commands, ThrottlePacket},
};

pub struct TelemetryApp {
    pub telemetry_data: Arc<Mutex<TelemetryData>>,
    pub throttle_packet: Arc<Mutex<ThrottlePacket>>,

    throttle_value: u16,
    status: Commands,
}

impl TelemetryApp {
    pub fn new(
        telemetry_data: Arc<Mutex<TelemetryData>>,
        throttle_packet: Arc<Mutex<ThrottlePacket>>,
    ) -> Self {
        Self {
            telemetry_data,
            throttle_packet,
            throttle_value: 0,
            status: Commands::Disarm,
        }
    }

    fn update_throttle_packet(&mut self) {
        if let Ok(mut throttle_guard) = self.throttle_packet.try_lock() {
            throttle_guard.set_throttle(self.throttle_value);
            println!("Throttle power is: {}", throttle_guard.throttle_power);

            match self.status {
                Commands::Set => throttle_guard.arm_status = Commands::Set,
                Commands::Arm => throttle_guard.arm_status = Commands::Arm,
                Commands::Disarm => throttle_guard.arm_status = Commands::Disarm,
                Commands::Reset => throttle_guard.arm_status = Commands::Reset,
            }
        }
    }
}

impl eframe::App for TelemetryApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        ctx.request_repaint_after(Duration::from_millis(16));
        ctx.pixels_per_point();

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("Telemetry Control Panel");
            ui.separator();

            ui.horizontal(|ui| {
                ui.vertical(|ui| {
                    ui.set_width(300.0);
                    ui.heading("Control Panel");
                    ui.separator();

                    ui.horizontal(|ui| {
                        if ui
                            .button(if self.status == Commands::Disarm {
                                "ARM"
                            } else {
                                "DISARM"
                            })
                            .clicked()
                        {
                            if self.status == Commands::Disarm {
                                self.status = Commands::Arm;
                            } else {
                                self.status = Commands::Disarm;
                            }
                            self.update_throttle_packet();
                        }
                    });

                    ui.separator();

                    ui.label("Set Throttle");
                    if ui
                        .add(egui::Slider::new(&mut self.throttle_value, 0..=100).text("Throttle"))
                        .changed()
                    {
                        self.update_throttle_packet();
                    }

                    ui.separator();

                    if ui
                        .add_sized(
                            [140., 40.],
                            egui::Button::new("Reset Microcontroller").fill(egui::Color32::RED),
                        )
                        .clicked()
                    {
                        self.throttle_value = 0;
                        self.status = Commands::Reset;
                        self.update_throttle_packet();
                    }
                });

                ui.separator();

                if let Ok(tele_guard) = self.telemetry_data.try_lock() {
                    ui.vertical(|ui| {
                        egui::Grid::new("telemetry_grid")
                            .num_columns(5)
                            .spacing([80.0, 8.0])
                            .striped(true)
                            .show(ui, |ui| {
                                ui.heading("Live Telemetry");
                                ui.end_row();

                                ui.label("THRUST X:");
                                ui.label(format!("{:.2}", tele_guard.adc1_ch1));
                                ui.end_row();

                                ui.label("THRUST Y:");
                                ui.label(format!("{:.2}", tele_guard.adc1_ch2));
                                ui.end_row();

                                ui.label("THRUST Z:");
                                ui.label(format!("{:.2}", tele_guard.adc1_ch3));
                                ui.end_row();

                                ui.label("ADC2:");
                                ui.label(format!("{:.2}", tele_guard.adc2));
                                ui.end_row();

                                ui.label("ADC3:");
                                ui.label(format!("{:.2}", tele_guard.adc3));
                                ui.end_row();

                                ui.heading("Powerboard");
                                ui.end_row();

                                ui.label("VOLTAGE:");
                                ui.label(format!("{:.2}", tele_guard.voltage));
                                ui.end_row();
                                ui.label("CURRENT:");
                                ui.label(format!("{:.2}", tele_guard.current));
                                ui.end_row();
                                ui.label("CHARGE:");
                                ui.label(format!("{:.2}", tele_guard.charge));
                                ui.end_row();
                                ui.label("TIMESTAMP:");
                                ui.label(format!("{}", tele_guard.timestamp));
                                ui.end_row();
                            });
                    });
                }
            });
        });
    }
}
