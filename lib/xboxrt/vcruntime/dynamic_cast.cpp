#include <windows.h>
#include <excpt_cxx.hpp>

struct RTTIBaseClassDescriptor
{
    TypeDescriptor *pTypeDescriptor;
    DWORD numContainedBases;
    PMD where;
    DWORD attributes;
};

struct RTTIBaseClassArray
{
    RTTIBaseClassDescriptor *arrayOfBaseClassDescriptors[];
};

struct RTTIClassHierarchyDescriptor
{
    DWORD signature;
    DWORD attributes;
    DWORD numBaseClasses;
    RTTIBaseClassArray *pBaseClassArray;
};

struct RTTICompleteObjectLocator
{
    DWORD signature;
    DWORD offset;
    DWORD cdOffset;
    TypeDescriptor *pTypeDescriptor;
    RTTIClassHierarchyDescriptor *pClassDescriptor;
};

static const DWORD *get_vftable (void *objptr)
{
    return *static_cast<const DWORD **>(objptr);
}

static const RTTICompleteObjectLocator *getobjlocator (void *objptr)
{
    const DWORD *vftable = get_vftable(objptr);
    return reinterpret_cast<const RTTICompleteObjectLocator *>(vftable[-1]);
}

extern "C" __cdecl void *__RTCastToVoid (void *objptr)
{
    if (!objptr) return nullptr;

    __try {
        const RTTICompleteObjectLocator *objlocator = getobjlocator(objptr);
        return static_cast<char *>(objptr) - objlocator->offset;
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        throw std::__non_rtti_object::__construct_from_string_literal("Access violation - no RTTI data!");
    }
}
