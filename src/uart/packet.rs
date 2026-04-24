// Packet to receive
// Size: 32 bytes
// [0] => status
// rest are packed floats

#[derive(Clone, Debug)]
pub struct TelemetryData {
    // Testbench
    pub adc1_ch1: f32,
    pub adc1_ch2: f32,
    pub adc1_ch3: f32,
    pub adc2: f32,
    pub adc3: f32,
    pub cksum: u16,

    // Powerboard
    pub voltage: f32,
    pub current: f32,
    pub charge: f32,
    pub timestamp: u32,
    pub status: u32,
}

impl TelemetryData {
    pub fn new() -> Self {
        let data = Self {
            adc1_ch1: 0.0,
            adc1_ch2: 0.0,
            adc1_ch3: 0.0,
            adc2: 0.0,
            adc3: 0.0,
            cksum: 0,

            voltage: 0.0,
            current: 0.0,
            charge: 0.0,
            timestamp: 0,
            status: 0,
        };
        data
    }

    // Size of the buffer will change
    pub fn process(&mut self, input: [u8; 42]) {
        self.adc1_ch1 = f32::from_le_bytes(input[1..5].try_into().unwrap());
        self.adc1_ch2 = f32::from_le_bytes(input[5..9].try_into().unwrap());
        self.adc1_ch3 = f32::from_le_bytes(input[9..13].try_into().unwrap());
        self.adc2 = f32::from_le_bytes(input[13..17].try_into().unwrap());
        self.adc3 = f32::from_le_bytes(input[17..21].try_into().unwrap());

        self.voltage = f32::from_le_bytes(input[21..25].try_into().unwrap());
        self.current = f32::from_le_bytes(input[25..29].try_into().unwrap());
        self.charge = f32::from_le_bytes(input[29..33].try_into().unwrap());
        self.timestamp = u32::from_le_bytes(input[33..37].try_into().unwrap());
        self.status = u32::from_le_bytes(input[37..41].try_into().unwrap());
        // println!("DEBUG {}", self.adc1_ch1);
    }
}
