#include "types.h"
#include "kernel.hh"
#include "mmu.h"
#include "amd64.h"
#include "spinlock.h"
#include "condvar.h"
#include "queue.h"
#include "proc.hh"
#include "cpu.hh"
#include "bits.hh"
#include "kmtrace.hh"
#include "sched.hh"
#include "kalloc.h"
#include "vm.hh"
#include "ns.hh"

u64
proc_hash(const u32 &p)
{
  return p;
}

//int __mpalign__ idle[NCPU];
xns<u32, proc*, proc_hash> *xnspid __mpalign__;
static struct proc *bootproc __mpalign__;

#if MTRACE
struct kstack_tag kstack_tag[NCPU];
#endif

enum { sched_debug = 0 };

void
sched(void)
{
  int intena;

#if SPINLOCK_DEBUG
  if(!holding(&myproc()->lock))
    panic("sched proc->lock");
#endif
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(get_proc_state(myproc()) == RUNNING)
    panic("sched running");
  if(readrflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  myproc()->curcycles += rdtsc() - myproc()->tsc;
  if (get_proc_state(myproc()) == ZOMBIE)
    mtstop(myproc());
  else
    mtpause(myproc());
  mtign();

  swtch(&myproc()->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&myproc()->lock);  //DOC: yieldlock
  set_proc_state(myproc(), RUNNABLE);
  sched();
  release(&myproc()->lock);
}


// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  // Still holding proc->lock from scheduler.
  release(&myproc()->lock);

  // Just for the first process. can't do it earlier
  // b/c file system code needs a process context
  // in which to call cv_sleep().
  if(myproc()->cwd == NULL) {
    mtstart(forkret, myproc());
    myproc()->cwd = namei("/");
    mtstop(myproc());
  }

  // Return to "caller", actually trapret (see allocproc).
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *p, *np;
  int fd;
  int wakeupinit;

  if(myproc() == bootproc)
    panic("init exiting");

  // Close all open files.
  for(fd = 0; fd < NOFILE; fd++){
    if(myproc()->ofile[fd]){
      fileclose(myproc()->ofile[fd]);
      myproc()->ofile[fd] = 0;
    }
  }

  // Kernel threads might not have a cwd
  if (myproc()->cwd != NULL) {
      iput(myproc()->cwd);
      myproc()->cwd = 0;
  }

  // Pass abandoned children to init.
  wakeupinit = 0;
  SLIST_FOREACH_SAFE(p, &(myproc()->childq), child_next, np) {
    acquire(&p->lock);
    p->parent = bootproc;
    if(get_proc_state(p) == ZOMBIE)
      wakeupinit = 1;
    SLIST_REMOVE(&(myproc()->childq), p, proc, child_next);
    release(&p->lock);

    acquire(&bootproc->lock);
    SLIST_INSERT_HEAD(&bootproc->childq, p, child_next);
    release(&bootproc->lock);
  }

  // Parent might be sleeping in wait().
  acquire(&(myproc()->lock));
  // Kernel threads might not have a parent
  if (myproc()->parent != NULL)
      cv_wakeup(&(myproc()->parent->cv));
  if (wakeupinit)
    cv_wakeup(&bootproc->cv); 

  // Jump into the scheduler, never to return.
  set_proc_state(myproc(), ZOMBIE);
  sched();
  panic("zombie exit");
}

class delayedfree : public rcu_freed {
 private:
  proc *_p;

 public:
  delayedfree(proc *p) : rcu_freed("delayed proc free"), _p(p) {}
  virtual void do_gc() {
    kmfree(_p);
    delete this;
  }
};

static void
freeproc(struct proc *p)
{
  destroylock(&p->lock);

  delayedfree *df = new delayedfree(p);
  gc_delayed(df);
}

// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  p = (proc*) kmalloc(sizeof(struct proc));
  if (p == 0) return 0;
  memset(p, 0, sizeof(*p));

  set_proc_state(p, EMBRYO);
  p->pid = xnspid->allockey();
  p->cpuid = mycpu()->id;
  p->on_runq = -1;
  p->cpu_pin = 0;
  initprocgc(p);
#if MTRACE
  p->mtrace_stacks.curr = -1;
#endif

  snprintf(p->lockname, sizeof(p->lockname), "cv:proc:%d", p->pid);
  initlock(&p->lock, p->lockname+3, LOCKSTAT_PROC);
  initcondvar(&p->cv, p->lockname);
  initcilkframe(&p->cilkframe);

  if (xnspid->insert(p->pid, p) < 0)
    panic("allocproc: ns_insert");

  // Allocate kernel stack if possible.
  if((p->kstack = (char*) ksalloc(slab_stack)) == 0){
    if (!xnspid->remove(p->pid, &p))
      panic("allocproc: ns_remove");
    freeproc(p);
    return 0;
  }

  sp = p->kstack + KSTACKSIZE;
  
  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // amd64 ABI mandates sp % 16 == 0 before a call instruction
  // (or after executing a ret instruction)
  if ((uptr) sp % 16)
    panic("allocproc: misaligned sp");

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 8;
  *(u64*)sp = (u64)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->rip = (uptr)forkret;

  return p;
}

// Set up first user process.
void
inituser(void)
{
  struct proc *p;
  extern u8 _initcode_start[];
  extern u64 _initcode_size;

  p = allocproc();
  bootproc = p;
  if((p->vmap = new vmap()) == 0)
    panic("userinit: out of vmaps?");
  vmnode *vmn =  new vmnode(PGROUNDUP(_initcode_size) / PGSIZE);
  if(vmn == 0)
    panic("userinit: vmn_allocpg");
  if(p->vmap->insert(vmn, 0, 1) < 0)
    panic("userinit: vmap_insert");
  if(p->vmap->copyout(0, _initcode_start, _initcode_size) < 0)
    panic("userinit: copyout");
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = UCSEG | 0x3;
  p->tf->ds = UDSEG | 0x3;
  p->tf->ss = p->tf->ds;
  p->tf->rflags = FL_IF;
  p->tf->rsp = PGSIZE;
  p->tf->rip = 0x0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = 0; // forkret will fix in the process's context
  acquire(&p->lock);
  addrun(p);
  release(&p->lock);
}

void
initproc(void)
{
  xnspid = new xns<u32, proc*, proc_hash>(false);
  if (xnspid == 0)
    panic("pinit");
}

// Grow/shrink current process's memory by n bytes.
// Growing may allocate vmas and physical memory,
// but avoids interfering with any existing vma.
// Assumes vmas around proc->brk are part of the growable heap.
// Shrinking just decreases proc->brk; doesn't deallocate.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  struct vmap *m = myproc()->vmap;
  auto curbrk = myproc()->brk;

  if(n < 0 && 0 - n <= curbrk){
    myproc()->brk += n;
    return 0;
  }

  if(n < 0 || n > USERTOP || curbrk + n > USERTOP)
    return -1;

  // look one page ahead, to check if the newly allocated region would
  // abut the next-higher vma? we can't allow that, since then a future
  // sbrk() would start to use the next region (e.g. the stack).
  uptr newstart = PGROUNDUP(curbrk);
  s64 newn = PGROUNDUP(n + curbrk - newstart);
  range *prev = 0;
  auto span = m->cr.search_lock(newstart, newn + PGSIZE);
  for (range *r: span) {
    vma *e = (vma*) r;

    if (e->vma_start <= newstart) {
      if (e->vma_end >= newstart + newn) {
        myproc()->brk += n;
        switchuvm(myproc());
        return 0;
      }

      newn -= e->vma_end - newstart;
      newstart = e->vma_end;
      prev = e;
    } else {
      cprintf("growproc: overlap with existing mapping; brk %lx n %d\n",
              curbrk, n);
      return -1;
    }
  }

  vmnode *vmn = new vmnode(newn / PGSIZE);
  if(vmn == 0){
    cprintf("growproc: vmn_allocpg failed\n");
    return -1;
  }

  vma *repl = new vma(m, newstart, newstart+newn, PRIVATE, vmn);
  if (!repl) {
    cprintf("growproc: out of vma\n");
    delete vmn;
    return -1;
  }

  span.replace(prev, repl);

  myproc()->brk += n;
  switchuvm(myproc());
  return 0;
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p;

  p = xnspid->lookup(pid);
  if (p == 0) {
    panic("kill");
    return -1;
  }
  acquire(&p->lock);
  p->killed = 1;
  if(get_proc_state(p) == SLEEPING){
    // XXX
    // we need to wake p up if it is cv_sleep()ing.
    // can't change p from SLEEPING to RUNNABLE since that
    //   would make some condvar->waiters a dangling reference,
    //   and the non-zero p->cv_next will cause a future panic.
    // can't call cv_wakeup(p->oncv) since that results in
    //   deadlock (addrun() acquires p->lock).
    // can't release p->lock then call cv_wakeup() since the
    //   cv might be deallocated while we're using it
    //   (pipes dynamically allocate condvars).
  }
  release(&p->lock);
  return 0;
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdumpall(void)
{
  static const char *states[] = {
    /* [UNUSED]   = */ "unused",
    /* [EMBRYO]   = */ "embryo",
    /* [SLEEPING] = */ "sleep ",
    /* [RUNNABLE] = */ "runble",
    /* [RUNNING]  = */ "run   ",
    /* [ZOMBIE]   = */ "zombie"
  };
  const char *name = "(no name)";
  const char *state;
  uptr pc[10];

  for (proc *p : xnspid) {
    if(get_proc_state(p) >= 0 && get_proc_state(p) < NELEM(states) && 
       states[get_proc_state(p)])
      state = states[get_proc_state(p)];
    else
      state = "???";
    
    if (p->name && p->name[0] != 0)
      name = p->name;
    
    cprintf("\n%-3d %-10s %8s %2u  %lu\n",
            p->pid, name, state, p->cpuid, p->tsc);
    
    if(get_proc_state(p) == SLEEPING){
      getcallerpcs((void*)p->context->rbp, pc, NELEM(pc));
      for(int i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %lx\n", pc[i]);
    }
  }
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(int flags)
{
  int i, pid;
  struct proc *np;
  int cow = 1;

  // cprintf("%d: fork\n", myproc()->pid);

  // Allocate process.
  if((np = allocproc()) == 0)
    return -1;

  if(flags == 0) {
    // Copy process state from p.
    if((np->vmap = myproc()->vmap->copy(cow)) == 0){
      ksfree(slab_stack, np->kstack);
      np->kstack = 0;
      set_proc_state(np, UNUSED);
      if (!xnspid->remove(np->pid, &np))
	panic("fork: ns_remove");
      freeproc(np);
      return -1;
    }
  } else {
    np->vmap = myproc()->vmap;
    np->vmap->ref++;
  }

  np->brk = myproc()->brk;
  np->parent = myproc();
  *np->tf = *myproc()->tf;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->rax = 0;

  for(i = 0; i < NOFILE; i++)
    if(myproc()->ofile[i])
      np->ofile[i] = filedup(myproc()->ofile[i]);
  np->cwd = idup(myproc()->cwd);
  pid = np->pid;
  safestrcpy(np->name, myproc()->name, sizeof(myproc()->name));
  acquire(&myproc()->lock);
  SLIST_INSERT_HEAD(&myproc()->childq, np, child_next);
  release(&myproc()->lock);

#if 0
  if (migrate(np)) {
    acquire(&np->lock);
    np->cpuid = mycpu()->id;
    addrun(np);
    release(&np->lock);
  }
#else
  acquire(&np->lock);
  np->cpuid = mycpu()->id;
  addrun(np);
  release(&np->lock);
#endif

  //  cprintf("%d: fork done (pid %d)\n", myproc()->pid, pid);
  return pid;
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p, *np;
  int havekids, pid;

  for(;;){
    // Scan children for ZOMBIEs
    havekids = 0;
    acquire(&myproc()->lock);
    SLIST_FOREACH_SAFE(p, &myproc()->childq, child_next, np) {
	havekids = 1;
	acquire(&p->lock);
	if(get_proc_state(p) == ZOMBIE){
	  release(&p->lock);	// noone else better be trying to lock p
	  pid = p->pid;
	  SLIST_REMOVE(&myproc()->childq, p, proc, child_next);
	  release(&myproc()->lock);
	  ksfree(slab_stack, p->kstack);
	  p->kstack = 0;
	  p->vmap->decref();
	  set_proc_state(p, UNUSED);
	  if (!xnspid->remove(p->pid, &p))
	    panic("wait: ns_remove");
	  p->pid = 0;
	  p->parent = 0;
	  p->name[0] = 0;
	  p->killed = 0;
          freeproc(p);
	  return pid;
	}
	release(&p->lock);
    }

    // No point waiting if we don't have any children.
    if(!havekids || myproc()->killed){
      release(&myproc()->lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    cv_sleep(&myproc()->cv, &myproc()->lock);  

    release(&myproc()->lock);
  }
}

void
threadhelper(void (*fn)(void *), void *arg)
{
  release(&myproc()->lock); // initially held by scheduler
  mtstart(fn, myproc());
  fn(arg);
  exit();
}

struct proc*
threadalloc(void (*fn)(void *), void *arg)
{
  struct proc *p;

  p = allocproc();
  if (p == NULL)
    return 0;
  
  p->vmap = new vmap();
  if (p->vmap == NULL) {
    freeproc(p);
    return 0;
  }

  p->context->rip = (u64)threadstub;
  p->context->r12 = (u64)fn;
  p->context->r13 = (u64)arg;
  p->parent = myproc();
  p->cwd = 0;
  return p;
}

void
threadpin(void (*fn)(void*), void *arg, const char *name, int cpu)
{
  struct proc *p;

  p = threadalloc(fn, arg);
  if (p == NULL)
    panic("threadpin: alloc");

  snprintf(p->name, sizeof(p->name), "%s", name);
  p->cpuid = cpu;
  p->cpu_pin = 1;
  acquire(&p->lock);
  set_proc_state(p, RUNNABLE);
  addrun(p);
  release(&p->lock);
}
