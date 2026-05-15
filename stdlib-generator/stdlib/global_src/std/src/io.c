#include <unistd.h>
#include <sys/syscall.h>

void print_V_amd64(int num) {
    char buf[16] = {0};
    char *rsi = buf + 15;
    int is_negative = 0;

    if (num < 0) {
        num = -num;
        is_negative = 1;
    }

    do {
        rsi--;
        *rsi = (num % 10) + '0';
        num /= 10;
    } while (num > 0);

    if (is_negative) {
        rsi--;
        *rsi = '-';
    }

    char *rdi = rsi;
    while (*rdi != '\0') {
        rdi++;
    }
    *rdi = '\n';

    long rdx = (rdi + 1) - rsi;

    long syscall_ret;
    __asm__ __volatile__(
        "syscall"
        : "=a"(syscall_ret)
        : "a"(SYS_write), "D"(1), "S"(rsi), "d"(rdx)
        : "rcx", "r11", "memory"
    );
}

int scan_V_amd64(void) {
    char buf[16] = {0};
    long bytes_read;

    __asm__ __volatile__(
        "syscall"
        : "=a"(bytes_read)
        : "a"(SYS_read), "D"(0), "S"(buf), "d"(16)
        : "rcx", "r11", "memory"
    );

    if (bytes_read <= 0) {
        __asm__ __volatile__(
            "syscall"
            :
            : "a"(SYS_exit), "D"(0)
        );
    }

    int result = 0;
    int is_negative = 0;
    int rcx = 0;

    if (buf[rcx] == '-') {
        rcx++;
        is_negative = 1;
    }

    while (1) {
        unsigned char dl = (unsigned char)buf[rcx];
        if (dl < '0' || dl > '9') {
            break;
        }

        result = (result * 10) + (dl - '0');
        rcx++;
    }

    if (is_negative) {
        result = -result;
    }

    return result;
}

#define SYS_write 1


void print_53(void) {
    char msg[4];
    msg[0] = '5';
    msg[1] = '3';
    msg[2] = '\n';
    msg[3] = 0;

    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(SYS_write), "D"(1), "S"(msg), "d"(3)
        : "rcx", "r11", "memory"
    );
}