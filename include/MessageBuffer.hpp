#include <cstdint>
#include <vector>
#include <cstdlib>

extern "C"
{
#include <sys/stat.h>
}

namespace nfs
{

    class MessageBuffer
    {
    public:
        MessageBuffer();

        MessageBuffer(char *data, uint64_t data_size);

        char *data();

        size_t size();

        void push_uint8_t(uint8_t code);

        void push_int64_t(int64_t data);

        int64_t pop_int64_t();

        void push_uint64_t(uint64_t data);

        uint64_t pop_uint64_t();

        void push_char_data(char *data, uint64_t data_size);

        std::vector<char> pop_char_data(uint64_t data_size);

        void push_stat(struct stat &statbuf);

        struct stat pop_stat();

    private:
        std::vector<char> buffer;
    };

}