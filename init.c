#include <stdio.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <bsp.h>
#include <rtems/tftp.h>
#include <rtems/rtems_bsdnet.h>

static
int do_chdir(const char *dname)
{
    int ret;
    fprintf(stderr, "\n\n********* chdir(\"%s\") *********", dname);
    ret = chdir(dname);
    fprintf(stderr, " -> %d (%d)\n", ret, errno);
    return ret;
}

static
void do_file(const char *fname, const char *expect)
{
    char rbuf[32];
    int fd, ret;
    ssize_t N;
    size_t elen = strlen(expect);
    off_t off;

    fprintf(stderr, "********* do_test(%s) *********\n", fname);

    /* tftpfs supports only O_WRONLY or O_RDONLY,
     *  QEMU's TFTP is read-only (ignores WRQ)
     */
    fd = open(fname, O_RDONLY);
    fprintf(stderr, "open(\"%s\") -> %d (%s)\n", fname, fd, strerror(errno));
    if(fd==-1) {
        fprintf(stderr, "ERROR: failed to open\n");
        return;
    }

    N = read(fd, rbuf, sizeof(rbuf)-1);
    rbuf[N>0 ? N : 0] = '\0';
    fprintf(stderr, "read() -> %d (%s) \"%s\"\n", (int)N, strerror(errno), rbuf);

    if(N!=elen)
        fprintf(stderr, "ERROR: length mis-match %d %u\n",
                (int)N, (unsigned)elen);
    else if(memcmp(rbuf, expect, elen)!=0)
        fprintf(stderr, "ERROR: mis-match\n");
    else
        fprintf(stderr, "OK\n");

    ret = close(fd);
    fprintf(stderr, "close() -> %d (%s)\n", ret, strerror(errno));
}

static const char file1[] = "hello world\n";
static const char file2[] = "this is a test\n";
static const char file3[] = "testing\n";
static const char file4[] = "file4\n";

rtems_task
Init (rtems_task_argument ignored)
{
    int ret;

    rtems_bsdnet_initialize_network();

    fprintf(stderr, "Beginning TFTP test\n");

    ret = mount_and_make_target_path("BOOTP_HOST:/", "/TFTP",
                                     RTEMS_FILESYSTEM_TYPE_TFTPFS,
                                     RTEMS_FILESYSTEM_READ_WRITE,
                                     "verbose");

    fprintf(stderr, "mount() -> %d (%s)\n", ret, strerror(errno));

    if(ret) return;

    if(!do_chdir("/")) {
        do_file("/TFTP/file1", file1);
        do_file("/TFTP/file2", file2);
        do_file("/TFTP/subdir/file3", file3);
    }

    if(!do_chdir("/TFTP/")) {
        do_file("file1", file1);
        do_file("file2", file2);
        do_file("subdir/file3", file3);
    }

    if(!do_chdir("subdir/")) {
        do_file("../file1", file1);
        do_file("file3", file3);
        do_file("another/file4", file4);
    }

    if(!do_chdir("another/")) {
        do_file("../../file1", file1);
        do_file("../file3", file3);
        do_file("file4", file4);
    }

    if(!do_chdir("/TFTP/subdir/")) {
        do_file("../file1", file1);
        do_file("file3", file3);
    }

    fprintf(stderr, "******** Expected errors ***********\n");

    fprintf(stderr, "Open non-existent file\n");
    ret = open("invalid", O_RDONLY);
    fprintf(stderr, "open(\"invalid\") -> %d (%s)\n", ret, strerror(errno));
    if(ret!=-1) fprintf(stderr, "ERROR: opened non-existent file\n");
    else fprintf(stderr, "OK\n");

    fprintf(stderr, "chdir to a file\n");
    ret = chdir("afile");
    fprintf(stderr, "chdir(\"afile\") -> %d (%s)\n", ret, strerror(errno));
    if(ret!=-1) fprintf(stderr, "ERROR: chdir to file\n");
    else fprintf(stderr, "OK\n");

    fprintf(stderr, "can't unmount while the current directory is in the TFTP tree\n");
    ret = unmount("/TFTP");
    fprintf(stderr, "unmount() -> %d (%s)\n", ret, strerror(errno));
    if(ret==0) fprintf(stderr, "ERROR: unmount while PWD under /TFTP\n");
    else fprintf(stderr, "OK\n");

    do_chdir("/");

    {
        int fd = open("/TFTP/file1", O_RDONLY);
        fprintf(stderr, "open(\"/TFTP/file1\") -> %d (%s)\n", fd, strerror(errno));
        if(fd==-1)
            fprintf(stderr, "ERROR: can't open file1 this time\n");
        else {
            fprintf(stderr, "can't unmount while a file is open in the TFTP tree\n");
            ret = unmount("/TFTP");
            fprintf(stderr, "unmount() -> %d (%s)\n", ret, strerror(errno));
            if(ret==0) fprintf(stderr, "ERROR: unmount while PWD under /TFTP\n");
            else fprintf(stderr, "OK\n");

            ret = close(fd);
            fprintf(stderr, "close() -> %d (%s)\n", ret, strerror(errno));
        }

    }

    ret = unmount("/TFTP");
    fprintf(stderr, "unmount() -> %d (%s)\n", ret, strerror(errno));
    if(ret!=0) fprintf(stderr, "ERROR: unmount\n");
    else fprintf(stderr, "OK\n");

    fprintf(stderr, "Done\n");
    bsp_reset();
    return;
}
