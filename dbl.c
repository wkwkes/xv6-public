#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    printf(1, "%d\n", dbl(atoi(argv[1])));
    exit();
}