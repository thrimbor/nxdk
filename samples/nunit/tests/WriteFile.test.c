#include "../nunit.h"
#include <windows.h>

TEST(WriteFile, NormalWrite)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_WriteFile_NormalWrite", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	BOOL result;
	SetLastError(ERROR_SUCCESS);
	result = WriteFile(h, "abc", 3, &bytesWritten, NULL);
	ASSERT_TRUE(result);
	ASSERT_EQ(bytesWritten, 3);

	CloseHandle(h);
}
