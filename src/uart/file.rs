use super::packet::TelemetryData;
use std::{
    env::current_dir,
    fs::{create_dir, File, OpenOptions},
    io::{self, Write},
    path::Path,
};

pub fn csv_starter(file_path: String) -> Result<File, String> {
    let mut file = OpenOptions::new()
        .write(true)
        .create(true)
        .truncate(true)
        .open(file_path)
        .map_err(|e| e.to_string())?;

    writeln!(
        file,
        "{:<10} {:<10} {:<10} {:<10} {:<10} {:<10} {:<10} {:<10} {:<10}",
        "THROTTLE_X",
        "THROTTLE_Y",
        "THROTTLE_Z",
        "ADC2",
        "ADC3",
        "VOLTAGE",
        "CURRENT",
        "CHARGE",
        "TIMESTAMP"
    )
    .map_err(|e| e.to_string())?;

    Ok(file)
}

pub fn path_generator() -> Result<String, Box<dyn std::error::Error>> {
    let working_directory = current_dir()?;

    let mut count = 0;

    if cfg!(windows) {
        let fullpath = format!("{}\\test{}.csv", working_directory.display(), count);
        //println!("{fullpath}");
        while !Path::new(&fullpath).exists() {
            let fullpath = format!("{}\\test{}.csv", working_directory.display(), count);
            //println!("zap");
            count += 1;
        }
        Ok(fullpath)
    } else {
        let mut fullpath = format!("{}/test{}.csv", working_directory.display(), count);
        //println!("{fullpath}");
        while Path::new(&fullpath).exists() {
            count += 1;
            fullpath = format!("{}/test{}.csv", working_directory.display(), count);
        }
        Ok(fullpath)
    }
}
