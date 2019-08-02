#include "../nunit.h"
#include <windows.h>

TEST(ReadFile, NormalRead)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_ReadFile_NormalRead", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);
	CloseHandle(h);

	h = CreateFileA("C:\\file_ReadFile_NormalRead", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	SetLastError(ERROR_SUCCESS);
	char buf[5];
	DWORD bytesRead;
	BOOL result;
	result = ReadFile(h, buf, 5, &bytesRead, NULL);
	ASSERT_TRUE(result);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);
	ASSERT_EQ(bytesRead, 3);
	ASSERT_EQ(buf[0], 'a');
	ASSERT_EQ(buf[1], 'b');
	ASSERT_EQ(buf[2], 'c');

	CloseHandle(h);
}

TEST(ReadFile, Eof)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_ReadFile_Eof", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);
	CloseHandle(h);

	h = CreateFileA("C:\\file_ReadFile_Eof", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	SetLastError(ERROR_SUCCESS);
	char buf[5];
	DWORD bytesRead;
	BOOL result;
	result = ReadFile(h, buf, 5, &bytesRead, NULL);
	ASSERT_TRUE(result);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);
	ASSERT_EQ(bytesRead, 3);

	result = ReadFile(h, buf, 5, &bytesRead, NULL);
	ASSERT_TRUE(result);
	ASSERT_EQ(bytesRead, 0);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);

	CloseHandle(h);
}
