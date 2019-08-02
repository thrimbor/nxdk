#include "../nunit.h"
#include <windows.h>

TEST(SetFilePointerEx, CurrentPos)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_SetFilePointerEx_CurrentPos", GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);

	SetLastError(ERROR_SUCCESS);
	BOOL result;
	LARGE_INTEGER distanceToMove;
	distanceToMove.QuadPart = -2;
	LARGE_INTEGER newFilePointer;
	result = SetFilePointerEx(h, distanceToMove, &newFilePointer, FILE_CURRENT);
	ASSERT_TRUE(result);
	ASSERT_EQ(newFilePointer.HighPart, 0);
	ASSERT_EQ(newFilePointer.LowPart, 1);
	char buf;
	DWORD bytesRead;
	ReadFile(h, &buf, 1, &bytesRead, NULL);
	ASSERT_EQ(buf, 'b');

	CloseHandle(h);
}

TEST(SetFilePointerEx, BeginPos)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_SetFilePointerEx_BeginPos", GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);

	SetLastError(ERROR_SUCCESS);
	BOOL result;
	LARGE_INTEGER distanceToMove;
	distanceToMove.QuadPart = 2;
	LARGE_INTEGER newFilePointer;
	result = SetFilePointerEx(h, distanceToMove, &newFilePointer, FILE_BEGIN);
	ASSERT_TRUE(result);
	ASSERT_EQ(newFilePointer.HighPart, 0);
	ASSERT_EQ(newFilePointer.LowPart, 2);
	char buf;
	DWORD bytesRead;
	ReadFile(h, &buf, 1, &bytesRead, NULL);
	ASSERT_EQ(buf, 'c');

	CloseHandle(h);
}

TEST(SetFilePointerEx, EndPos)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_SetFilePointerEx_EndPos", GENERIC_WRITE | GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	WriteFile(h, "abc", 3, &bytesWritten, NULL);

	SetLastError(ERROR_SUCCESS);
	BOOL result;
	LARGE_INTEGER distanceToMove;
	distanceToMove.QuadPart = -2;
	LARGE_INTEGER newFilePointer;
	result = SetFilePointerEx(h, distanceToMove, &newFilePointer, FILE_END);
	ASSERT_TRUE(result);
	ASSERT_EQ(newFilePointer.HighPart, 0);
	ASSERT_EQ(newFilePointer.LowPart, 1);
	char buf;
	DWORD bytesRead;
	ReadFile(h, &buf, 1, &bytesRead, NULL);
	ASSERT_EQ(buf, 'b');

	CloseHandle(h);
}
