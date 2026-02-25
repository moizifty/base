#include "osCoreLinux.h"
#include "stdio.h"

typedef struct CmdLineHashMap CmdLineHashMap;
void ProgramMain(CmdLineHashMap *);

int main(int argc, char **argv)
{
    // int fd = open("/dev/ttyACM1", O_RDWR | O_NOCTTY);

    // if (fd == -1)
    // {
    //     perror("Failed to open serial port");
    // }
    // else
    // {
    //     printf("connection opened: %d\n", fd);
    // }


    // struct termios serielPortSettings;
    // tcgetattr(fd, &serielPortSettings);

    // serielPortSettings.c_lflag &= ~(ICANON | ECHO | ISIG | ECHOE);
    // serielPortSettings.c_iflag &= ~(IXON | IXOFF | IXANY);

    // serielPortSettings.c_cflag |= CREAD | CLOCAL;
    // serielPortSettings.c_cflag &= ~CRTSCTS;

    // serielPortSettings.c_cflag &= ~PARENB;
    // serielPortSettings.c_cflag &= ~CSTOPB;
    // serielPortSettings.c_cflag &= ~CSIZE;
    // serielPortSettings.c_cflag |= CS8;

    // cfsetispeed(&serielPortSettings, B115200);
    // cfsetospeed(&serielPortSettings, B115200);

    // tcsetattr(fd, TCSANOW, &serielPortSettings);

    // char buf[255] ={0};
    // read(fd, buf, 255);

    // printf("%s\n", buf);
    // close(fd);
    BaseMainThreadEntry(ProgramMain, (u64)argc, argv);
    return 0;
}