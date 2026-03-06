#ifndef OS_CORE_LINUX_H
#define OS_CORE_LINUX_H

#define _GNU_SOURCE
#include <sys/mman.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "base/baseCoreTypes.h"

#endif