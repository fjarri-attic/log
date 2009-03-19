#include "file.h"


File::File( const void *file_name, bool name_is_unicode, bool keep_closed )
{
	size_t buffer_size;

	FileHandle = INVALID_HANDLE_VALUE;
	FileName = NULL;
	FileNameIsUnicode = name_is_unicode;
	KeepClosed = keep_closed;

	if(name_is_unicode)
		buffer_size = (wcsnlen((const wchar_t *)file_name, MAX_PATH) + 1) * sizeof(wchar_t);
	else
		buffer_size = (strnlen((const char *)file_name, MAX_PATH) + 1) * sizeof(char);

	FileName = new char[buffer_size];
	memcpy(FileName, file_name, buffer_size);
}

File::~File()
{
	if(FileHandle != INVALID_HANDLE_VALUE)
		CloseHandle(FileHandle);

	if(FileName)
		delete[] FileName;
}

int File::Open()
{
	if(FileHandle == INVALID_HANDLE_VALUE)
	{
		if(FileNameIsUnicode)
			FileHandle = CreateFileW((wchar_t *)FileName, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		else
			FileHandle = CreateFileA((char *)FileName, FILE_APPEND_DATA, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if(FileHandle == INVALID_HANDLE_VALUE) 
			return GetLastError();
	}

	return 0;
}

void File::Close()
{
	if(KeepClosed && (FileHandle != INVALID_HANDLE_VALUE))
	{
		CloseHandle(FileHandle);
		FileHandle = INVALID_HANDLE_VALUE;
	}
}

int File::Write( const void *buffer, size_t buffer_size )
{
	DWORD res = Open();
	if(res)
		return res;

	DWORD size_written;
	if(!WriteFile(FileHandle, buffer, (DWORD)buffer_size, &size_written, NULL) || size_written != (DWORD)buffer_size)
	{
		Close();
		return GetLastError();
	}

	Close();
	return 0;
}
