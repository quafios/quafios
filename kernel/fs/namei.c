/*
 *        +----------------------------------------------------------+
 *        | +------------------------------------------------------+ |
 *        | |  Quafios Kernel 1.0.2.                               | |
 *        | |  -> Filesystem: File name routines.                  | |
 *        | +------------------------------------------------------+ |
 *        +----------------------------------------------------------+
 *
 * This file is part of Quafios 1.0.2 source code.
 * Copyright (C) 2014  Mostafa Abd El-Aziz Mohamed.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Quafios.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Visit http://www.quafios.com/ for contact information.
 *
 */

#include <arch/type.h>
#include <lib/string.h>
#include <sys/mm.h>
#include <sys/fs.h>
#include <sys/scheduler.h>

/***************************************************************************/
/*                              alloc_path()                               */
/***************************************************************************/

char *alloc_path(char *parent, char *child) {
    char *ret;
    if (!strcmp(child, ".")) {
        /* return the same path of parent. */
        ret = (char *) kmalloc(strlen(parent)+1);
        if (!ret)
            return NULL;
        strcpy(ret, parent);
    } else if (!strcmp(child, "..")) {
        /* trim last component of parent path.
         * it is guaranteed that parent is not "/";
         */
        int32_t len = strlen(parent);
        while(parent[--len] != '/');
        if (!len)
            len = 1; /* cases like "/home" and ".." */
        ret = (char *) kmalloc(len+1);
        if (!ret)
            return NULL;
        ret[len--] = 0;
        while(len >= 0)
            ret[len] = parent[len--];
    } else if (!strcmp(parent, "/")) {
        /* "/" with "etc" */
        ret = (char *) kmalloc(strlen(child)+2);
        if (!ret)
            return NULL;
        ret[0] = '/';
        strcpy(&ret[1], child);
    } else {
        /* concatenate both paths: */
        int32_t len1 = strlen(parent);
        int32_t len2 = strlen(child);
        ret = (char *) kmalloc(len1+1+len2+1);
        if (!ret)
            return NULL;
        strcpy(&ret[0], parent);
        ret[len1] = '/';
        strcpy(&ret[len1+1], child);
    }
    return ret;
}

/***************************************************************************/
/*                              divide_path()                              */
/***************************************************************************/

int32_t divide_path(char *pathname,
                    int32_t trail,
                    char **parent,
                    char **child) {

    /* used by routines that create new nodes in the system.
     * it separates /home/george/.bashrc to "/home/george"
     * and ".bashrc". useful for: creat("/home/george/.bashrc");
     * if trail is 1, trailing slashes are allowed.
     */

    char *tmp;
    char *parent_start;
    char *child_start;
    int32_t i = strlen(pathname);

    /* trailing slash? */
    if ((!trail) && pathname[i-1] == '/')
        return EINVAL;

    /* pathname is empty? */
    if (!strlen(pathname))
        return EINVAL; /* pathname = "" */

    /* skip trailing slash if found: */
    while(i >= 0 && pathname[--i] == '/');
    if (i == -1)
        return EEXIST; /* pathname = "/", "//", "///", ... */

    /* allocate a temporary string: */
    tmp = kmalloc(i+1);
    if (!tmp)
        return ENOMEM;

    /* copy pathname to kernel memory: */
    strcpy(tmp, pathname);

    /* trim trailing slashes: */
    tmp[i+1] = 0;

    /* get parent and child strings: */
    while(i >= 0 && tmp[--i] != '/');
    if (i > 0) {
        /* separate "/var/drugs" to "/var" and "drugs" */
        tmp[i] = 0;
        parent_start = tmp;
        child_start = &tmp[i+1];
    } else if (i == 0) {
        /* separate "/bin" to "/" and "bin" */
        parent_start = "/";
        child_start = &tmp[1];
    } else {
        /* pathname = "george" only. */
        parent_start = ".";
        child_start = tmp;
    }

    /* child names of "." & ".." are not allowed: */
    if (!strcmp(child_start, ".") || !strcmp(child_start, "..")) {
        kfree(tmp);
        return EEXIST;
    }

    /* copy parent: */
    *parent = kmalloc(strlen(parent_start)+1);
    if (*parent == NULL) {
        kfree(tmp);
        return ENOMEM;
    }
    strcpy(*parent, parent_start);

    /* copy child: */
    *child = kmalloc(strlen(child_start)+1);
    if (*child == NULL) {
        kfree(tmp);
        kfree(*parent);
        return ENOMEM;
    }
    strcpy(*child, child_start);

    /* done: */
    kfree(tmp);
    return ESUCCESS;
}

/***************************************************************************/
/*                                namei()                                  */
/***************************************************************************/

int32_t namei(file_t *start, char *pathname, namei_t *ret) {

    char *tmp;             /* for temporary storage. */
    inode_t *inode = NULL; /* current node we are working on. */
    char *fullpath;        /* full path of the file. */
    vfsmount_t *mnt;       /* mount point of inode. */
    int32_t err = ESUCCESS;
    int32_t i = 0;
    int32_t f = 0; /* ended flag. */

    /* I) Determine the starting point:  */
    /* ------------------------------- - */
    if (pathname[0] == '/') {

        /* the root of the file system: */
        mnt = vfsroot;

        /* sometimes "/" is mounted many times, we must get over this: */
        while(1) {

            /* get super block: */
            super_block_t *sb = mnt->sb;

            /* drop the inode (because we'll read another): */
            if (inode)
                iput(inode);

            /* open "/" using the superblock sb: */
            if (!(inode = (inode_t *) iget(sb, sb->root_ino))) {
                return ENOMEM;
            }

            /* no sub mount point? */
            if (!(inode->submount)) {
                break;
            }

            /* get sub mount point: */
            mnt = inode->submount;

        }

        /* path of the currently open inode: */
        fullpath = "/";

    } else if (!start) {

        /* start point is the current working directory: */
        mnt = curproc->cwd->mp;

        inode = (inode_t *) iget(curproc->cwd->inode->sb,
                                 curproc->cwd->inode->ino);

        if (!inode)
            return ENOMEM;

        fullpath = curproc->cwd->path;

    } else {

        /* start point is the file opened by "start" */
        mnt = start->mp;

        inode = (inode_t *) iget(start->inode->sb,
                                 start->inode->ino);

        if (!inode)
            return ENOMEM;

        fullpath = start->path;
    }

    /* II) Copy paths to kernel memory:  */
    /* --------------------------------- */
    tmp = kmalloc(strlen(pathname)+1);
    if (!tmp) {
        iput(inode);
        return ENOMEM;
    }
    strcpy(tmp, pathname);
    pathname = tmp;

    tmp = kmalloc(strlen(fullpath)+1);
    if (!tmp) {
        iput(inode);
        kfree(pathname);
        return ENOMEM;
    }
    strcpy(tmp, fullpath);
    fullpath = tmp;

    /* III) Loop on path components ( interesting (Y) ):  */
    /* -------------------------------------------------- */
    while(!f) {

        /* i is the offset of next component
         * in pathname. get component length:
         */
        inode_t *child;
        int32_t j = i;
        while(pathname[j] != '/' && pathname[j] != 0)
            j++;

        /* currently open inode must be directory: */
        if ((inode->mode & FT_MASK) != FT_DIR) {
            err = ENOTDIR;
            break;
        }

        /* is it last component? */
        if (!pathname[j]) {
            f = 1;
        } else {
            pathname[j] = 0; /* terminate in \0: */
        }

        /* empty component name? like in //home */
        if (i == j) {
            i++;
            continue;
        }

        /* "." within the root of a mounted filesystem? */
        if (inode->ino == inode->sb->root_ino &&
            (!strcmp(&pathname[i], "."))) {
            /* this shouldn't affect "cur": */
            i += 2;
            continue;
        }

        /* ".." within the root of a mounted filesystem? */
        if (inode->ino == inode->sb->root_ino &&
            (!strcmp(&pathname[i], ".."))) {
            if (!strcmp(fullpath, "/")) {
                /* we are in the root of vfs,
                 * it is like: open("/..")
                 * silly.. isn't it?
                 */
                i += 3;
                continue;
            } else {
                /* this is kinda tricky: */
                ino_t ino;
                super_block_t *sb;
                iput(inode);
                ino = mnt->mp_inode->ino;
                sb = (mnt=mnt->vfsparent)->sb;
                inode = (inode_t *) iget(sb, ino);
                if (!inode) {
                    err = ENOMEM;
                    break;
                }

                /* parse the ".." again. */
                if (!f)
                    pathname[j] = '/';
                else
                    f = 0;
                continue;

            }
        }

        /* get inode number of this component */
        err = mnt->sb->fsdriver->lookup(inode, &pathname[i], &child);
        iput(inode);
        if (err)
            break;
        inode = child;

        /* this component is a mount point? */
        while (inode->submount) {
            mnt = inode->submount;
            iput(inode);
            inode = (inode_t *) iget(mnt->sb, mnt->sb->root_ino);
            if (!inode) {
                err = ENOMEM;
                break;
            }
        }
        if (err)
            break;

        /* now append the component name to the path: */
        tmp = alloc_path(fullpath, &pathname[i]);
        if (!tmp) {
            iput(inode);
            err = ENOMEM;
            break;
        }
        kfree(fullpath);
        fullpath = tmp;

        /* go to next component */
        i = j+1;
    }

    /* IV) Return:  */
    /* ------------ */
    kfree(pathname);
    if (err) {
        kfree(fullpath);
    } else {
        ret->path = fullpath;
        ret->inode = inode;
        ret->mp = mnt;
    }
    return err;

}

/***************************************************************************/
/*                                mknod()                                  */
/***************************************************************************/

int32_t mknod(char *pathname, mode_t mode, uint32_t devid) {

    /* mknod() system call */
    char *parent, *child;
    inode_t *dir;
    super_block_t *sb;
    namei_t namei_data;
    int32_t err;

    /* divide pathname: */
    if (err = divide_path(pathname, 0, &parent, &child))
        return -err;

    /* get the inode structure of parent: */
    err = namei(NULL, parent, &namei_data);
    kfree(parent); /* no need anymore. */
    if (err) {
        kfree(child);
        return -err;
    }

    /* extract information from namei_data: */
    dir = namei_data.inode;
    sb = dir->sb;
    kfree(namei_data.path); /* not needed. */

    /* call filesystem driver: */
    err = sb->fsdriver->mknod(dir, child, mode, devid);

    /* that's all! */
    iput(dir);
    kfree(child);
    return -err;

}

/***************************************************************************/
/*                               rename()                                  */
/***************************************************************************/

int32_t rename(char *oldpath, char *newpath) {

    /* rename() system call */
    char *old_parent, *old_child;
    char *new_parent, *new_child;
    namei_t old_data, new_data;
    inode_t *old_dir, *new_dir;
    int32_t err;

    /* divide "oldpath": */
    if (err = divide_path(oldpath, 0, &old_parent, &old_child))
        return -err;

    /* divide "newpath": */
    if (err = divide_path(newpath, 0, &new_parent, &new_child)) {
        kfree(old_parent);
        kfree(old_child);
        return -err;
    }

    /* get the inode structure of old_parent: */
    err = namei(NULL, old_parent, &old_data);
    if (err) {
        kfree(old_parent);
        kfree(old_child);
        kfree(new_parent);
        kfree(new_child);
        return -err;
    }
    old_dir = old_data.inode;
    kfree(old_data.path);

    /* get the inode structure of new_parent: */
    err = namei(NULL, new_parent, &new_data);
    if (err) {
        iput(old_dir);
        kfree(old_parent);
        kfree(old_child);
        kfree(new_parent);
        kfree(new_child);
        return -err;
    }
    new_dir = new_data.inode;
    kfree(new_data.path);

    /* both paths must be on the same mount point. */
    if (old_data.mp != new_data.mp) {
        iput(new_dir);
        iput(old_dir);
        kfree(old_parent);
        kfree(old_child);
        kfree(new_parent);
        kfree(new_child);
        return -EXDEV;
    }

    /* call the filesystem: */
    /* TODO: err = old_dir->sb->fsdriver->rename(); */

    /* done: */
    iput(new_dir);
    iput(old_dir);
    kfree(old_parent);
    kfree(old_child);
    kfree(new_parent);
    kfree(new_child);
    return -err;

}

/***************************************************************************/
/*                                link()                                   */
/***************************************************************************/

int32_t link(char *oldpath, char *newpath) {

    /* link() system call */
    char *new_parent, *new_child;
    namei_t inode_data, dir_data;
    inode_t *inode, *dir;
    int32_t err;

    /* get the inode structure of oldpath: */
    err = namei(NULL, oldpath, &inode_data);
    if (err)
        return -err;
    inode = inode_data.inode;
    kfree(inode_data.path);

    /* divide "newpath": */
    if (err = divide_path(newpath, 0, &new_parent, &new_child)) {
        iput(inode);
        return -err;
    }

    /* get the inode structure of new_parent: */
    err = namei(NULL, new_parent, &dir_data);
    if (err) {
        iput(inode);
        kfree(new_parent);
        kfree(new_child);
        return -err;
    }
    dir = dir_data.inode;
    kfree(dir_data.path);

    /* both paths must be on the same mount point. */
    if (inode_data.mp != dir_data.mp) {
        iput(inode);
        iput(dir);
        kfree(new_parent);
        kfree(new_child);
        return -EXDEV;
    }

    /* call the filesystem: */
    err = inode->sb->fsdriver->link(inode, dir, new_child);

    /* done: */
    iput(inode);
    iput(dir);
    kfree(new_parent);
    kfree(new_child);
    return -err;

}

/***************************************************************************/
/*                                ulink()                                  */
/***************************************************************************/

int32_t unlink(char *pathname) {

    /* ulink() system call */
    char *parent, *child;
    inode_t *dir;
    super_block_t *sb;
    namei_t namei_data;
    int32_t err;

    /* divide pathname: */
    if (err = divide_path(pathname, 0, &parent, &child))
        return -err;

    /* get the inode structure of parent: */
    err = namei(NULL, parent, &namei_data);
    kfree(parent); /* no need anymore. */
    if (err) {
        kfree(child);
        return -err;
    }

    /* extract information from namei_data: */
    dir = namei_data.inode;
    sb = dir->sb;
    kfree(namei_data.path); /* not needed. */

    /* call filesystem driver: */
    err = sb->fsdriver->unlink(dir, child);

    /* that's all! */
    iput(dir);
    kfree(child);
    return -err;

}

/***************************************************************************/
/*                                mkdir()                                  */
/***************************************************************************/

int32_t mkdir(char *pathname, mode_t mode) {

    /* mkdir() system call */
    char *parent, *child;
    inode_t *dir;
    super_block_t *sb;
    namei_t namei_data;
    int32_t err;

    /* divide pathname: */
    if (err = divide_path(pathname, 1, &parent, &child))
        return -err;

    /* get the inode structure of parent: */
    err = namei(NULL, parent, &namei_data);
    kfree(parent); /* no need anymore. */
    if (err) {
        kfree(child);
        return -err;
    }

    /* extract information from namei_data: */
    dir = namei_data.inode;
    sb = dir->sb;
    kfree(namei_data.path); /* not needed. */

    /* call filesystem driver: */
    err = sb->fsdriver->mkdir(dir, child, mode);

    /* that's all! */
    iput(dir);
    kfree(child);
    return -err;

}

/***************************************************************************/
/*                                rmdir()                                  */
/***************************************************************************/

int32_t rmdir(char *pathname) {

    /* rmdir() system call */
    char *parent, *child;
    inode_t *dir;
    super_block_t *sb;
    namei_t namei_data;
    int32_t err;

    /* divide pathname: */
    if (err = divide_path(pathname, 1, &parent, &child))
        return -err;

    /* get the inode structure of parent: */
    err = namei(NULL, parent, &namei_data);
    kfree(parent); /* no need anymore. */
    if (err) {
        kfree(child);
        return -err;
    }

    /* extract information from namei_data: */
    dir = namei_data.inode;
    sb = dir->sb;
    kfree(namei_data.path); /* not needed. */

    /* call filesystem driver: */
    err = sb->fsdriver->rmdir(dir, child);

    /* that's all! */
    iput(dir);
    kfree(child);
    return -err;

}
