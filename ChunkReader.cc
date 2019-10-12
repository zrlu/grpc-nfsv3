#include "ChunkReader.h"

#include <iostream>
#include <stdio.h>

ChunkReader::ChunkReader(const char *data, const off_t offset, const size_t size, const size_t chunk_size):
m_chunk_size(chunk_size),
m_size(size),
m_num_chunks(((int)size / (int)chunk_size) + 1),
m_from_file(false),
m_data(data),
m_offset(offset),
m_cur_chunk_idx(0)
{
}

ChunkReader::ChunkReader(int fd, const off_t offset, const size_t size, const size_t chunk_size):
m_chunk_size(chunk_size),
m_size(
    [fd](){
        struct stat statbuf;
        fstat(fd, &statbuf);
        return statbuf.st_size;
    }()
),
m_num_chunks(((int)m_size / (int)m_chunk_size) + 1),
m_from_file(true),
m_fd(fd),
m_offset(offset),
m_cur_chunk_idx(0)
{

}

bool ChunkReader::has_next()
{
    return m_cur_chunk_idx < m_num_chunks;
}

ssize_t ChunkReader::read_next(char *buffer)
{
    if (m_from_file)
    {
        if (lseek(m_fd, m_offset + m_cur_chunk_idx*m_chunk_size, SEEK_SET) == -1) return -1;
        int retval = read(m_fd, buffer, std::min(m_size, m_chunk_size));
        if (retval == -1) return -1;
        m_cur_chunk_idx++;
        return retval;
    } else {
        bool last = m_cur_chunk_idx == m_num_chunks - 1;
        int length = last ? m_size % m_chunk_size : m_chunk_size;
        memcpy(buffer, m_data + m_cur_chunk_idx*m_chunk_size, length);
        m_cur_chunk_idx++;
        return length;
    }
}

int ChunkReader::cur_chunk_idx()
{
    return m_cur_chunk_idx;
}

ChunkReader::~ChunkReader()
{
}