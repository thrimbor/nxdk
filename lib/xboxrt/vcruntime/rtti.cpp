// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2024 Stefan Schmidt

#include <excpt_cxx.hpp>
#include <vcruntime_typeinfo.h>
#include <windef.h>

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
    /// Always zero?
    DWORD signature;

    /// The offset of this object within the complete object
    DWORD offset;

    DWORD cdOffset;

    /// The object's type descriptor, for type and type name comparison
    TypeDescriptor *pTypeDescriptor;

    RTTIClassHierarchyDescriptor *pClassDescriptor;
};

/**
 * Simple helper to turn a polymorphic object pointer into a vftable pointer
 * @param objptr Pointer to a polymorphic object
 * @return Pointer to the vftable (DWORD array)
*/
static const DWORD *get_vftable (void *objptr) {
    return *static_cast<const DWORD **>(objptr);
}

/**
 * Simple helper to obtain the complete object locator pointer of a polymorphic object
 * @param objptr Pointer to a polymorphic object
 * @return Pointer to the RTTICompleteObjectLocator of the object
*/
static const RTTICompleteObjectLocator *getobjlocator (void *objptr) {
    const DWORD *vftable = get_vftable(objptr);
    return reinterpret_cast<const RTTICompleteObjectLocator *>(vftable[-1]);
}

/**
 * Runtime function to implement casting a polymorphic object to a void pointer
 * dynamic_cast<void *>(obj_ptr)
 * @param objptr A pointer to a polymorphic object
 * @return nullptr if objptr was null, otherwise the true start address of the object
*/
extern "C" __cdecl PVOID __RTCastToVoid (PVOID objptr) {
    if (objptr == nullptr) {
        return nullptr;
    }

    __try {
        const RTTICompleteObjectLocator *objlocator = getobjlocator(objptr);
        return static_cast<char *>(objptr) - objlocator->offset;
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        throw std::__non_rtti_object::__construct_from_string_literal("Access violation - no RTTI data!");
    }

}
