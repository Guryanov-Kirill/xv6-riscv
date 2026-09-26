#ifndef _PROCINFO_H_
#define _PROCINFO_H_

struct procinfo {
  int pid;
  int ppid;
  char name[16];
  int state;
};

#endif