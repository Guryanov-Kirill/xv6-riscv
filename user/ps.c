#include "kernel/types.h"
#include "kernel/procinfo.h"
#include "user/user.h"
#include <stddef.h>

int
main(void)
{
  int lim = 16;
  struct procinfo *psInfo = malloc(sizeof(struct procinfo) * lim);
  if (psInfo == NULL) {
    fprintf(2, "Malloc failed\n");
    exit(1);
  }
  int psCount = ps_listinfo(psInfo, lim);
  if (psCount < 0) {
    fprintf(2, "Syscall failed\n");
    exit(1);
  }
  while (psCount > lim) {
    free(psInfo);
    lim *= 2;
    psInfo = malloc(sizeof(struct procinfo) * lim);
    if (psInfo == NULL) {
      fprintf(2, "Malloc failed\n");
      exit(1);
    }
    psCount = ps_listinfo(psInfo, lim);
    if (psCount < 0) {
      fprintf(2, "Syscall failed\n");
      exit(1);
    }
  }

  printf("PID\tNAME\t\tSTATE\t\tPPID\n");
  const char *stateStr[] = {"UNUSED",   "USED",    "SLEEPING",
                            "RUNNABLE", "RUNNING", "ZOMBIE"};

  for (int i = 0; i < psCount; i++) {
    int state = psInfo[i].state;
    if (state != 0 && state != 1) {
      printf("%d\t%s\t\t%s\t%d\n", psInfo[i].pid, psInfo[i].name,
             stateStr[state], psInfo[i].ppid);
    }
  }

  free(psInfo);
  exit(0);
}