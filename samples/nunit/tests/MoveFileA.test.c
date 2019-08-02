#include "../nunit.h"
#include <windows.h>

TEST(MoveFileA, NonExisting)
{
	BOOL result;
	result = MoveFileA("C:\\file_MoveFileA_NonExisting", "C:\\file_MoveFileA_NonExisting_Target");
	ASSERT_EQ(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_FILE_NOT_FOUND);
}

TEST(MoveFileA, ExistingTarget)
{
	HANDLE h;
	h = CreateFileA("C:\\file_MoveFileA_ExistingTarget_Source", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	CloseHandle(h);
	SetLastError(ERROR_SUCCESS);

	h = CreateFileA("C:\\file_MoveFileA_ExistingTarget", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	CloseHandle(h);
	SetLastError(ERROR_SUCCESS);

	BOOL result;
	result = MoveFileA("C:\\file_MoveFileA_ExistingTarget_Source", "C:\\file_MoveFileA_ExistingTarget");
	ASSERT_EQ(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_ALREADY_EXISTS);
}

TEST(MoveFileA, Valid)
{
	HANDLE h;
	DWORD bytesWritten;
	h = CreateFileA("C:\\file_MoveFileA_Valid", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	CloseHandle(h);
	SetLastError(ERROR_SUCCESS);

	BOOL result;
	result = MoveFileA("C:\\file_MoveFileA_Valid", "C:\\file_MoveFileA_Valid_Target");
	ASSERT_NE(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);
}
