#include "types.h"
#include "stat.h"
#include "user.h"

int 
main(int argc, char *argv[])
{
  int pri = -1;

  if (argc == 2) {
    pri = getscheduler(atoi(argv[1]));
  } else {
      printf(2, "usage: setscheduler pid\n");
      exit();
  }
  
  if (pri == -1) {
    printf(2, "fatal error\n");
  } else {
    printf(2, "priority : %d\n", pri);
  }
  exit();
}