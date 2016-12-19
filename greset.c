#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

int
set_interface_attribs (int fd, int speed, int parity)
{
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0)
  {
    fprintf (stderr, "error %d from tcgetattr", errno);
    return -1;
  }

  cfsetospeed (&tty, speed);
  cfsetispeed (&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
  // disable IGNBRK for mismatched speed tests; otherwise receive break
  // as \000 chars
  tty.c_iflag &= ~IGNBRK;         // disable break processing
  tty.c_lflag = 0;                // no signaling chars, no echo,
                                  // no canonical processing
  tty.c_oflag = 0;                // no remapping, no delays
  tty.c_cc[VMIN]  = 0;            // read doesn't block
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

  tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls,
                                  // enable reading
  tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
  tty.c_cflag |= parity;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
  {
    fprintf (stderr, "error %d from tcsetattr", errno);
    return -1;
  }
  return 0;
}

void
set_blocking (int fd, int should_block)
{
  struct termios tty;
  memset (&tty, 0, sizeof tty);
  if (tcgetattr (fd, &tty) != 0)
  {
    fprintf (stderr, "error %d from tggetattr", errno);
    return;
  }

  tty.c_cc[VMIN]  = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
    fprintf (stderr, "error %d setting term attributes", errno);
}



char *portname = "/dev/ttyACM0";
int fd_serial = -1;

int initGlobalReset(void)
{
  fd_serial = open (portname, O_RDWR | O_NOCTTY);
  if (fd_serial < 0)
  {
    fprintf (stderr, "error %d opening %s: %s", errno, portname, strerror (errno));
    return 0;
  }
  set_interface_attribs (fd_serial, B9600, 0);  // set speed to 115,200 bps, 8n1 (no parity)
  set_blocking (fd_serial, 0);                  // set blocking
  return 1;
}

int finiGlobalReset(void)
{
  close(fd_serial);
  return 1;
}

int sendGlobalReset(char val)
{
  char send = 'b';
  if(val) send = 'a'; 
  if(fd_serial < 0) return 0;
  while(1)
  {
    write (fd_serial, &send, 1);
    char buf [100];
    int n = read (fd_serial, buf, sizeof buf);  // read up to 100 characters if ready to read
    if(n >= 1 && buf[0] == send) break;
  }
  return 1;
}

/*int main(int argc, char* argv[])
{
  initGlobalReset();
  // Stupid and sexy blink, our 'hello world'
  sendGlobalReset(1);
  sleep(1);
  sendGlobalReset(0);
  sleep(1);
  sendGlobalReset(1);
  sleep(1);
  sendGlobalReset(0);
  sleep(1);
  sendGlobalReset(1);
  sleep(1);
  sendGlobalReset(0);
  finiGlobalReset();
  return 0;
}*/
