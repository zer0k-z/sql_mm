#pragma once

#include <stdlib.h>
#include <string.h>

class BaseMemTable
{
public:
    BaseMemTable(unsigned int init_size)
    {
        membase = (unsigned char *)malloc(init_size);
        size = init_size;
        tail = 0;
    }

    ~BaseMemTable()
    {
        free(membase);
        membase = NULL;
    }

public:
    /**
     * Allocates 'size' bytes of memory.
     * Optionally outputs the address through 'addr'.
     * Returns an index >= 0 on success, < 0 on failure.
     */
    int CreateMem(unsigned int addsize, void **addr)
    {
        int idx = (int)tail;

        while (tail + addsize >= size)
        {
            size *= 2;
            membase = (unsigned char *)realloc(membase, size);
        }

        tail += addsize;
        if (addr)
        {
            *addr = (void *)&membase[idx];
        }

        return idx;
    }

    /**
     * Given an index into the memory table, returns its address.
     * Returns NULL if invalid.
     */
    void *GetAddress(int index)
    {
        if (index < 0 || (unsigned int)index >= tail)
        {
            return NULL;
        }
        return &membase[index];
    }

    /**
     * Scraps the memory table.  For caching purposes, the memory
     * is not freed, however subsequent calls to CreateMem() will
     * begin at the first index again.
     */
    void Reset()
    {
        tail = 0;
    }

    inline unsigned int GetMemUsage()
    {
        return size;
    }

    inline unsigned int GetActualMemUsed()
    {
        return tail;
    }

private:
    unsigned char *membase;
    unsigned int size;
    unsigned int tail;
};

class BaseStringTable
{
public:
    BaseStringTable(unsigned int init_size) : m_table(init_size) {}

public:
    /**
     * Adds a string to the string table and returns its index.
     */
    int AddString(const char *string)
    {
        return AddString(string, strlen(string));
    }

    /**
     * Adds a string to the string table and returns its index.
     */
    int AddString(const char *string, size_t length)
    {
        size_t len = length + 1;
        int idx;
        char *addr;

        idx = m_table.CreateMem(len, (void **)&addr);
        memcpy(addr, string, length + 1);
        return idx;
    }

    /**
     * Given an index into the string table, returns the associated string.
     */
    inline const char *GetString(int str)
    {
        return (const char *)m_table.GetAddress(str);
    }

    /**
     * Scraps the string table. For caching purposes, the memory
     * is not freed, however subsequent calls to AddString() will
     * begin at the first index again.
     */
    void Reset()
    {
        m_table.Reset();
    }

    /**
     * Returns the parent BaseMemTable that this string table uses.
     */
    inline BaseMemTable *GetMemTable()
    {
        return &m_table;
    }

private:
    BaseMemTable m_table;
};
