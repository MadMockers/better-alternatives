
#ifndef BALT_CONFIG_H
#define BALT_CONFIG_H

#include "map.h"
#include "list.h"

char *config_resolve_link(int pid, int uid, int gid, const char *link, char *out, size_t s);
List config_read_all(int uid, int gid);

#endif // !BALT_CONFIG_H
