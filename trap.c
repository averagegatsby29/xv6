#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
miniprogram(void){
  cprintf("found a divide by 0 error\n");
  exit();
}

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
  
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_DIVIDE:

    if(0){  // our code breaks if this isn't here ;_____;
      cprintf("ayy lmao");
    }

    if(proc->sig_handlers[SIGFPE] == 0){
      cprintf("No handler for current signal; killing current process\n");
      kill(proc->pid);
    }

    uint old_eip  = tf->eip;
    uint old_eax  = tf->eax;
    uint old_edx  = tf->edx;
    uint old_ecx  = tf->ecx;

    *((uint*)(proc->tf->esp-4)) = old_eip;
    *((uint*)(proc->tf->esp-8)) = old_eax;
    *((uint*)(proc->tf->esp-12)) = old_ecx;
    *((uint*)(proc->tf->esp-16)) = old_edx;
    *((uint*)(proc->tf->esp-20)) = SIGFPE;
    *((uint*)(proc->tf->esp-24)) = proc->trampoline_addr;
    //*((uint*)(proc->tf->esp-28)) = (uint)proc->pop;
    tf->esp = tf->esp-24;
    tf->eip = (uint)(proc->sig_handlers[SIGFPE]);

    break;




  case T_IRQ0 + IRQ_TIMER:
    if(cpu->id == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();

    // handle alarm

    if(proc && (tf->cs & 3) == 3 && proc->alarmed == ALRM_NOTACTIVATED){
      proc->currticks++;
      cprintf("just incremented currticks\n");
      if(proc->currticks >= proc->alarmticks){
        // send SIGALRM
        proc->currticks = 0;
        proc->alarmed = ALRM_ACTIVATED;
      }
    }



    break;

  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpu->id, tf->cs, tf->eip);
    lapiceoi();
    break;
   
  //PAGEBREAK: 13
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpu->id, tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpu->id, tf->eip, 
            rcr2());
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running 
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER && (tf->cs&3) == DPL_USER){
    yield();
    if(proc->alarmed == ALRM_ACTIVATED){ 


      if(proc->sig_handlers[SIGALRM] == 0){
        cprintf("No handler for current signal; killing current process\n");
        kill(proc->pid);
      }


        proc->alarmed = ALRM_DEAD;
          
      uint old_eip  = tf->eip;
      uint old_eax  = tf->eax;
      uint old_edx  = tf->edx;
      uint old_ecx  = tf->ecx;

      *((uint*)(proc->tf->esp-4)) = old_eip;
      *((uint*)(proc->tf->esp-8)) = old_eax;
      *((uint*)(proc->tf->esp-12)) = old_ecx;
      *((uint*)(proc->tf->esp-16)) = old_edx;
      *((uint*)(proc->tf->esp-20)) = SIGALRM;
      *((uint*)(proc->tf->esp-24)) = proc->trampoline_addr;
      //*((uint*)(proc->tf->esp-28)) = (uint)proc->pop;
      tf->esp = tf->esp-24;
      tf->eip = (uint)(proc->sig_handlers[SIGALRM]);

    }
  }
  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();
}
