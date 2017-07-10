#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[]) 
{
  int pid, i;
  if ((pid = fork()) == 0) { // child
    pid = getpid(); 

    for(i = 0; i < 30; i++)
      printf(1, "child\t%d\t%d\t%d\n", pid, getscheduler(pid), i);
  } else { // parent
    int ppid = getpid();

    setscheduler(ppid, 100);
    
    for(i = 0; i < 30; i++) 
      printf(1, "parent\t%d\t%d\t%d\n", ppid, getscheduler(ppid), i);

    wait();
  }
  exit();
}