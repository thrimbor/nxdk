#include "../nunit.h"
#include <windows.h>

TEST(GetFileSize, GetFileSize)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_GetFileSize", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);

	SetLastError(ERROR_SUCCESS);
	DWORD filesize = GetFileSize(h, NULL);
	ASSERT_EQ(filesize, 3);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);

	CloseHandle(h);
}

TEST(GetFileSize, InvalidHandle)
{
	SetLastError(ERROR_SUCCESS);
	DWORD filesize = GetFileSize(INVALID_HANDLE_VALUE, NULL);
	ASSERT_EQ(filesize, INVALID_FILE_SIZE);
}

TEST(GetFileSizeEx, GetFileSizeEx)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_GetFileSizeEx", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);

	SetLastError(ERROR_SUCCESS);
	LARGE_INTEGER fileSize;
	BOOL result;
	result = GetFileSizeEx(h, &fileSize);
	ASSERT_TRUE((result));
	ASSERT_EQ(fileSize.HighPart, 0);
	ASSERT_EQ(fileSize.LowPart, 3);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);

	CloseHandle(h);
}
