#include "NFSConnection.hpp"
#include "NFSServer.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <thread>

extern "C"
{
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
}

void test_server();
void client1_test();
void client2_test();

int main() {
    std::system("rm -rf examples/acceptance_test/testfs/*");
    std::system("mkdir examples/acceptance_test/testfs/testdir");

    std::thread server(test_server);
    sleep(1);
    std::thread client1(client1_test);
    sleep(1);
    std::thread client2(client2_test);

    client1.join();
    client2.join();

    sleep(1);
    std::system("rm -rf examples/acceptance_test/testfs/*");
    std::cerr << "TEST PASSED" << std::endl;
    std::exit(0);
}

void test_server() {
    nfs::NFSServer("examples/acceptance_test/test_server.conf").run();
}

void client1_test() {
    srand(time(NULL));

    nfs:nfs::NFSConnection connection;
    nfs::ConnectReturn creturn = connection.connect(
        "localhost",
        "nfstest",
        "nfstest",
        "test"
    );
    if (creturn != nfs::ConnectReturn::OK) {
        std::cerr << "client1 failed to connect" << std::endl;
        throw;
    }


    int fd = connection.open("test_file", O_WRONLY | O_CREAT, S_IRWXU);
    if (fd < 0) {
        std::cerr << "client1 failed to open test_file " << connection.get_error() << std::endl;
        throw;
    }


    std::vector<char> test_file;
    int size = 100 + (rand() % 1000);
    for (int i = 0; i < size; ++i) {
        test_file.push_back(static_cast<char>(rand()));
    }
    int wreturn = connection.write(fd, test_file.data(), test_file.size());
    if (wreturn != size) {
        std::cerr << "client1 failed to write test_file: " << connection.get_error() << std::endl;
        throw;
    }


    int rclose = connection.close(fd);
    if (rclose < 0) {
        std::cerr << "client1 failed to close test_file: " << connection.get_error() << std::endl;
        throw;
    }


    int fd2 = connection.open("test_file", O_RDONLY, 0);
    if (fd2 < 0) {
        std::cerr << "client1 failed to open test_file " << connection.get_error() << std::endl;
        throw;
    }


    std::vector<char> read_file;
    for (int i = 0; i < size; ++i) {
        read_file.push_back(0);
    }
    int rreturn = connection.read(fd2, read_file.data(), read_file.size());
    if (rreturn != size) {
        std::cerr << "client1 failed to read test_file: " << connection.get_error() << std::endl;
        throw;
    }
    if (read_file != test_file) {
        std::cerr << "client1 read_file is different than write_file" << std::endl;
        throw;
    }


    int rclose2 = connection.close(fd2);
    if (rclose2 < 0) {
        std::cerr << "client1 failed to close read_file: " << connection.get_error() << std::endl;
        throw;
    }


    int fd3 = connection.open("test_file", O_RDONLY, 0);
    if (fd3 < 0) {
        std::cerr << "client1 failed to open test_file " << connection.get_error() << std::endl;
        throw;
    }


    int rlseek = connection.lseek(fd3, 0, SEEK_END);
    if (rlseek != size) {
        std::cerr << "client1 failed to lseek test_file: " << connection.get_error() << std::endl;
        throw;
    }


    int rclose3 = connection.close(fd3);
    if (rclose3 < 0) {
        std::cerr << "client1 failed to close read_file: " << connection.get_error() << std::endl;
        throw;
    }


    int fd4 = connection.open("test_file", O_RDONLY, 0);
    if (fd4 < 0) {
        std::cerr << "client1 failed to open test_file " << connection.get_error() << std::endl;
        throw;
    }


    struct stat statbuf;
    int rfstat = connection.fstat(fd4, &statbuf);
    if (rfstat < 0) {
        std::cerr << "client1 failed to fstat test_file: " << connection.get_error() << std::endl;
        throw;
    }
    struct stat localstat;
    int lfd = open("examples/acceptance_test/testfs/test_file", O_RDONLY, 0);
    if (lfd < 0) {
        std::cerr << "client1 failed to open test_file locally: " << errno << std::endl;
        throw;
    }
    int lrfstat = fstat(lfd, &localstat);
    if (lrfstat < 0) {
        std::cerr << "client1 failed to fstat test_file locally: " << errno << std::endl;
        throw;
    }
    if (
        statbuf.st_dev != localstat.st_dev ||
        statbuf.st_ino != localstat.st_ino ||
        statbuf.st_mode != localstat.st_mode ||
        statbuf.st_nlink != localstat.st_nlink ||
        statbuf.st_uid != localstat.st_uid ||
        statbuf.st_gid != localstat.st_gid ||
        statbuf.st_rdev != localstat.st_rdev ||
        statbuf.st_size != localstat.st_size ||
        statbuf.st_blksize != localstat.st_blksize ||
        statbuf.st_blocks != localstat.st_blocks
    ) {
        std::cerr << "client1 fstat is different form local fstat" << std::endl;
        throw;
    }


    int rclose4 = connection.close(fd4);
    if (rclose4 < 0) {
        std::cerr << "client1 failed to close read_file: " << connection.get_error() << std::endl;
        throw;
    }


    int runlink = connection.unlink("test_file");
    if (runlink < 0) {
        std::cerr << "client1 failed to unlink test_file: " << connection.get_error() << std::endl;
        throw;
    }
    int unlinkfd = open("examples/acceptance_test/testfs/test_file", O_RDONLY, 0);
    if (unlinkfd != -1) {
        std::cerr << "client1 unlink didnt delete file test_file" << std::endl;
        throw;
    }
    if (errno != ENOENT) {
        std::cerr << "client1 local unlink check failed for wrong reason: " << errno << std::endl;
        throw;
    }


    int fddirfstat = connection.open("testdir", O_RDONLY, 0);
    if (fddirfstat < 0) {
        std::cerr << "client1 failed to open testdir: " << connection.get_error() << std::endl;
        throw;
    }
    struct stat dirfstat;
    int rdiffstat = connection.fstat(fddirfstat, &dirfstat);
    if (rdiffstat < 0) {
        std::cerr << "client1 failed to fstat testdir: " << connection.get_error() << std::endl;
        throw;
    }
    struct stat localstatdir;
    int lfddir = open("examples/acceptance_test/testfs/testdir", O_RDONLY, 0);
    if (lfddir < 0) {
        std::cerr << "client1 failed to open testdir locally: " << errno << std::endl;
        throw;
    }
    int lrfstatdir = fstat(lfddir, &localstatdir);
    if (lrfstatdir < 0) {
        std::cerr << "client1 failed to fstat testdir locally: " << errno << std::endl;
        throw;
    }
    if (
        localstatdir.st_dev != dirfstat.st_dev ||
        localstatdir.st_ino != dirfstat.st_ino ||
        localstatdir.st_mode != dirfstat.st_mode ||
        localstatdir.st_nlink != dirfstat.st_nlink ||
        localstatdir.st_uid != dirfstat.st_uid ||
        localstatdir.st_gid != dirfstat.st_gid ||
        localstatdir.st_rdev != dirfstat.st_rdev ||
        localstatdir.st_size != dirfstat.st_size ||
        localstatdir.st_blksize != dirfstat.st_blksize ||
        localstatdir.st_blocks != dirfstat.st_blocks
    ) {
        std::cerr << "client1 fstat is different form local fstat" << std::endl;
        throw;
    }


    int fdflock = connection.open("flock_test", O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
    if (fdflock < 0) {
        std::cerr << "client1 failed to open flock_test: " << connection.get_error() << std::endl;
        throw;
    }
    int rflock = connection.flock(fdflock, LOCK_EX);
    if (rflock < 0) {
        std::cerr << "client1 failed to flock flock_test: " << connection.get_error() << std::endl;
        throw;
    }
    int rflockwrite = connection.write(fdflock, "1", 1);
    if (rflockwrite != 1) {
        std::cerr << "client1 failed to write to flock_test: " << connection.get_error() << std::endl;
        throw;
    }
    sleep(2);
    int rflockwrite2 = connection.write(fdflock, "1", 1);
    if (rflockwrite2 != 1) {
        std::cerr << "client1 failed to write to flock_test: " << connection.get_error() << std::endl;
        throw;
    }
    int rflock2 = connection.flock(fdflock, LOCK_UN);
    if (rflock2 < 0) {
        std::cerr << "client1 failed to unflock flock_test: " << connection.get_error() << std::endl;
        throw;
    }
    int flockclose = connection.close(fdflock);
    if (flockclose < 0) {
        std::cerr << "client1 failed to close flock_test: " << connection.get_error() << std::endl;
        throw;
    }
    int fdflock2 = connection.open("flock_test", O_RDONLY, 0);
    if (fdflock < 0) {
        std::cerr << "client1 failed to open flock_test: " << connection.get_error() << std::endl;
        throw;
    }
    char readflock_buff[5] = {0, 0, 0, 0, 0};
    int readflock = connection.read(fdflock2, &readflock_buff, 4);
    if (readflock != 3) {
        std::cerr << "client1 failed to read flock_test: " << connection.get_error() << std::endl;
        throw;
    }
    if (strcmp("112", readflock_buff) != 0) {
        std::cerr << "client1 failed the flock_test: " << readflock_buff << " instead of 112"  << std::endl;
        throw;
    }
    int readflockclose = connection.close(fdflock2);
    if (readflock < 0) {
        std::cerr << "client1 failed to close flock_test: " << connection.get_error() << std::endl;
        throw;
    }
}

void client2_test() {
    nfs:nfs::NFSConnection connection;
    nfs::ConnectReturn creturn = connection.connect(
        "localhost",
        "nfstest",
        "nfstest",
        "test"
    );
    if (creturn != nfs::ConnectReturn::OK) {
        std::cerr << "client2 failed to connect" << std::endl;
        throw;
    }


    int fdflock = connection.open("flock_test", O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
    if (fdflock < 0) {
        std::cerr << "client2 failed to open flock_test: " << errno << std::endl;
        throw;
    }

    int rflock = connection.flock(fdflock, LOCK_EX);
    if (rflock < 0) {
        std::cerr << "client2 failed to flock flock_test: " << connection.get_error() << std::endl;
        throw;
    }

    int rwrite = connection.write(fdflock, "2", 1);
    if (rwrite != 1) {
        std::cerr << "client2 failed to write to flock_test: " << errno << std::endl;
        throw;
    }

    int rflock2 = connection.flock(fdflock, LOCK_UN);
    if (rflock2 < 0) {
        std::cerr << "client2 failed to unflock flock_test: " << connection.get_error() << std::endl;
        throw;
    }

    int rclose = connection.close(fdflock);
    if (rclose < 0) {
        std::cerr << "client2 failed to close flock_test: " << errno << std::endl;
        throw;
    }
}