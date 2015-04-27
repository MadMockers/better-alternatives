
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

static int balt_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
             off_t offset, struct fuse_file_info *fi)
{
    (void) offset;
    (void) fi;

    if (strcmp(path, "/") != 0)
        return -ENOENT;

    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    struct fuse_context *ctxt = fuse_get_context();
    List list = config_read_all(ctxt->uid, ctxt->gid);

    char *name;
    while( (name = CALL(list, pop)) )
    {
        filler(buf, name, NULL, 0);
        free(name);
    }

    DELETE(list);

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

