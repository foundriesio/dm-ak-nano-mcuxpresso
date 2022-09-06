#ifndef __AKNANO_CLIENT_H__
#define __AKNANO_CLIENT_H__
#include "aknano_priv.h"

int initStorage();
int initTime();
time_t get_current_epoch();
status_t AkNanoGenRandomBytes(char *output, size_t size);

#endif
