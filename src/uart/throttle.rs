// Packet to send
pub struct ThrottlePacket {
    pub arm_status: Commands,
    pub throttle_power: u16,
    pub checksum: u16,
}

// List of commands
#[repr(u8)]
#[derive(Copy, Clone, PartialEq, Eq)]
pub enum Commands {
    Disarm = 0b00,
    Arm,
    Set,
    Reset,
}

impl ThrottlePacket {
    pub fn new(cmd: Commands) -> Self {
        let packet = Self {
            arm_status: Commands::Set,
            throttle_power: 0u16,
            checksum: 0u16,
        };

        packet
    }

    pub fn set_throttle(&mut self, power: u16) {
        self.throttle_power = power;
    }

    pub fn prep_packet(&self) -> [u8; 5] {
        let mut buf = [0u8; 5];
        buf[0] = self.arm_status as u8;
        buf[1] = (self.throttle_power & 0xFF) as u8;
        buf[2] = ((self.throttle_power << 8) & 0xFF) as u8;

        // buf[1] = 0xFF;
        // buf[2] = 0xFF;
        buf[3] = (self.throttle_power & 0xFF) as u8;
        buf[4] = ((self.throttle_power >> 8) & 0xFF) as u8; // TODO checksum
        buf
    }
}
