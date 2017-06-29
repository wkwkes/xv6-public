#include "types.h"
#include "stat.h"
#include "user.h"

int 
main(int argc, char *argv[])
{
  if(argc != 3) {
      printf(2, "usage: setscheduler pid priority\n");
      exit();
  }
  if (setscheduler(atoi(argv[1]), atoi(argv[2])) < 0) {
      printf(2, "fatal error\n");
  }
  exit();
}