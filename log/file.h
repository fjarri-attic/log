#ifndef _FILE_H
#define _FILE_H

#include <Windows.h>

class File
{
	HANDLE FileHandle;
	void *FileName;
	bool FileNameIsUnicode;
	bool KeepClosed;
public:
	File(const void *file_name, bool name_is_unicode, bool keep_closed);
	~File();
	int Open();
	void Close();
	int Write(const void *buffer, size_t buffer_size);
};

#endif