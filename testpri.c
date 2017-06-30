#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(int argc, char *argv[]) 
{
  int pid;
  int i;
  if ((pid = fork()) == 0) { // child
    pid = getpid(); 
    for(i = 0; i < 1000; i++)
      printf(1, "child\t%d\t%d\n", pid, getscheduler(pid));
    // setscheduler(pid, 1000);
  } else { // parent
    int ppid = getpid();
    setscheduler(ppid, 100);
    for(i = 0; i < 1000; i++) 
      printf(1, "parent\t%d\t%d\n", ppid, getscheduler(ppid));
    wait();
  }
  exit();
}