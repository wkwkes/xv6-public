#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    printf(1, "%d\n", sys_dbl(atoi(argv[1])));
    exit();
}