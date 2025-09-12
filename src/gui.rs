use eframe::egui;
use std::sync::{Arc, Mutex};

use crate::uart::{
    packet::TelemetryData,
    throttle::{Commands, ThrottlePacket},
};

pub struct TelemetryApp {
    pub telemetry_data: Arc<Mutex<TelemetryData>>,
    pub throttle_packet: Arc<Mutex<ThrottlePacket>>,

    throttle_value: u16,
    is_armed: bool,
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
            is_armed: false,
        }
    }

    fn update_throttle_packet(&mut self) {
        if let Ok(mut throttle_guard) = self.throttle_packet.try_lock() {
            throttle_guard.set_throttle(self.throttle_value);

            if self.is_armed {
                throttle_guard.arm_status = Commands::Arm;
            } else {
                throttle_guard.arm_status = Commands::Disarm;
            }
        }
    }
}

impl eframe::App for TelemetryApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        ctx.request_repaint();

        egui::CentralPanel::default().show(ctx, |ui| {
            ui.heading("Telemetry Control Panel");
            ui.separator();

            ui.horizontal(|ui| {
                ui.vertical(|ui| {
                    ui.set_width(600.0);
                    ui.heading("Control Panel");
                    ui.separator();

                    ui.horizontal(|ui| {
                        if ui
                            .button(if self.is_armed { "DISARM" } else { "ARM" })
                            .clicked()
                        {
                            self.is_armed = !self.is_armed;
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
                        .add(egui::Button::new("Reset Microcontroller").fill(egui::Color32::RED))
                        .clicked()
                    {
                        self.throttle_value = 0;
                        self.is_armed = false;
                        self.update_throttle_packet();
                    }
                });

                ui.separator();

                ui.vertical(|ui| {
                    ui.heading("Live Telemetry");
                    ui.separator();

                    if let Ok(tele_guard) = self.telemetry_data.try_lock() {
                        egui::Grid::new("telemetry_grid")
                            .num_columns(2)
                            .spacing([80.0, 8.0])
                            .striped(true)
                            .show(ui, |ui| {
                                ui.label("ADC1 CH1:");
                                ui.label(format!("{:.3}", tele_guard.adc1_ch1));
                                ui.end_row();

                                ui.label("ADC1 CH2:");
                                ui.label(format!("{:.3}", tele_guard.adc1_ch2));
                                ui.end_row();

                                ui.label("ADC1 CH3:");
                                ui.label(format!("{:.3}", tele_guard.adc1_ch3));
                                ui.end_row();

                                ui.label("ADC2:");
                                ui.label(format!("{:.3}", tele_guard.adc2));
                                ui.end_row();

                                ui.label("ADC3:");
                                ui.label(format!("{:.3}", tele_guard.adc3));
                                ui.end_row();
                            });
                    } else {
                        ui.label("No telemetry data available");
                    }
                });
            });
        });
    }
}
