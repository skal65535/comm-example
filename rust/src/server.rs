use std::thread;
use std::net::{TcpListener, TcpStream, Shutdown};
use std::io::{Read, Write};

fn handle_client(mut stream: TcpStream) {
  let mut data = [0 as u8; 50];
  while match stream.read(&mut data) {
    Ok(size) => {
      stream.write(&data[0..size]).unwrap();  // send back msg
      true
    },
    Err(_) => {
      println!("An error occurred, terminating connection with {}", stream.peer_addr().unwrap());
      stream.shutdown(Shutdown::Both).unwrap();
      false
    }
  } {}
}

fn main() {
  let listener = TcpListener::bind("0.0.0.0:3333").unwrap();
  // accept connections and process them, spawning a new thread for each one
  println!("Server listening on port 3333");
  for stream in listener.incoming() {
    match stream {
      Ok(stream) => {
        println!("New connection: {}", stream.peer_addr().unwrap());
        thread::spawn(move || { handle_client(stream) });
      },
      Err(e) => {
        println!("connection failed with error: {}", e);
      }
    }
  }
  drop(listener);  // close server socket
}
