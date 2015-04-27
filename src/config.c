
#include <limits.h>
#include <ctype.h>
#include <stdint.h>
#include <talloc.h>
#include <stdint.h>
#include <string.h>

#include <pwd.h>
#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"

static char *read_config(char *file, const char *link, char *out, size_t s)
{
    FILE *f = fopen(file, "r");
    if(!f)
        return NULL;

    int count;
    int bin_priority = INT_MIN;
    char buf[1024];

    /* limit the number of lines as this may be untrusted data
     * This leaves us open to a maximum of 10MiB mem usage per file */
    for(count = 0;count < 10 * 1024 && !feof(f);count++)
    {
        char *line = fgets(buf, sizeof(buf), f);
        if(!line)
            break;
        if(line[0] == '#')
            continue;
        if(strstr(line, "//") == line)
            continue;
        for(;*line && isspace(*line);line++); // skip leading whitespace
        
        char *tmp, *alt_name, *bin_name, *str_priority;
        if(!(alt_name = strtok_r(line, " \t\r\n\v\f", &tmp)))
            continue;
        if(!(bin_name = strtok_r(NULL, " \t\r\n\v\f", &tmp)))
            continue;
        if(!(str_priority = strtok_r(NULL, " \t\r\n\v\f", &tmp)))
            continue;
        int priority = strtol(str_priority, NULL, 0);

        if(strcmp(alt_name, link) != 0)
            continue;

        if(strlen(bin_name) > PATH_MAX-1)
            continue;

        struct stat buf;
        if(stat(bin_name, &buf) == -1)
        {
            syslog(LOG_ERR, "Skipping non-existant binary '%s'", bin_name);
        }

        if(priority > bin_priority)
        {
            strncpy(out, bin_name, s-1);
            bin_priority = priority;
        }
    }
    fclose(f);

    if(bin_priority > INT_MIN)
        return out;
    return NULL;
}

static void read_all(char *file, Map map)
{
    FILE *f = fopen(file, "r");
    if(!f)
        return;

    char buf[1024];
    while(!feof(f))
    {
        char *line = fgets(buf, sizeof(buf), f);
        if(!line)
            break;
        if(line[0] == '#')
            continue;
        if(strstr(line, "//") == line)
            continue;
        for(;*line && isspace(*line);line++); // skip leading whitespace
        if(!*line)
            continue;
        
        char *tmp, *alt_name, *bin_name, *str_priority;
        if(!(alt_name = strtok_r(line, " \t\r\n\v\f", &tmp)))
            continue;
        if(!(bin_name = strtok_r(NULL, " \t\r\n\v\f", &tmp)))
            continue;
        if(!(str_priority = strtok_r(NULL, " \t\r\n\v\f", &tmp)))
            continue;

        if(!CALL(map, get, alt_name))
        {
            char *name = strdup(alt_name);
            CALL(map, put, name, name);
        }
    }
    fclose(f);
}

List config_read_all(int uid, int gid)
{
    /* TODO: by executable */

    /* search order: $HOME/.alternatives
     *               /etc/balternatives/$GID.alternatives
     *               /etc/balternatives/.alternatives
     */

    Map map = NEW(Map, str_hash, str_equals);

    struct passwd *pw;
    char path[PATH_MAX];
    
    pw = getpwuid(uid);
    if(pw)
    {
        snprintf(path, sizeof(path), "%s/.alternatives", pw->pw_dir);
        read_all(path, map);
    }

    snprintf(path, sizeof(path), "/etc/balternatives/%d.alternatives", gid);
    read_all(path, map);

    read_all("/etc/balternatives/.alternatives", map);

    List list = CALL(map, to_list);
    DELETE(map);
    return list;
}

char *config_resolve_link(int pid, int uid, int gid, const char *link, char *out, size_t s)
{
    /* TODO: by executable */

    /* search order: $HOME/.alternatives
     *               /etc/balternatives/$GID.alternatives
     *               /etc/balternatives/.alternatives
     */

    struct passwd *pw;
    char path[PATH_MAX];
    
    pw = getpwuid(uid);
    if(pw)
    {
        snprintf(path, sizeof(path), "%s/.alternatives", pw->pw_dir);
        if(read_config(path, link, out, s))
            return out;
    }

    snprintf(path, sizeof(path), "/etc/balternatives/%d.alternatives", gid);
    if(read_config(path, link, out, s))
        return out;

    if(read_config("/etc/balternatives/.alternatives", link, out, s))
        return out;
    return NULL;
}

