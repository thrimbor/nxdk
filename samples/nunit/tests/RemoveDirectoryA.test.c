#include "../nunit.h"
#include <windows.h>

TEST(RemoveDirectoryA, Valid)
{
	BOOL result;
	SetLastError(ERROR_SUCCESS);
	result = CreateDirectoryA("C:\\file_RemoveDirectoryA_Valid", NULL);
	ASSERT_NE(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);

	result = RemoveDirectoryA("C:\\file_RemoveDirectoryA_Valid");
	ASSERT_NE(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);
}

TEST(RemoveDirectoryA, NonExisting)
{
	BOOL result;
	SetLastError(ERROR_SUCCESS);
	result = RemoveDirectoryA("C:\\file_RemoveDirectoryA_NonExisting");
	ASSERT_EQ(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_FILE_NOT_FOUND);
}

TEST(RemoveDirectoryA, NotADir)
{
	HANDLE h;
	h = CreateFileA("C:\\file_RemoveDirectoryA_NotADir", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	CloseHandle(h);
	SetLastError(ERROR_SUCCESS);

	BOOL result;
	result = RemoveDirectoryA("C:\\file_RemoveDirectoryA_NotADir");
	ASSERT_EQ(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_DIRECTORY);
}
