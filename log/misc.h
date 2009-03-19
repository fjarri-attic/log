#ifndef _MISC_H
#define _MISC_H

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
	void Resize(size_t new_size, bool keep_data = false);
	void *GetPtr();
	size_t GetDataSize();
};

void ExpandLF(Buffer &src_buf, Buffer &dst_buf);
int UnicodeToMbStr(Buffer &src, Buffer &dst);

#endif