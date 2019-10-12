#ifndef __CHUNKREADER_H__
#define __CHUNKREADER_H__

#include <unistd.h>
#include <errno.h>
#include <string.h>


class ChunkReader
{
    const size_t m_chunk_size;
    const size_t m_size;
    const int m_num_chunks;
    bool m_from_file;
    int m_fd;
    const off_t m_offset;
    const char *m_data;
    int m_cur_chunk_idx;

public:
    ChunkReader(const char *data, const off_t offset, const size_t size, const size_t chunk_size);
    ChunkReader(const int fd, const off_t offset, const size_t size, const size_t chunk_size);

    bool has_next();
    ssize_t read_next(char *buffer);
    int cur_chunk_idx();

    ~ChunkReader();
};

#endif