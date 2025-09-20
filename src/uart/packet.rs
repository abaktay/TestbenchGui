// Packet to receive
// Size: 32 bytes
// [0] => status
// rest are packed floats

#[derive(Clone, Debug)]
pub struct TelemetryData {
    pub adc1_ch1: f32,
    pub adc1_ch2: f32,
    pub adc1_ch3: f32,
    pub adc2: f32,
    pub adc3: f32,
    pub cksum: u16,
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
        };
        data
    }

    // Size of the buffer will change
    pub fn process(
        &mut self,
        input: [u8; 30],
        mean: &mut Vec<f32>,
        mean2: &mut Vec<f32>,
        mean3: &mut Vec<f32>,
    ) {
        self.adc1_ch1 = f32::from_le_bytes(input[1..5].try_into().unwrap());
        self.adc1_ch2 = f32::from_le_bytes(input[5..9].try_into().unwrap());
        self.adc1_ch3 = f32::from_le_bytes(input[9..13].try_into().unwrap());
        self.adc2 = f32::from_le_bytes(input[13..17].try_into().unwrap());
        self.adc3 = f32::from_le_bytes(input[17..21].try_into().unwrap());
        // println!("DEBUG {}", self.adc1_ch1);
        mean.push(self.adc1_ch1);
        mean2.push(self.adc1_ch2);
        mean3.push(self.adc1_ch3);
    }
}
