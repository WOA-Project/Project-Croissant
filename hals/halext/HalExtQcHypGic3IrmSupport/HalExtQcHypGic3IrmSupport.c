/*++

Copyright (c) 2024 The DuoWoA authors

Module Name:

    HalExtQcHypGic3IrmSupport.c

Abstract:

    This file implements a HAL Extension Module for 
    Qualcomm Hypervisor Missing GicV3 IRM Bit Support.

Author:

    Gustave Monce (gmonce) 8-23-2024

--*/

//
// ------------------------------------------------------------------- Includes
//

#include <arm64intr.h>
#include <nthalext.h>

//
// ---------------------------------------------------------------- Definitions
//

#define VBAR_EL1_SYSREG ARM64_SYSREG(3, 0, 12, 0, 0)

//
// ------------------------------------------------- Data Structure Definitions
//

//
// ----------------------------------------------- Internal Function Prototypes
//

//
// ------------------------------------------------------------------ Functions
//

NTSTATUS
AddResourceGroup (
    __in ULONG Handle,
    __in PCSRT_RESOURCE_GROUP_HEADER ResourceGroup
    )

/*++

Routine Description:

    This routine patches the main kernel HAL code to work without the IRM Bit.

Arguments:

    Handle - Supplies the HAL Extension handle which must be passed to other
        HAL Extension APIs.

    ResourceGroup - Supplies a pointer to the Resource Group which the
        HAL Extension has been installed on.

Return Value:

    NTSTATUS code.

--*/

{
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(ResourceGroup);

    // VBAR_EL1 always points to KiArm64ExceptionVectors on Windows
    UINT64 VBAR_EL1 = _ReadStatusReg(VBAR_EL1_SYSREG);

    // Try to find the base of the kernel now
    UINT64 KernelBase = VBAR_EL1;
    while (TRUE)
    {
        if (*((PUINT16)KernelBase) == 0x5A4D) // MZ
        {
            // We found the base of the kernel image.
            break;
        }

        KernelBase--;
    }

    // Now that we have the base of the kernel, scan the entire file for
    // the problematic instruction

    // TODO: Get Kernel Size?
    UINT64 GICINSTR = 0;
    UINT64 current = KernelBase;
    while (TRUE)
    {
        if (*(UINT32 *)current == 0xD518CBA8) // msr icc_sgi1r_el1, x8
        {
            GICINSTR = current;
            break;
        }

        if (*(UINT32 *)current == 0xD518CBAA) // msr icc_sgi1r_el1, x10
        {
            GICINSTR = current;
            break;
        }

        current += sizeof(UINT32);
    }

    // We now have the address of the msr icc_sgi1r_el1, xXX instruction
    // We now inject our code

    // TODO

    // We always return success as we are not a real hal extension.
    return STATUS_SUCCESS;
}
