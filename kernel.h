#include "mmu.h"

#define KBASE 0xFFFFFFFF80000000ull
#define PBASE 0xFFFFFF0000000000ull
// XXX(sbw) arbitrary for right now..
#define USERTOP 0x0000000040000000ull

#define KCSEG (2<<3)  /* kernel code segment */
#define KDSEG (3<<3)  /* kernel data segment */

#define NULL ((void *)0)

static inline uptr v2p(void *a) { return (uptr) a  - KBASE; }
static inline void *p2v(uptr a) { return (void *) a + KBASE; }

#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

#define cmpswap(ptr, old, new) __sync_bool_compare_and_swap(ptr, old, new)
#define subfetch(ptr, val)     __sync_sub_and_fetch(ptr, val)

struct spinlock;
struct condvar;
struct context;
struct vmnode;
struct inode;
struct file;
struct stat;
struct proc;
struct vmap;
struct pipe;

// bio.c
void            binit(void);
struct buf*     bread(u32, u64, int writer);
void            brelse(struct buf*, int writer);
void            bwrite(struct buf*);

// cga.c
void            cgaputc(int c);

// condvar.c
void            initcondvar(struct condvar *, char *);
void            cv_sleep(struct condvar *cv, struct spinlock*);
void            cv_wakeup(struct condvar *cv);

// console.c
void            cprintf(const char*, ...);
void            panic(const char*) __attribute__((noreturn));
void            snprintf(char *buf, u32 n, char *fmt, ...);
void            consoleintr(int(*)(void));

// exec.c
int             exec(char*, char**);

// file.c
struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, char*, int n);
int             filestat(struct file*, struct stat*);
int             filewrite(struct file*, char*, int n);

// fs.c
int             namecmp(const char*, const char*);
struct inode*   dirlookup(struct inode*, char*);
struct inode*   ialloc(u32, short);
struct inode*   namei(char*);
void            iput(struct inode*);
struct inode*   iget(u32 dev, u32 inum);
void            ilock(struct inode*, int writer);
void            iunlockput(struct inode*);
void            iupdate(struct inode*);
void            iunlock(struct inode*);
int             readi(struct inode*, char*, u32, u32);
void            stati(struct inode*, struct stat*);
int             writei(struct inode*, char*, u32, u32);
struct inode*   idup(struct inode*);
struct inode*   nameiparent(char*, char*);
int             dirlink(struct inode*, char*, u32);
void            dir_init(struct inode *dp);
void	        dir_flush(struct inode *dp);

// ide.c
void            ideinit(void);
void            ideintr(void);
void            iderw(struct buf*);

// ioapic.c
void            ioapicenable(int irq, int cpu);

// kalloc.c
char*           kalloc(void);
char*           ksalloc(void);
void            kfree(void *);
void*           kmalloc(u64);
void            kmfree(void*);

// kbd.c
void            kbdintr(void);

// lapic.c
int             cpunum(void);
void            lapicstartap(u8, u32 addr);
void            lapiceoi(void);
void            lapic_tlbflush(u32);

// mp.c
extern int      ncpu;

// ns.c
enum {
  nskey_int = 1,
  nskey_ii,
  nskey_str,
  nskey_dirname,
  nskey_iis
};

struct nskey {
  int type;
  union {
    u64 i;
    struct {
      u64 a;
      u64 b;
    } ii;
    char *s;
    char *dirname;
    struct {
      u64 a;
      u64 b;
      char *s;
    } iis;
  } u;
};

#define KI(v)	    (struct nskey){.type=nskey_int,.u.i=v}
#define KII(x,y)    (struct nskey){.type=nskey_ii,.u.ii.a=x,.u.ii.b=y}
#define KS(v)	    (struct nskey){.type=nskey_str,.u.s=v}
#define KD(v)	    (struct nskey){.type=nskey_dirname,.u.dirname=v}
#define KIIS(x,y,z) (struct nskey){.type=nskey_iis,.u.iis.a=x, \
						   .u.iis.b=y, \
						   .u.iis.s=z}

void            nsinit(void);
struct ns*      nsalloc(int allowdup);
void		nsfree(struct ns*);
u64             ns_allockey(struct ns*);
int             ns_insert(struct ns*, struct nskey key, void*);
void*           ns_lookup(struct ns*, struct nskey key);
void*           ns_remove(struct ns *ns, struct nskey key, void *val); // removed val
void*           ns_enumerate(struct ns *ns, void *(*f)(void *, void *, void *), void *arg);
void*           ns_enumerate_key(struct ns *ns, struct nskey key, void *(*f)(void *, void *), void *arg);

// picirq.c
void            picenable(int);
void            piceoi(void);

// pipe.c
int             pipealloc(struct file**, struct file**);
void            pipeclose(struct pipe*, int);
int             piperead(struct pipe*, char*, int);
int             pipewrite(struct pipe*, char*, int);

// proc.c
void            addrun(struct proc *);
struct proc*    copyproc(struct proc*);
void            exit(void);
int             fork(int);
int             growproc(int);
int             kill(int);
void            pinit(void);
void            procdumpall(void);
void            scheduler(void) __attribute__((noreturn));
void            sched(void);
void            userinit(void);
int             wait(void);
void            yield(void);
void            migrate(struct proc *);

// rcu.c
void            rcuinit(void);
void            rcu_begin_write(struct spinlock *);
void            rcu_end_write(struct spinlock *);
void            rcu_begin_read(void);
void            rcu_end_read(void);
void            rcu_delayed(void*, void (*dofree)(void*));
void            rcu_delayed2(int, u64, void (*dofree)(int, u64));
void		rcu_gc(void);
void		rcu_gc_worker(void);

// spinlock.c
void            acquire(struct spinlock*);
void            getcallerpcs(void*, uptr*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, char*);
void            release(struct spinlock*);
void            pushcli(void);
void            popcli(void);

// syscall.c
int             argint64(int, u64*);
int             argint32(int n, int *ip);
int             argptr(int, char**, int);
int             argstr(int, char**);
int             fetchint64(uptr, u64*);
int             fetchstr(uptr, char**);
void            syscall(void);

// string.c
int             memcmp(const void*, const void*, u32);
void*           memmove(void*, const void*, u32);
void*           memset(void*, int, u32);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, u32);
char*           strncpy(char*, const char*, int);
int             strcmp(const char *p, const char *q);

// swtch.S
void            swtch(struct context**, struct context*);

// trap.c
extern struct segdesc bootgdt[NSEGS];
extern u64     ticks;
extern struct spinlock tickslock;
extern struct condvar cv_ticks;

// uart.c
void            uartputc(char c);
void            uartintr(void);

// vm.c
struct vmap *   vmap_alloc(void);
struct vmnode*  vmn_alloc(u64, u32);
struct vmnode*  vmn_allocpg(u64);
int             vmap_insert(struct vmap*, struct vmnode *, uptr);
struct vma *    vmap_lookup(struct vmap*, uptr, uptr);
int             copyout(struct vmap *, uptr, void*, u64);
void            vmn_free(struct vmnode *);
void            switchuvm(struct proc*);
void            switchkvm(void);
int             pagefault(struct vmap *, uptr, u32);
void            vmap_decref(struct vmap *);
int             vmn_load(struct vmnode *, struct inode*, u64, u64);
int             vmap_remove(struct vmap *, uptr, u64);
void            updatepages(pml4e_t*, void*, void*, int);
struct vmap *   vmap_copy(struct vmap *, int);