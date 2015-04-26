
#ifndef BALT_CONFIG_H
#define BALT_CONFIG_H

#include "map.h"
#include "list.h"

struct alt_entry
{
    struct map_entry entry;
    char *alt_name;
};

char *config_resolve_link(int pid, int uid, int gid, const char *link, char *out, size_t s);
void config_read_all(int uid, int gid, Map map);

#endif // !BALT_CONFIG_H
