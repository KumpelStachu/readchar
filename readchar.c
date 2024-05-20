#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  bool last = true;
  double time = 0;
  char *c = mmap(NULL, sizeof(*c), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  while (++argv, --argc) {
    if (strncmp(*argv, "-t", 2) == 0) {
      time = atof(*argv + 2);
    } else if (strncmp(*argv, "-f", 2) == 0) {
      last = false;
    } else {
      fprintf(stderr, "Usage: %s [-t<ms>] [-first]\n", argv[0]);
      return 1;
    }
  }

  struct termios t_old, t_new;
  tcgetattr(STDIN_FILENO, &t_old);
  t_new = t_old;
  t_new.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &t_new);

  if (time == 0) {
    *c = getchar();
  } else {
    pid_t pid = fork();
    if (pid < 0) {
      perror("fork");
      return 1;
    } else if (pid == 0) {
      while (true) {
        char t = getchar();
        *c = last || !*c ? t : *c;
      }
    } else {
      usleep(time * 1000);
      kill(pid, SIGKILL);
    }
  }

  if (*c == 0) {
    return 1;
  }

  tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
  putchar(*c);
  munmap(c, sizeof(*c));

  return 0;
}
