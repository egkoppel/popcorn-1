use core::fmt::{self, Write};
use spin::Mutex;

#[repr(C)]
struct File {
	fd: u32
}

extern "C" {
	static stdin: *const File;
	static stdout: *const File;
	static stderr: *const File;
	static stdserial: *const File;

	fn term_clear();
	fn kfputc(f: *const File, byte: u8);
}

pub fn clear() {
	unsafe { term_clear(); }
}

pub struct Writer {
	fd: *const File
}

unsafe impl Sync for Writer {}
unsafe impl Send for Writer {}

use lazy_static::lazy_static;

lazy_static! {
	static ref STDOUT: Mutex<Writer> = Mutex::new(Writer {
		fd: unsafe { stdout }
	});
	static ref STDERR: Mutex<Writer> = Mutex::new(Writer {
		fd: unsafe { stderr }
	});
	static ref STDSERIAL: Mutex<Writer> = Mutex::new(Writer {
		fd: unsafe { stdserial }
	});
}

pub enum Color {
	RESET,
	BLACK,
	RED,
	GREEN,
	YELLOW,
	BLUE,
	MAGENTA,
	CYAN,
	WHITE
}

impl Color {
	pub fn to_str(&self) -> &str {
		match *self {
			Self::RESET => "\x1b[0m",
			Self::BLACK => "\x1b[30m",
			Self::RED => "\x1b[31m",
			Self::GREEN => "\x1b[32m",
			Self::YELLOW => "\x1b[33m",
			Self::BLUE => "\x1b[34m",
			Self::MAGENTA => "\x1b[35m",
			Self::CYAN => "\x1b[36m",
			Self::WHITE => "\x1b[37m"
		}
	}
}

impl Writer {
	pub fn write_byte(&mut self, byte: u8) {
		unsafe { kfputc(self.fd, byte); }
	}

	pub fn write_string(&mut self, s: &str) {
		for byte in s.bytes() {
			self.write_byte(byte);
		}
	}
}

impl fmt::Write for Writer {
	fn write_str(&mut self, s: &str) -> fmt::Result {
		self.write_string(s);
		Ok(())
	}
}

#[macro_export]
macro_rules! println {
	() => (print!("\n"));
	($($arg:tt)*) => (print!("{}\n", format_args!($($arg)*)));
}

#[macro_export]
macro_rules! print {
	($($arg:tt)*) => ($crate::stdio::_print(format_args!($($arg)*)));
}

#[macro_export]
macro_rules! eprintln {
	() => (eprint!("\n"));
	($($arg:tt)*) => (eprint!("{}\n", format_args!($($arg)*)));
}

#[macro_export]
macro_rules! eprint {
	($($arg:tt)*) => ($crate::stdio::_eprint(format_args!($($arg)*)));
}

#[macro_export]
macro_rules! serialprintln {
	() => (eprint!("\n"));
	($($arg:tt)*) => (serialprint!("{}\n", format_args!($($arg)*)));
}

#[macro_export]
macro_rules! serialprint {
	($($arg:tt)*) => ($crate::stdio::_serialprint(format_args!($($arg)*)));
}

pub fn _print(args: fmt::Arguments) {
	STDOUT.lock().write_fmt(args).unwrap();
}

pub fn _eprint(args: fmt::Arguments) {
	let mut out = STDERR.lock();
	out.write_string(Color::RED.to_str());
	out.write_fmt(args).unwrap();
	out.write_string(Color::RESET.to_str());
}

pub fn _serialprint(args: fmt::Arguments) {
	STDSERIAL.lock().write_fmt(args).unwrap();
}