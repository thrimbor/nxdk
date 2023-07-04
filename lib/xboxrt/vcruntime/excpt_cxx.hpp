#ifndef __VCRUNTIME_EXCPT_CXX_H__
#define __VCRUNTIME_EXCPT_CXX_H__

#include <excpt.h>

struct TypeDescriptor
{
    const void *pVFTable;
    void *spare;
    char name[0];
};

struct UnwindMapEntry
{
    int toState;
    void (*action)();
};

struct HandlerType
{
    DWORD adjectives;
    TypeDescriptor *pType;
    int dispCatchObj; // ebp-relative offset to the memory reserved for the exception object
    void *addressOfHandler;
};
#define TYPE_FLAG_CONST 1
#define TYPE_FLAG_VOLATILE 2
#define TYPE_FLAG_REFERENCE 8

struct TryBlockMapEntry
{
    int tryLow;
    int tryHigh;
    int catchHigh;
    int nCatches;
    HandlerType *pHandlerArray;
};

struct ESTypeList
{
    int nCount;
    HandlerType *pTypeArray;
};

struct FuncInfo
{
    DWORD magicNumber;
    int maxState;
    UnwindMapEntry *pUnwindMap;
    DWORD nTryBlocks;
    TryBlockMapEntry *pTryBlockMap;
    DWORD nIPMapEntries;
    void *pIPtoStateMap;
    ESTypeList *pESTypeList;
    int EHFlags;
};

struct PMD
{
    int mdisp; // member offset
    int pdisp; // offset of the vbtable
    int vdisp; // offset to the displacement value inside the vbtable
};

struct CatchableType
{
    DWORD properties; // 0x01: simple type, can be copied by memmove, 0x02: can be caught by reference only, 0x04: has virtual bases
    TypeDescriptor *pType;
    PMD thisDisplacement;
    int sizeOrOffset;
    void (*copyFunction)(); // copy constructor address (0 = trivially copyable)
};
#define CLASS_IS_SIMPLE_TYPE 1
#define CLASS_HAS_VIRTUAL_BASE 4

struct CatchableTypeArray
{
    int nCatchableTypes;
    CatchableType *arrayOfCatchableTypes[0];
};

struct _ThrowInfo
{

    DWORD attributes; // 0x01: const, 0x02: volatile
    void (*pmfnUnwind)(); // exception destructor
    int (*pForwardCompat)(); // forward compatibility handler
    CatchableTypeArray *pCatchableTypeArray;
};

#endif
