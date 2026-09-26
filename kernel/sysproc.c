#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"
#include "procinfo.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0) {
      return -1;
    }
  } else {
    // Lazily allocate memory for this process: increase its memory
    // size but don't allocate memory. If the processes uses the
    // memory, vmfault() will allocate it.
    if (addr + n < addr)
      return -1;
    if (addr + n > TRAPFRAME)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
      release(&tickslock);
      return -1;
    }
    sleep_prepare(&ticks);
    release(&tickslock);
    sleep();
    acquire(&tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kkill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_ps_listinfo(void)
{
  uint64 uaddr;
  int lim;

  argaddr(0, &uaddr);
  argint(1, &lim);
  if (uaddr == 0 || lim < 0) {
    return -1;
  }

  extern struct proc proc[NPROC];
  extern struct spinlock wait_lock;

  int count = 0;

  for (int i = 0; i < NPROC; i++) {
    struct proc *p = &proc[i];

    acquire(&p->lock);
    if (p->state == UNUSED || p->state == USED) {
      release(&p->lock);
      continue;
    }

    struct procinfo pi;
    pi.pid = p->pid;
    pi.state = p->state;

    for (int j = 0; j < 16; j++) {
      pi.name[j] = p->name[j];
      if (p->name[j] == '\0')
        break;
    }

    release(&p->lock);

    acquire(&wait_lock);
    struct proc *parent = p->parent;
    if (parent) {
      acquire(&parent->lock);
      pi.ppid = parent->pid;
      release(&parent->lock);
    } else {
      pi.ppid = 0;
    }
    release(&wait_lock);

    if (count < lim) {
      uint64 dst = uaddr + count * sizeof(struct procinfo);

      // ИСПРАВЛЕНО: Ровно 5 аргументов в правильном для вашей ОС порядке типов данных
      if (copyout(myproc()->pagetable, dst, (uint64)&pi, (char *)&pi,
                  sizeof(struct procinfo)) < 0) {
        printk("asdasdasd");
        return -1;
      }
    }

    count++;
  }

  return count;
}
