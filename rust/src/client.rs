use std::net::{TcpStream};
use std::io::{Read, Write};
use std::str::from_utf8;

fn main() {
  match TcpStream::connect("localhost:3333") {
    Ok(mut stream) => {
      println!("Successfully connected to server in port 3333");

      let msg = b"Hello!";
      stream.write(msg).unwrap();
      println!("Sent '{}', awaiting reply...", String::from_utf8((&msg).to_vec()).unwrap());

      let mut data = [0 as u8; 6 /* = length(msg)*/];
      match stream.read_exact(&mut data) {
        Ok(_) => {
          if &data == msg {
            println!("Valid reply!");
          } else {
            let text = from_utf8(&data).unwrap();
            println!("Bad reply: {}", text);
          }
        },
        Err(e) => {
          println!("Data read failed with error: {}", e);
        }
      }
    },
    Err(e) => {
      println!("Connection failed with error: {}", e);
    }
  }
  println!("Terminated.");
}
