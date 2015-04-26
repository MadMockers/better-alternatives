
#define FUSE_USE_VERSION 26

#include <limits.h>
#include <fuse.h>
#include <errno.h>
#include <string.h>

#include "config.h"

static char *resolve_link(const char *link, char *out, size_t out_size)
{
    struct fuse_context *ctxt = fuse_get_context();
    return config_resolve_link(ctxt->pid, ctxt->uid, ctxt->gid, link+1, out, out_size);
}

static int balt_getattr(const char *link, struct stat *stat)
{
    memset(stat, '\0', sizeof(struct stat));
    if(strcmp(link, "/") == 0)
    {
        stat->st_mode = S_IFDIR | 0555;
        stat->st_nlink = 2;
        return 0;
    }

    char buf[PATH_MAX];
    char *f = resolve_link(link, buf, sizeof(buf));
    if(!f)
        return -ENOENT;
    stat->st_mode = S_IFLNK | 0444;
    stat->st_nlink = 1;
    stat->st_size = strlen(f);
    return 0;
}

static int balt_readlink(const char *link, char *buf, size_t size)
{
    if(strcmp(link, "/") == 0)
        return -EINVAL;

    char tmp[PATH_MAX];
    char *f = resolve_link(link, tmp, sizeof(tmp));
    if(!f)
        return -ENOENT;

    strncpy(buf, f, size-1);
    f[size-1] = '\0';

    return 0;
}

struct readdir_context
{
    fuse_fill_dir_t filler;
    void *buf;
};

static void add_entry(void *ctxt, struct tree_node *n)
{
    struct map_bucket *b = list_entry(n, struct map_bucket, node);
    struct readdir_context *c = ctxt;

    struct alt_entry *e;
    struct map_entry *m, *i;
    list_for_each_entry_safe(m, i, &b->list, list)
    {
        e = list_entry(m, struct alt_entry, entry);
        c->filler(c->buf, e->alt_name, NULL, 0);

        /* Freeing this corrupts the Map, but we aren't using it anymore */
        free(e->alt_name);
        free(e);
    }
    
}

static long long str_hash(void *a)
{
    unsigned char *str = a;
    long long hash = 5381;
    int c;

    while ( (c = *str++) )
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static char str_equals(void *a, void *b)
{
    return !strcmp(a, b);
}

static int balt_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    Map map = NEW(Map, str_hash, str_equals);

    struct fuse_context *ctxt = fuse_get_context();
    config_read_all(ctxt->uid, ctxt->gid, map);

    struct readdir_context c = {
        .filler = filler,
        .buf = buf,
    };

    CALL(map->btree, iterate, add_entry, &c);
    DELETE(map);

    return 0;
}

int main(int argc, char *argv[])
{
    struct fuse_operations fops = {
        .getattr = balt_getattr,
        .readlink = balt_readlink,
        .readdir = balt_readdir,
    };
    fuse_main(argc, argv, &fops, NULL);
    return 0;
}

