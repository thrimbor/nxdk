#include "../nunit.h"
#include <windows.h>

TEST(CreateFileA, NullString)
{
	HANDLE h;
	h = CreateFileA(NULL, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_EQ(h, INVALID_HANDLE_VALUE);
	ASSERT_EQ(GetLastError(), ERROR_PATH_NOT_FOUND);
}

TEST(CreateFileA, EmptyString)
{
	HANDLE h;
	h = CreateFileA("", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_EQ(h, INVALID_HANDLE_VALUE);
	ASSERT_EQ(GetLastError(), ERROR_PATH_NOT_FOUND);
}

// POSIX semantics test

TEST(CreateFileA, InvalidDisposition)
{
	HANDLE h;
	h = CreateFileA("C:\\file_CreateFileA_InvalidDispositon", GENERIC_WRITE, 0, NULL, 0, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_EQ(h, INVALID_HANDLE_VALUE);
	ASSERT_EQ(GetLastError(), ERROR_INVALID_PARAMETER);
}

TEST(CreateFileA, TruncateWithoutWrite)
{
	HANDLE h;
	h = CreateFileA("C:\\file_CreateFileA_TruncateWithoutWrite", GENERIC_READ, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_EQ(h, INVALID_HANDLE_VALUE);
	ASSERT_EQ(GetLastError(), ERROR_INVALID_PARAMETER);
}

TEST(CreateFileA, ValidWriteOpen)
{
	HANDLE h;
	h = CreateFileA("C:\\file_CreateFileA_ValidWriteOpen", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);
	CloseHandle(h);
}

TEST(CreateFileA, CreateAlwaysOpenTwice)
{
	HANDLE h;
	h = CreateFileA("C:\\file_CreateFileA_CreateAlwaysOpenTwice", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	ASSERT_EQ(GetLastError(), ERROR_SUCCESS);
	CloseHandle(h);
	h = CreateFileA("C:\\file_CreateFileA_CreateAlwaysOpenTwice", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	ASSERT_NE(h, INVALID_HANDLE_VALUE);
	ASSERT_EQ(GetLastError(), ERROR_ALREADY_EXISTS);
	CloseHandle(h);
}
