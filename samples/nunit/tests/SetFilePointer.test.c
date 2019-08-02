#include "../nunit.h"
#include <windows.h>

TEST(SetFilePointer, CurrentPos)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_SetFilePointer_CurrentPos", GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);

	SetLastError(ERROR_SUCCESS);
	DWORD posInfo;
	posInfo = SetFilePointer(h, -2, NULL, FILE_CURRENT);
	ASSERT_EQ(posInfo, 1);
	char buf;
	DWORD bytesRead;
	ReadFile(h, &buf, 1, &bytesRead, NULL);
	ASSERT_EQ(buf, 'b');

	CloseHandle(h);
}

TEST(SetFilePointer, BeginPos)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_SetFilePointer_BeginPos", GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);

	SetLastError(ERROR_SUCCESS);
	DWORD posInfo;
	posInfo = SetFilePointer(h, 2, NULL, FILE_BEGIN);
	ASSERT_EQ(posInfo, 2);
	char buf;
	DWORD bytesRead;
	ReadFile(h, &buf, 1, &bytesRead, NULL);
	ASSERT_EQ(buf, 'c');

	CloseHandle(h);
}

TEST(SetFilePointer, EndPos)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_SetFilePointer_EndPos", GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);

	SetLastError(ERROR_SUCCESS);
	DWORD posInfo;
	posInfo = SetFilePointer(h, -2, NULL, FILE_END);
	ASSERT_EQ(posInfo, 1);
	char buf;
	DWORD bytesRead;
	ReadFile(h, &buf, 1, &bytesRead, NULL);
	ASSERT_EQ(buf, 'b');

	CloseHandle(h);
}
