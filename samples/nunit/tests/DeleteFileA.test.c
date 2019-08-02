#include "../nunit.h"
#include <windows.h>

TEST(DeleteFileA, NonExisting)
{
	BOOL result;
	result = DeleteFileA("C:\\file_DeleteFile_NonExisting");
	ASSERT_EQ(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_FILE_NOT_FOUND);
}

TEST(DeleteFileA, ExistingFile)
{
	HANDLE h;
	h = CreateFileA("C:\\file_DeleteFile_ExistingFile", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	CloseHandle(h);
	SetLastError(ERROR_SUCCESS);
	BOOL result;
	result = DeleteFileA("C:\\file_DeleteFile_ExistingFile");
	ASSERT_NE(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);
}
