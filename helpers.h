#ifndef __HELPERS_H__
#define __HELPERS_H__

#include <sys/types.h>
#include <sys/stat.h>

#include "nfs.grpc.pb.h"

#include "helpers.h"

void copytimespec2TimeSpec(const struct timespec &src, nfs::TimeSpec *dst)
{
    dst->set_tv_sec(src.tv_sec);
    dst->set_tv_nsec(src.tv_nsec);
}

void copyTimeSpec2timespec(const nfs::TimeSpec &src, struct timespec *dst)
{
    dst->tv_sec = src.tv_sec();
    dst->tv_nsec = src.tv_nsec();
}

void copystat2Stat(const struct stat &src, nfs::Stat *dst)
{
    dst->set_st_dev(src.st_dev);
    dst->set_st_ino(src.st_ino);
    dst->set_st_mode(src.st_mode);
    dst->set_st_nlink(src.st_nlink);
    dst->set_st_uid(src.st_uid);
    dst->set_st_gid(src.st_gid);
    dst->set_st_rdev(src.st_rdev);
    dst->set_st_size(src.st_size);
    dst->set_st_blksize(src.st_blksize);
    dst->set_st_blocks(src.st_blocks);
    nfs::TimeSpec *st_atime_ = new nfs::TimeSpec;
    nfs::TimeSpec *st_mtime_ = new nfs::TimeSpec;
    nfs::TimeSpec *st_ctime_ = new nfs::TimeSpec;
    copytimespec2TimeSpec(src.st_atim, st_atime_);
    copytimespec2TimeSpec(src.st_mtim, st_mtime_);
    copytimespec2TimeSpec(src.st_ctim, st_ctime_);
    dst->set_allocated_st_atime_(st_atime_);
    dst->set_allocated_st_mtime_(st_mtime_);
    dst->set_allocated_st_ctime_(st_ctime_);
}

void copyStat2stat(const nfs::Stat &src, struct stat *dst)
{
    dst->st_dev = src.st_dev();
    dst->st_ino = src.st_ino();
    dst->st_mode = src.st_mode();
    dst->st_nlink = src.st_nlink();
    dst->st_uid = src.st_uid();
    dst->st_gid = src.st_gid();
    dst->st_rdev = src.st_rdev();
    dst->st_size = src.st_size();
    dst->st_blksize = src.st_blksize();
    dst->st_blocks = src.st_blocks();
    const nfs::TimeSpec st_atime_ = src.st_atime_();
    const nfs::TimeSpec st_mtime_ = src.st_mtime_();
    const nfs::TimeSpec st_ctime_ = src.st_ctime_();
    copyTimeSpec2timespec(st_atime_, &(dst->st_atim));
    copyTimeSpec2timespec(st_mtime_, &(dst->st_mtim));
    copyTimeSpec2timespec(st_ctime_, &(dst->st_ctim));
}

#endif