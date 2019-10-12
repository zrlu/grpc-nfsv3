#include "ChunkReader.h"

#include <iostream>
#include <stdio.h>

ChunkReader::ChunkReader(const char *buffer, const off_t offset, const size_t size, const size_t chunk_size):
m_chunk_size(chunk_size),
m_size(size),
m_num_chunks(((int)size / (int)chunk_size) + 1),
m_from_file(false),
m_buffer(buffer),
m_offset(offset),
m_cur_chunk_idx(0)
{
}

ChunkReader::ChunkReader(int fd, const off_t offset, const size_t size, const size_t chunk_size):
m_chunk_size(chunk_size),
m_size(size),
m_num_chunks(((int)size / (int)chunk_size) + 1),
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
        int retval = read(m_fd, buffer, m_chunk_size);
        if (retval == -1) return -1;
        m_cur_chunk_idx++;
        return retval;
    } else {
        int start = m_cur_chunk_idx*m_chunk_size;
        int end = std::min( m_offset + (m_cur_chunk_idx+1)*m_chunk_size, m_size );
        std::string(buffer).substr(start, end).copy(buffer, end - start);
        return end - start;
    }
}

int ChunkReader::cur_chunk_idx()
{
    return m_cur_chunk_idx;
}

ChunkReader::~ChunkReader()
{
}