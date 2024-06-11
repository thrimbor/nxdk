// SPDX-License-Identifier: MIT

// SPDX-FileCopyrightText: 2024 Stefan Schmidt

#include <excpt_cxx.hpp>
#include <string.h>
#include <vcruntime_typeinfo.h>
#include <windef.h>

// LLVM seems to have issues resolving runtime functions that are pulled in by
// other runtime functions, so we manually force inclusion
#pragma comment(linker, "/include:__CxxThrowException@8")

struct RTTIClassHierarchyDescriptor;

/**
 * Descriptor for a base class in an object's inheritance hierarchy
*/
struct RTTIBaseClassDescriptor
{
    /// RTTI Type Descriptor for this class
    TypeDescriptor *pTypeDescriptor;

    /// Specifies how many base classes this class has. This counts both
    /// directly and indirectly inherited bases.
    DWORD numContainedBases;

    PMD where;

    /// Attributes, such as whether the inheritance type is private/protected
    DWORD attributes;

    /// Optional, check for existence with BCD_HASPCHD attribute before access
    RTTIClassHierarchyDescriptor *pClassDescriptor;
};

#define BCD_NOTVISIBLE 0x00000001
#define BCD_AMBIGUOUS 0x00000002
#define BCD_PRIVORPROTBASE 0x00000004
#define BCD_HASPCHD 0x00000040

struct RTTIBaseClassArray
{
    /// The array of Base Class Descriptors of all base classes of the type,
    /// sorted left-to-right and depth-first.
    RTTIBaseClassDescriptor *arrayOfBaseClassDescriptors[];
};

/**
 * Describes the inheritance hierarchy of a C++ class
*/
struct RTTIClassHierarchyDescriptor
{
    /// Always zero?
    DWORD signature;

    /// Whether it's single, multiple or even virtual inheritance
    DWORD attributes;

    /// Number of elements in pBaseClassArray
    DWORD numBaseClasses;

    /// Struct that contains the array of base classes
    RTTIBaseClassArray *pBaseClassArray;
};

#define CHD_MULTINH 0x00000001
#define CHD_VIRTINH 0x00000002

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
#include <hal/debug.h>

/**
 * Helper to find the base class descriptor of the target class in an object's
 * inheritance chain, if it exists. Also ensures that the inheritance type
 * between source and target class allows casting.
 * @param obj_locator Complete object locator of the object which is being cast
 * @param source_type Type descriptor of the source class
 * @param target_type Type descriptor of the target class
 * @return nullptr if the cast fails, else Base Class Descriptor of the target class
*/
const RTTIBaseClassDescriptor *get_target_type_si (const RTTICompleteObjectLocator *obj_locator, const TypeDescriptor *source_type, const TypeDescriptor *target_type)
{
    // Search for the base class by comparing TypeDescriptor-pointers. In the
    // single image case, which is the norm on the Xbox, this will be enough,
    // and it's cheaper than comparing the decorated names.
    for (DWORD i = 0; i < obj_locator->pClassDescriptor->numBaseClasses; i++) {
        const RTTIBaseClassDescriptor *target_bcd_candidate = obj_locator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors[i];
        const TypeDescriptor *target_descriptor_candidate = target_bcd_candidate->pTypeDescriptor;

        if (target_descriptor_candidate == target_type) {
            // Target type found, but we have to keep searching for the source type,
            // and make sure there's no non-public inheritance going on, or we fail.
            // In the single-inheritance case, base classes are simply sorted
            // from bottom to top, so the source type comes after the target type.

            for (DWORD j = i + 1; j < obj_locator->pClassDescriptor->numBaseClasses; j++) {
                const RTTIBaseClassDescriptor *source_bcd_candidate = obj_locator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors[j];

                if ((source_bcd_candidate->attributes & BCD_PRIVORPROTBASE) != 0) {
                    // Downcasting "through" protected or private inheritance is not allowed
                    return nullptr;
                }

                const TypeDescriptor *src_descriptor_candidate = source_bcd_candidate->pTypeDescriptor;
                if (src_descriptor_candidate == source_type) {
                    // Source type found, cast is valid
                    return target_bcd_candidate;
                }
            }

            // Source type not found in inheritance hierarchy
            return nullptr;
        }
    }

    // If the fast search didn't succeed, we check again, with a
    // computationally more expensive full decorated name comparison
    for (DWORD i = 0; i < obj_locator->pClassDescriptor->numBaseClasses; i++) {
        const RTTIBaseClassDescriptor *target_bcd_candidate = obj_locator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors[i];
        const TypeDescriptor *target_descriptor_candidate = target_bcd_candidate->pTypeDescriptor;

        if (strcmp(target_descriptor_candidate->name, target_type->name) == 0) {
            // Target type found, but we have to keep searching for the source type,
            // and make sure there's no non-public inheritance going on, or we fail.
            // In the single-inheritance case, base classes are simply sorted
            // from bottom to top, so the source type comes after the target type.

            for (DWORD j = i + 1; j < obj_locator->pClassDescriptor->numBaseClasses; j++) {
                const RTTIBaseClassDescriptor *source_bcd_candidate = obj_locator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors[j];

                if ((source_bcd_candidate->attributes & BCD_PRIVORPROTBASE) != 0) {
                    // Downcasting "through" protected or private inheritance is not allowed
                    return nullptr;
                }

                const TypeDescriptor *src_descriptor_candidate = source_bcd_candidate->pTypeDescriptor;
                if (strcmp(src_descriptor_candidate->name, source_type->name) == 0) {
                    // Source type found, cast is valid
                    return target_bcd_candidate;
                }
            }

            // Source type not found in inheritance hierarchy
            return nullptr;
        }
    }

    // Both searches found no base class descriptor for the target
    return nullptr;
}

const RTTIBaseClassDescriptor *get_target_type_mi (const RTTICompleteObjectLocator *obj_locator, const TypeDescriptor *source_type, const TypeDescriptor *target_type)
{
    DWORD target_num_bases = 0;
    DWORD target_index = -1;

    const RTTIBaseClassDescriptor *found_source_bcd = nullptr;
    const RTTIBaseClassDescriptor *found_target_bcd = nullptr;

    for (DWORD i = 0; i < obj_locator->pClassDescriptor->numBaseClasses; i++) {
        const RTTIBaseClassDescriptor *bcd_candidate = obj_locator->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors[i];
        const TypeDescriptor *td_candidate = bcd_candidate->pTypeDescriptor;

        debugPrint("Class: %s\n", td_candidate->name);
        //if (td_candidate == target_type) {
            debugPrint("contained Bases: %d\n", bcd_candidate->numContainedBases);
        //}

        // Trick to skip RTTI checks for base classes in the list that are
        // bases of the target
        if (i - target_index > target_num_bases) {
            if ((td_candidate == target_type) || (strcmp(td_candidate->name, target_type->name) == 0)) {
                // If the Base Class Descriptor of the source was already
                // found, that means we have either a cross-cast or an
                // up-cast, and we're done right now
                if (found_source_bcd != nullptr) {
                    // Check if the found base classes are unreachable
                    if ((bcd_candidate->attributes & (BCD_NOTVISIBLE | BCD_AMBIGUOUS)) != 0) {
                        return nullptr;
                    }
                    if ((found_source_bcd->attributes & BCD_NOTVISIBLE) != 0) {
                        return nullptr;
                    }

                    return bcd_candidate;
                }

                found_target_bcd = bcd_candidate;
                target_index = i;
                target_num_bases = bcd_candidate->numContainedBases;
            }
        }

        if ((td_candidate == source_type) || (strcmp(td_candidate->name, source_type->name) == 0)) {
            // No target Base Class Descriptor found yet? Remember source BCD
            // and keep searching
            if (found_target_bcd == nullptr) {
                found_source_bcd = bcd_candidate;
                continue;
            }

            if (i - target_index <= target_num_bases) {
                // Index inside the target's base class interval, so
                // it's a down-cast

                // FIXME: comments
                if ((found_target_bcd->attributes & BCD_HASPCHD) == 0) {
                    if ((target_index == 0) && ((bcd_candidate->attributes & BCD_NOTVISIBLE) != 0)) {
                        return nullptr;
                    } else {
                        return found_target_bcd;
                    }
                }

                const RTTIBaseClassDescriptor *source_bcd_in_target = found_target_bcd->pClassDescriptor->pBaseClassArray->arrayOfBaseClassDescriptors[i - target_index];
                if ((source_bcd_in_target->attributes & BCD_NOTVISIBLE) != 0) {
                    return nullptr;
                } else {
                    return found_target_bcd;
                }
            } else {
                // Cross-cast or up-cast
                if ((found_target_bcd->attributes & (BCD_NOTVISIBLE | BCD_AMBIGUOUS)) != 0) {
                    return nullptr;
                }
                if ((bcd_candidate->attributes & BCD_NOTVISIBLE) != 0) {
                    return nullptr;
                }

                return found_target_bcd;
            }
        }
    }

    // We never found fitting source and target Base Class Descriptors, so
    // the cast fails
    return nullptr;
}

const RTTIBaseClassDescriptor *get_target_type_vi (const RTTICompleteObjectLocator *obj_locator, const TypeDescriptor *source_type, const TypeDescriptor *target_type)
{
    assert(0);
}


static void *get_complete_obj_ptr (void *objptr);
static ptrdiff_t pmd_to_offset (const void *this_ptr, const PMD *pmd);

/**
 *
 * @param inptr Pointer to a polymorphic object
 * @param VfDelta Offset of virtual function pointer in object
 * @param SrcType Static type of object pointed to by the inptr parameter
 * @param TargetType Intended result of cast
 * @param isReference true if input is reference, else if input is pointer
*/
extern "C" __cdecl PVOID __RTDynamicCast (PVOID inptr, LONG VfDelta, PVOID SrcType, PVOID TargetType, BOOL isReference) throw (...)
{
    __try {
        const RTTIBaseClassDescriptor *new_type;
        const RTTICompleteObjectLocator *objlocator = getobjlocator(inptr);
        const DWORD inheritance_attributes = objlocator->pClassDescriptor->attributes;
        debugPrint("attributes %lu\n", inheritance_attributes);

        const TypeDescriptor *src_type = reinterpret_cast<TypeDescriptor *>(SrcType);
        const TypeDescriptor *target_type = reinterpret_cast<TypeDescriptor *>(TargetType);

        if ((inheritance_attributes & CHD_MULTINH) == 0) {
            // Single inheritance
            new_type = get_target_type_si(objlocator, src_type, target_type);
        } else if ((inheritance_attributes & CHD_VIRTINH) == 0) {
            // Multiple non-virtual inheritance
            new_type = get_target_type_mi(objlocator, src_type, target_type);
        } else {
            // Virtual inheritance
            new_type = get_target_type_vi(objlocator, src_type, target_type);
            assert(0);
            // FIXME
        }


        // If cast fails
        if (new_type == nullptr) {
            if (isReference) {
                throw std::bad_cast();
            }

            return nullptr;
        }

        // - get complete object pointer
        // - add PMD offset
        auto complete_obj_ptr = get_complete_obj_ptr(inptr);
        return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(complete_obj_ptr) + pmd_to_offset(complete_obj_ptr, &new_type->where));

        // FIXME: Convert and return
    } __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
        throw std::__non_rtti_object::__construct_from_string_literal("Access violation - no RTTI data!");
    }
}

static void *get_complete_obj_ptr (void *objptr)
{
    const RTTICompleteObjectLocator *objlocator = getobjlocator(objptr);
    return static_cast<char *>(objptr) - objlocator->offset;
}

static ptrdiff_t pmd_to_offset (const void *this_ptr, const PMD *pmd)
{
    ptrdiff_t offset = 0;

    if (pmd->pdisp >= 0) {
        offset = pmd->pdisp;
        offset += *(uint32_t *)((char *)*(ptrdiff_t *)((char *)this_ptr + offset) + pmd->vdisp);
    }

    offset += pmd->mdisp;
    return offset;
}
