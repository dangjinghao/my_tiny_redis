
#pragma once
#include <stdio.h>
#include <stdlib.h>
#define log_errno(msg) {fprintf(stderr,"%d-%s: %s\n",errno,strerror(errno),msg);exit(1);}