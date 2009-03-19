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
	void *GetPtr() const;
	size_t GetDataSize() const;
};

void ExpandLF(const Buffer &src_buf, Buffer &dst_buf);
int UnicodeToMbStr(const Buffer &src, Buffer &dst);

#endif
