#include "../nunit.h"
#include <windows.h>

TEST(CreateDirectoryA, Valid)
{
	BOOL result;
	SetLastError(ERROR_SUCCESS);
	result = CreateDirectoryA("C:\\file_CreateDirectoryA_Valid", NULL);
	ASSERT_NE(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);
}

TEST(CreateDirectoryA, ExistingDir)
{
	BOOL result;
	SetLastError(ERROR_SUCCESS);
	result = CreateDirectoryA("C:\\file_CreateDirectoryA_ExistingDir", NULL);
	ASSERT_NE(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);

	result = CreateDirectoryA("C:\\file_CreateDirectoryA_ExistingDir", NULL);
	ASSERT_EQ(result, 0);
	ASSERT_EQ(GetLastError(), ERROR_ALREADY_EXISTS);
}
