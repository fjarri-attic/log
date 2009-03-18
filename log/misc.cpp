#include "misc.h"
#include <Windows.h>

Buffer::Buffer() 
{
	Ptr = NULL;
	Size = 0;
	DataSize = 0;
}

Buffer::~Buffer()
{
	Free();
}

void Buffer::Free()
{
	if(Ptr)
	{
		delete Ptr;
		Ptr = NULL;
	}
}

void Buffer::Resize(size_t new_size, bool keep_data)
{
	char *new_ptr = new char[new_size];
	
	if(keep_data)
	{
		size_t size_to_copy = (new_size > DataSize ? DataSize : new_size);
		if(size_to_copy)
			memcpy(new_ptr, Ptr, size_to_copy);
	}

	Free();

	Ptr = new_ptr;
	DataSize = new_size;
	if(new_size > Size)
		Size = new_size;
}

void *Buffer::GetPtr()
{
	return (void *)Ptr;
}

size_t Buffer::GetDataSize()
{
	return DataSize;
}


//
void ExpandLF(Buffer &src_buf, Buffer &dst_buf)
{
	char *src = (char*)src_buf.GetPtr();
	size_t src_len = src_buf.GetDataSize();

	// Find number of \n's
	const char *p = src;
	int lf_num = 0;

	for(; *p != '\0'; p++)
		if(*p == '\n')
			lf_num++;

	// Expand destination buffer
	dst_buf.Resize((src_len + lf_num) * sizeof(char));

	char *dst = (char*)dst_buf.GetPtr();
	char *end = src + src_len * sizeof(char);
	char *prev = src;

	while(src != end)
	{
		if(*src != '\n')
		{
			src += sizeof(char);
			continue;
		}

		size_t size_to_copy = src - prev;
		if(size_to_copy)
		{
			memcpy(dst, prev, size_to_copy);
			dst += size_to_copy;
		}
		dst[0] = '\r';
		dst[1] = '\n';
		dst += 2 * sizeof(char);
		src += sizeof(char);
		prev = src;
	}
}


//
DWORD UnicodeToMbStr(Buffer &src, Buffer &dst)
{
	int t;

	// get the length of buffer to hold converted string
	t = WideCharToMultiByte(
		CP_THREAD_ACP,			// current thread's code page 
		0,						// flags
		(LPCWSTR)src.GetPtr(),	// source string
		-1,						// string is null-terminated
		NULL, 0,				// we need just length of necessary buffer  
		NULL, NULL);			// use default chars if some symbol cannot be converted 

	// if failed, return, doing nothing
	if(!t)
		return GetLastError();

	// allocate buffer for resulting string
	dst.Resize(t);

	// perform conversion
	if(!WideCharToMultiByte(CP_THREAD_ACP, 0, (LPCWSTR)src.GetPtr(), -1, (LPSTR)dst.GetPtr(), t, NULL, NULL))
		return GetLastError();

	return 0;
}