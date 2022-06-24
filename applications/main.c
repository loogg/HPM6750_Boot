#include <rtthread.h>
#include <rtdevice.h>
#include "system.h"

int main(void) {
    while (1) {
        system_process();
    }
}
