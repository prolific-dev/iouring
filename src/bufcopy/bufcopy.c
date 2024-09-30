#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/ioctl.h>
#include <liburing.h>

#include <sched.h>


double start, end;
ssize_t n;


off_t file_size;

int buf_size = 2048;

void set_cpu_affinity(int cpu)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
    if (sched_setaffinity(0, sizeof(mask), &mask) < 0)
    {
        perror("sched_setaffinity");
        exit(1);
    }
}

off_t get_file_size(int fd)
{
    struct stat st;

    if (fstat(fd, &st) < 0)
    {
        perror("fstat");
        exit(1);
    }

    if (S_ISBLK(st.st_mode))
    {
        unsigned long long bytes;
        if (ioctl(fd, BLKGETSIZE64, &bytes) < 0)
        {
            perror("ioctl");
            exit(1);
        }
        return bytes;
    }

    if (S_ISREG(st.st_mode))
        return st.st_size;

    fprintf(stderr, "unknown file type\n");

    exit(1);
}

double get_time()
{
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec * 0.000001;
}

void measured_read(const char *filename)
{
    int fd;
    int i;
    off_t file_size;

    void *buf;

    unsigned long long num_reads;
    unsigned long long num_writes;


    posix_memalign(&buf, buf_size, buf_size);
    if (!buf)
    {
        perror("malloc");
        exit(1);
    }

    fd = open(filename, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }

    file_size = get_file_size(fd);
    num_reads = (unsigned long long) file_size / buf_size;
    if (file_size % buf_size) num_reads++;

    start = get_time();
    for (i = 0; i < num_reads; i++)
    {
        n = read(fd, buf, buf_size);
        if (n < 0) {
            perror("read");
            exit(1);
        }
        if (n == 0) {
            break;
        }
    }
    end = get_time();

    printf("read (num_reads=%llu): %f\n", num_reads, end - start);
    close(fd);

    free(buf);
}

void measured_io_uring_read(const char *filename)
{
    int rc;
    int i;
    int fd;
    struct io_uring ring;
    struct io_uring_params p;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    struct iovec iovecs[1];
    struct __kernel_timespec ts;
    off_t file_size;
    off_t offset;
    int fds[0];

    void *buf;

    unsigned long long num_reads;
    unsigned long long num_writes;

    ts.tv_sec = 1;
    ts.tv_nsec = 0;

    offset = 0;

    buf = malloc(buf_size);
    if (!buf)
    {
        perror("malloc");
        exit(1);
    }

    fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    fds[0] = fd;

    file_size = get_file_size(fd);
    num_reads = (unsigned long long) file_size / buf_size;
    if (file_size % buf_size) num_reads++;

    iovecs[0].iov_base = buf;
    iovecs[0].iov_len = buf_size;

    memset(&p, 0, sizeof(p));

    /*
    p.flags |= IORING_SETUP_SQPOLL;
    p.sq_thread_cpu = 3;
    p.sq_thread_idle = 5000;
    */

    if (io_uring_queue_init(1, &ring, 0) < 0)
    {
        perror("io_uring_queue_init");
        exit(1);
    }

    /*
    if (io_uring_queue_init_params(32, &ring, &p) < 0)
    {
        perror("io_uring_queue_init_params");
        exit(1);
    }
    */

    if (io_uring_register_files(&ring, &fd, 1) < 0)
    {
        perror("io_uring_register_files");
        exit(1);
    }

    if (io_uring_register_buffers(&ring, iovecs, 1) < 0)
    {
        perror("io_uring_register_buffers");
        exit(1);
    }

    start = get_time();
    for (i = 0; i < num_reads; i++)
    {
        rc = io_uring_sqring_wait(&ring);
        if (rc < 0)
        {
            if (rc == -EINVAL)
                perror("io_uring_sqring_wait: EINVAL");

            perror("io_uring_sqring_wait");
            close(fd);
            io_uring_queue_exit(&ring);
            exit(1);
        }
        sqe = io_uring_get_sqe(&ring);
        if (!sqe)
        {
            perror("io_uring_get_sqe");
            close(fd);
            io_uring_queue_exit(&ring);
            exit(1);
        }

        /*
        io_uring_prep_read(sqe, 0, iovecs[0].iov_base, buf_size, i * buf_size);
        */
        io_uring_prep_read_fixed(sqe, 0, iovecs[0].iov_base, buf_size, i * buf_size, 0);
        io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

        io_uring_submit(&ring);

        rc = io_uring_wait_cqe(&ring, &cqe);
        if (rc < 0)
        {
            fprintf(stderr, "rc: %d\n", rc);
            perror("io_uring_wait_cqe");
            close(fd);
            io_uring_queue_exit(&ring);
            exit(1);
        }

        io_uring_cqe_seen(&ring, cqe);

    }
    end = get_time();

    printf("io_uring (num_reads=%llu): %f\n", num_reads, end - start);

    io_uring_unregister_files(&ring);
    io_uring_unregister_buffers(&ring);
    io_uring_queue_exit(&ring);
    close(fd);

    free(buf);
}

void measured_write(const char *filename)
{
    int fd, file_fd;
    int i;
    off_t file_size;

    unsigned long long num_writes = 0;
    unsigned long long num_reads;

    void *buf;

    posix_memalign(&buf, buf_size, buf_size);
    if (!buf)
    {
        perror("malloc");
        exit(1);
    }

    fd = open("/dev/null", O_WRONLY);
    if (fd < 0)
    {
        perror("open");
        exit(1);
    }

    file_fd = open(filename, O_RDONLY);
    if (file_fd < 0)
    {
        perror("open");
        exit(1);
    }

    file_size = get_file_size(file_fd);
    num_reads = (unsigned long long) file_size / buf_size;
    if (file_size % buf_size) num_reads++;

    start = get_time();
    for (i = 0; i < num_reads; i++)
    {
        n = write(fd, buf, buf_size);
        num_writes++;
        if (n < 0) {
            perror("write");
            exit(1);
        }
    }
    end = get_time();

    printf("write (num_writes=%llu): %f\n", num_writes, end - start);
    close(fd);
    close(file_fd);

    free(buf);
}

void measured_io_uring_write(const char *filename)
{
    int rc, file_fd;
    int i;
    int fd;
    struct io_uring ring;
    struct io_uring_params p;
    struct io_uring_sqe *sqe;
    struct io_uring_cqe *cqe;
    struct iovec iovecs[1];
    struct __kernel_timespec ts;
    off_t file_size;
    off_t offset;
    int fds[0];

    void *buf;

    unsigned long long num_reads;
    unsigned long long num_writes;

    ts.tv_sec = 1;
    ts.tv_nsec = 0;

    offset = 0;

    buf = malloc(buf_size);
    if (!buf)
    {
        perror("malloc");
        exit(1);
    }

    fd = open("/dev/null", O_WRONLY);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    file_fd = open(filename, O_RDONLY);
    if (file_fd < 0)
    {
        perror("open");
        exit(1);
    }

    fds[0] = fd;

    file_size = get_file_size(file_fd);
    num_reads = (unsigned long long) file_size / buf_size;
    if (file_size % buf_size) num_reads++;

    iovecs[0].iov_base = buf;
    iovecs[0].iov_len = buf_size;

    memset(&p, 0, sizeof(p));

    /*
    p.flags |= IORING_SETUP_SQPOLL;
    p.sq_thread_cpu = 3;
    p.sq_thread_idle = 5000;
    */

    if (io_uring_queue_init(1, &ring, 0) < 0)
    {
        perror("io_uring_queue_init");
        exit(1);
    }

    /*
    if (io_uring_queue_init_params(32, &ring, &p) < 0)
    {
        perror("io_uring_queue_init_params");
        exit(1);
    }
    */

    if (io_uring_register_files(&ring, &fd, 1) < 0)
    {
        perror("io_uring_register_files");
        exit(1);
    }

    if (io_uring_register_buffers(&ring, iovecs, 1) < 0)
    {
        perror("io_uring_register_buffers");
        exit(1);
    }

    start = get_time();
    for (i = 0; i < num_reads; i++)
    {
        rc = io_uring_sqring_wait(&ring);
        if (rc < 0)
        {
            if (rc == -EINVAL)
                perror("io_uring_sqring_wait: EINVAL");

            perror("io_uring_sqring_wait");
            close(fd);
            io_uring_queue_exit(&ring);
            exit(1);
        }
        sqe = io_uring_get_sqe(&ring);
        if (!sqe)
        {
            perror("io_uring_get_sqe");
            close(fd);
            io_uring_queue_exit(&ring);
            exit(1);
        }

        /*
        io_uring_prep_read(sqe, 0, iovecs[0].iov_base, buf_size, i * buf_size);
        */
        io_uring_prep_write_fixed(sqe, 0, iovecs[0].iov_base, buf_size, i * buf_size, 0);
        io_uring_sqe_set_flags(sqe, IOSQE_FIXED_FILE);

        io_uring_submit(&ring);

        rc = io_uring_wait_cqe(&ring, &cqe);
        if (rc < 0)
        {
            fprintf(stderr, "rc: %d\n", rc);
            perror("io_uring_wait_cqe");
            close(fd);
            io_uring_queue_exit(&ring);
            exit(1);
        }

        num_writes++;

        io_uring_cqe_seen(&ring, cqe);

    }
    end = get_time();

    printf("io_uring (num_writes=%llu): %f\n", num_writes, end - start);

    io_uring_unregister_files(&ring);
    io_uring_unregister_buffers(&ring);
    io_uring_queue_exit(&ring);
    close(fd);
    close(file_fd);

    free(buf);
}

int main() {
    const char *filename = "/home/w123694/build/iouring/src/bufcopy/75M.txt";

    measured_read(filename);
    measured_io_uring_read(filename);

    measured_write(filename);
    measured_io_uring_write(filename);

    return 0;
}