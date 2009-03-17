#ifndef _MISC_H
#define _MISC_H

#include <stdlib.h>
#include <memory.h>
#include <Windows.h>

class Buffer
{
private:
	char *Ptr;
	size_t Size;
	size_t DataSize;
public:
	Buffer();
	~Buffer();
	void Free();
	void Resize(size_t new_size);
	void *GetPtr();
	size_t GetDataSize();
};

void ExpandLF(Buffer &src_buf, Buffer &dst_buf);
DWORD UnicodeToMbStr(Buffer &src, Buffer &dst);

#endif