#include "MemoryUtilities.h"






bool MemoryUtilities::IsValidPtr(void* memoryPtr)
{
    MEMORY_BASIC_INFORMATION mbi;

    if (VirtualQuery(memoryPtr, &mbi, sizeof(mbi)) != sizeof(mbi)) // Try to request (query) information about the given memory region, return False if attempt fails.
        return false;

    if (mbi.State != MEM_COMMIT) // Memory region must be commited (actually backed by physical memory). Return False if it's not.
        return false;

    /* Strip out guard and cache flags to examine the core protection flags. */
    DWORD protectionFlags = mbi.Protect & ~(PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE);
    if ((protectionFlags & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) == false) // See if we're able to request one of the read permissions (readable or executable+read), return False if we aren't.
        return false;

    return true;
}

bool MemoryUtilities::IsValidAddress(const uintptr_t& memoryAddress)
{
    void* memoryPtr = reinterpret_cast<void*>(memoryAddress);
    return IsValidPtr(memoryPtr);
}




void* MemoryUtilities::PtrAddOffset(void* memoryPtr, size_t offset)
{
    /* Since void* doesn't support pointer arithmetics, we need to convert it to uintptr_t first. */
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PtrAddOffset(memoryAddress, offset);
}

void* MemoryUtilities::PtrAddOffset(const uintptr_t& memoryAddress, size_t offset)
{
    /* Calculate the new address by adding the offset. */
    uintptr_t newMemoryAddress = AddressAddOffset(memoryAddress, offset);
    if (newMemoryAddress == 0x0) // Return null pointer if memory region is invalid.
        return nullptr;

    return reinterpret_cast<void*>(newMemoryAddress);
}


uintptr_t MemoryUtilities::AddressAddOffset(void* memoryPtr, size_t offset)
{
    /* Since void* doesn't support pointer arithmetics, we need to convert it to uintptr_t first. */
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return AddressAddOffset(memoryAddress, offset);
}

uintptr_t MemoryUtilities::AddressAddOffset(const uintptr_t& memoryAddress, size_t offset)
{
    /* Calculate the new address by adding the offset. */
    uintptr_t newMemoryAddress = memoryAddress + offset;

    /* Before dereferencing, verify we can read sizeof(uintptr_t) bytes at 'newMemoryAddress'. */
    /* Abort and return 0 if memory isn't readable */
    if (IsValidAddress(newMemoryAddress) == false || IsValidAddress((newMemoryAddress + sizeof(uintptr_t) - 1)) == false)
        return 0x0;

    /* Return the valid address */
    return newMemoryAddress;
}




void* MemoryUtilities::PtrFollowPointerChain(void* memoryPtr, const std::vector<uintptr_t>& memoryOffsets)
{
    /* Since void* doesn't support pointer arithmetics, we need to convert it to uintptr_t first. */
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);

    /* Calculate the new address by following the pointer chain. */
    uintptr_t newMemoryAddress = AddressFollowPointerChain(memoryAddress, memoryOffsets);
    if (newMemoryAddress == 0x0) // Return null pointer if memory region is invalid.
        return nullptr;

    return reinterpret_cast<void*>(newMemoryAddress);
}

uintptr_t MemoryUtilities::AddressFollowPointerChain(const uintptr_t& memoryAddress, const std::vector<uintptr_t>& memoryOffsets)
{
    uintptr_t newMemoryAddress = memoryAddress;
    size_t offsetsCount = memoryOffsets.size();

    for (size_t i = 0; i < offsetsCount; ++i)
    {
        /* Before dereferencing, verify we can read sizeof(uintptr_t) bytes at 'newMemoryAddress'. */
        /* Abort and return 0 if memory isn't readable */
        if (IsValidAddress(newMemoryAddress) == false || IsValidAddress((newMemoryAddress + sizeof(uintptr_t) - 1)) == false)
            return 0x0;

        /* Read the next pointer value from memory and add the current offset to advance to the next address in the chain. */
        newMemoryAddress = *reinterpret_cast<uintptr_t*>(newMemoryAddress);
        newMemoryAddress += memoryOffsets[i];
    }

    /* After processing all offsets, 'ptr' should point to the final data. */
    return newMemoryAddress;
}




bool MemoryUtilities::GetBool(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetBool(memoryAddress);
}

bool MemoryUtilities::GetBool(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read and return the boolean value from the target address. */
    return *reinterpret_cast<bool*>(memoryAddress);
}

bool MemoryUtilities::SetBool(void* memoryPtr, bool newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetBool(memoryAddress, newValue);
}

bool MemoryUtilities::SetBool(const uintptr_t& memoryAddress, bool newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Cast the address to bool* */
    bool* targetBool = reinterpret_cast<bool*>(memoryAddress);
    SIZE_T byteSize = sizeof(bool);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetBool, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new boolean value. */
    *targetBool = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetBool, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchBool(void* memoryPtr, bool from, bool to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchBool(memoryAddress, from, to);
}

bool MemoryUtilities::PatchBool(const uintptr_t& memoryAddress, bool from, bool to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    bool* targetBool = reinterpret_cast<bool*>(memoryAddress);
    if (*targetBool != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(bool);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetBool, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new boolean value */
    *targetBool = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetBool, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectGetBool(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetBool(memoryAddress);
}

bool MemoryUtilities::IndirectGetBool(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return false;

    /* Read and return the boolean value from the target address. */
    return *reinterpret_cast<bool*>(dataAddress);
}

bool MemoryUtilities::IndirectSetBool(void* memoryPtr, bool newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetBool(memoryAddress, newValue);
}

bool MemoryUtilities::IndirectSetBool(const uintptr_t& memoryAddress, bool newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Cast the address to bool* */
    bool* targetBool = reinterpret_cast<bool*>(dataAddress);
    SIZE_T byteSize = sizeof(bool);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetBool, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new boolean value. */
    *targetBool = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetBool, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchBool(void* memoryPtr, bool from, bool to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchBool(memoryAddress, from, to);
}

bool MemoryUtilities::IndirectPatchBool(const uintptr_t& memoryAddress, bool from, bool to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    bool* targetBool = reinterpret_cast<bool*>(dataAddress);
    if (*targetBool != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(bool);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetBool, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new boolean value */
    *targetBool = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetBool, byteSize, oldProtect, &tmp);

    return true;
}




int8_t MemoryUtilities::GetInt8(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetInt8(memoryAddress);
}

int8_t MemoryUtilities::GetInt8(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read and return the 8-bit integer from the target address. */
    return *reinterpret_cast<int8_t*>(memoryAddress);
}

bool MemoryUtilities::SetInt8(void* memoryPtr, int8_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetInt8(memoryAddress, newValue);
}

bool MemoryUtilities::SetInt8(const uintptr_t& memoryAddress, int8_t newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Cast the address to int8_t* */
    int8_t* targetInt = reinterpret_cast<int8_t*>(memoryAddress);
    SIZE_T byteSize = sizeof(int8_t);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value. */
    *targetInt = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchInt8(void* memoryPtr, int8_t from, int8_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchInt8(memoryAddress, from, to);
}

bool MemoryUtilities::PatchInt8(const uintptr_t& memoryAddress, int8_t from, int8_t to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    int8_t* targetInt = reinterpret_cast<int8_t*>(memoryAddress);
    if (*targetInt != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(int8_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value */
    *targetInt = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

int8_t MemoryUtilities::IndirectGetInt8(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetInt8(memoryAddress);
}

int8_t MemoryUtilities::IndirectGetInt8(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return -1;

    /* Read and return the 8-bit integer from the target address. */
    return *reinterpret_cast<int8_t*>(dataAddress);
}

bool MemoryUtilities::IndirectSetInt8(void* memoryPtr, int8_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetInt8(memoryAddress, newValue);
}

bool MemoryUtilities::IndirectSetInt8(const uintptr_t& memoryAddress, int8_t newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Cast the address to int8_t* */
    int8_t* targetInt = reinterpret_cast<int8_t*>(dataAddress);
    SIZE_T byteSize = sizeof(int8_t);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value. */
    *targetInt = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchInt8(void* memoryPtr, int8_t from, int8_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchInt8(memoryAddress, from, to);
}

bool MemoryUtilities::IndirectPatchInt8(const uintptr_t& memoryAddress, int8_t from, int8_t to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    int8_t* targetInt = reinterpret_cast<int8_t*>(dataAddress);
    if (*targetInt != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(int8_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value */
    *targetInt = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}





int16_t MemoryUtilities::GetInt16(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetInt16(memoryAddress);
}

int16_t MemoryUtilities::GetInt16(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read and return the 16-bit integer from the target address. */
    return *reinterpret_cast<int16_t*>(memoryAddress);
}

bool MemoryUtilities::SetInt16(void* memoryPtr, int16_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetInt16(memoryAddress, newValue);
}

bool MemoryUtilities::SetInt16(const uintptr_t& memoryAddress, int16_t newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Cast the address to int16_t* */
    int16_t* targetInt = reinterpret_cast<int16_t*>(memoryAddress);
    SIZE_T byteSize = sizeof(int16_t);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value. */
    *targetInt = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchInt16(void* memoryPtr, int16_t from, int16_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchInt16(memoryAddress, from, to);
}

bool MemoryUtilities::PatchInt16(const uintptr_t& memoryAddress, int16_t from, int16_t to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    int16_t* targetInt = reinterpret_cast<int16_t*>(memoryAddress);
    if (*targetInt != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(int16_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value */
    *targetInt = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

int16_t MemoryUtilities::IndirectGetInt16(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetInt16(memoryAddress);
}

int16_t MemoryUtilities::IndirectGetInt16(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return -1;

    /* Read and return the 16-bit integer from the target address. */
    return *reinterpret_cast<int16_t*>(dataAddress);
}

bool MemoryUtilities::IndirectSetInt16(void* memoryPtr, int16_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetInt16(memoryAddress, newValue);
}

bool MemoryUtilities::IndirectSetInt16(const uintptr_t& memoryAddress, int16_t newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Cast the address to int16_t* */
    int16_t* targetInt = reinterpret_cast<int16_t*>(dataAddress);
    SIZE_T byteSize = sizeof(int16_t);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value. */
    *targetInt = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchInt16(void* memoryPtr, int16_t from, int16_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchInt16(memoryAddress, from, to);
}

bool MemoryUtilities::IndirectPatchInt16(const uintptr_t& memoryAddress, int16_t from, int16_t to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    int16_t* targetInt = reinterpret_cast<int16_t*>(dataAddress);
    if (*targetInt != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(int16_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value */
    *targetInt = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}





int32_t MemoryUtilities::GetInt32(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetInt32(memoryAddress);

}

int32_t MemoryUtilities::GetInt32(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read and return the 32-bit integer from the target address. */
    return *reinterpret_cast<int32_t*>(memoryAddress);
}

bool MemoryUtilities::SetInt32(void* memoryPtr, int32_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetInt32(memoryAddress, newValue);
}

bool MemoryUtilities::SetInt32(const uintptr_t& memoryAddress, int32_t newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Cast the address to int32_t* */
    int32_t* targetInt = reinterpret_cast<int32_t*>(memoryAddress);
    SIZE_T byteSize = sizeof(int32_t);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value. */
    *targetInt = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchInt32(void* memoryPtr, int32_t from, int32_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchInt32(memoryAddress, from, to);
}

bool MemoryUtilities::PatchInt32(const uintptr_t& memoryAddress, int32_t from, int32_t to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    int32_t* targetInt = reinterpret_cast<int32_t*>(memoryAddress);
    if (*targetInt != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(int32_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value */
    *targetInt = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}




int32_t MemoryUtilities::IndirectGetInt32(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetInt32(memoryAddress);
}

int32_t MemoryUtilities::IndirectGetInt32(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return -1;

    /* Read and return the 32-bit integer from the target address. */
    return *reinterpret_cast<int32_t*>(dataAddress);
}

bool MemoryUtilities::IndirectSetInt32(void* memoryPtr, int32_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetInt32(memoryAddress, newValue);
}

bool MemoryUtilities::IndirectSetInt32(const uintptr_t& memoryAddress, int32_t newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Cast the address to int32_t* */
    int32_t* targetInt = reinterpret_cast<int32_t*>(dataAddress);
    SIZE_T byteSize = sizeof(int32_t);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value. */
    *targetInt = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchInt32(void* memoryPtr, int32_t from, int32_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchInt32(memoryAddress, from, to);
}

bool MemoryUtilities::IndirectPatchInt32(const uintptr_t& memoryAddress, int32_t from, int32_t to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    int32_t* targetInt = reinterpret_cast<int32_t*>(dataAddress);
    if (*targetInt != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(int32_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value */
    *targetInt = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}




int64_t MemoryUtilities::GetInt64(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetInt64(memoryAddress);
}

int64_t MemoryUtilities::GetInt64(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read and return the 64-bit integer from the target address. */
    return *reinterpret_cast<int64_t*>(memoryAddress);
}

bool MemoryUtilities::SetInt64(void* memoryPtr, int64_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetInt64(memoryAddress, newValue);
}

bool MemoryUtilities::SetInt64(const uintptr_t& memoryAddress, int64_t newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Cast the address to int64_t* */
    int64_t* targetInt = reinterpret_cast<int64_t*>(memoryAddress);
    SIZE_T byteSize = sizeof(int64_t);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value. */
    *targetInt = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchInt64(void* memoryPtr, int64_t from, int64_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchInt64(memoryAddress, from, to);
}

bool MemoryUtilities::PatchInt64(const uintptr_t& memoryAddress, int64_t from, int64_t to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    int64_t* targetInt = reinterpret_cast<int64_t*>(memoryAddress);
    if (*targetInt != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(int64_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value */
    *targetInt = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

int64_t MemoryUtilities::IndirectGetInt64(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetInt64(memoryAddress);
}

int64_t MemoryUtilities::IndirectGetInt64(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return -1;

    /* Read and return the 64-bit integer from the target address. */
    return *reinterpret_cast<int64_t*>(dataAddress);
}

bool MemoryUtilities::IndirectSetInt64(void* memoryPtr, int64_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetInt64(memoryAddress, newValue);
}

bool MemoryUtilities::IndirectSetInt64(const uintptr_t& memoryAddress, int64_t newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Cast the address to int64_t* */
    int64_t* targetInt = reinterpret_cast<int64_t*>(dataAddress);
    SIZE_T byteSize = sizeof(int64_t);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value. */
    *targetInt = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchInt64(void* memoryPtr, int64_t from, int64_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchInt64(memoryAddress, from, to);
}

bool MemoryUtilities::IndirectPatchInt64(const uintptr_t& memoryAddress, int64_t from, int64_t to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    int64_t* targetInt = reinterpret_cast<int64_t*>(dataAddress);
    if (*targetInt != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(int64_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetInt, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new integer value */
    *targetInt = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetInt, byteSize, oldProtect, &tmp);

    return true;
}




float MemoryUtilities::GetFloat(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetFloat(memoryAddress);
}

float MemoryUtilities::GetFloat(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1.0f;

    /* Read and return the 32-bit float from the target address. */
    return *reinterpret_cast<float*>(memoryAddress);
}

bool MemoryUtilities::SetFloat(void* memoryPtr, float newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetFloat(memoryAddress, newValue);
}

bool MemoryUtilities::SetFloat(const uintptr_t& memoryAddress, float newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Cast the address to float* */
    float* targetFloat = reinterpret_cast<float*>(memoryAddress);
    SIZE_T byteSize = sizeof(float);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetFloat, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new float value. */
    *targetFloat = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetFloat, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchFloat(void* memoryPtr, float from, float to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchFloat(memoryAddress, from, to);
}

bool MemoryUtilities::PatchFloat(const uintptr_t& memoryAddress, float from, float to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    float* targetFloat = reinterpret_cast<float*>(memoryAddress);
    if (*targetFloat != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(float);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetFloat, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new float value */
    *targetFloat = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetFloat, byteSize, oldProtect, &tmp);

    return true;
}

float MemoryUtilities::IndirectGetFloat(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetFloat(memoryAddress);
}

float MemoryUtilities::IndirectGetFloat(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1.0f;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return -1.0f;

    /* Read and return the 32-bit float from the target address. */
    return *reinterpret_cast<float*>(dataAddress);
}

bool MemoryUtilities::IndirectSetFloat(void* memoryPtr, float newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetFloat(memoryAddress, newValue);
}

bool MemoryUtilities::IndirectSetFloat(const uintptr_t& memoryAddress, float newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Cast the address to float* */
    float* targetFloat = reinterpret_cast<float*>(dataAddress);
    SIZE_T byteSize = sizeof(float);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetFloat, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new float value. */
    *targetFloat = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetFloat, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchFloat(void* memoryPtr, float from, float to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchFloat(memoryAddress, from, to);
}

bool MemoryUtilities::IndirectPatchFloat(const uintptr_t& memoryAddress, float from, float to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    float* targetFloat = reinterpret_cast<float*>(dataAddress);
    if (*targetFloat != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(float);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetFloat, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new float value */
    *targetFloat = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetFloat, byteSize, oldProtect, &tmp);

    return true;
}




double MemoryUtilities::GetDouble(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetDouble(memoryAddress);
}

double MemoryUtilities::GetDouble(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1.0;

    /* Read and return the double value from the target address. */
    return *reinterpret_cast<double*>(memoryAddress);
}

bool MemoryUtilities::SetDouble(void* memoryPtr, double newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetDouble(memoryAddress, newValue);
}

bool MemoryUtilities::SetDouble(const uintptr_t& memoryAddress, double newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Cast the address to double* */
    double* targetDouble = reinterpret_cast<double*>(memoryAddress);
    SIZE_T byteSize = sizeof(double);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetDouble, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new double value. */
    *targetDouble = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetDouble, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchDouble(void* memoryPtr, double from, double to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchDouble(memoryAddress, from, to);
}

bool MemoryUtilities::PatchDouble(const uintptr_t& memoryAddress, double from, double to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    double* targetDouble = reinterpret_cast<double*>(memoryAddress);
    if (*targetDouble != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(double);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetDouble, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new double value */
    *targetDouble = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetDouble, byteSize, oldProtect, &tmp);

    return true;
}

double MemoryUtilities::IndirectGetDouble(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetDouble(memoryAddress);
}

double MemoryUtilities::IndirectGetDouble(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1.0;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return -1.0;

    /* Read and return the double value from the target address. */
    return *reinterpret_cast<double*>(dataAddress);
}

bool MemoryUtilities::IndirectSetDouble(void* memoryPtr, double newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetDouble(memoryAddress, newValue);
}

bool MemoryUtilities::IndirectSetDouble(const uintptr_t& memoryAddress, double newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Cast the address to double* */
    double* targetDouble = reinterpret_cast<double*>(dataAddress);
    SIZE_T byteSize = sizeof(double);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetDouble, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new double value. */
    *targetDouble = newValue;

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetDouble, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchDouble(void* memoryPtr, double from, double to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchDouble(memoryAddress, from, to);
}

bool MemoryUtilities::IndirectPatchDouble(const uintptr_t& memoryAddress, double from, double to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    double* targetDouble = reinterpret_cast<double*>(dataAddress);
    if (*targetDouble != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(double);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(targetDouble, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new double value */
    *targetDouble = to;

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(targetDouble, byteSize, oldProtect, &tmp);

    return true;
}




std::string MemoryUtilities::GetString(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetString(memoryAddress);
}

std::string MemoryUtilities::GetString(void* memoryPtr, size_t maxLength)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetString(memoryAddress, maxLength);
}

std::string MemoryUtilities::GetString(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::string();

    /* Read and return the string from the target address. */
    const char* strPtr = reinterpret_cast<const char*>(memoryAddress);
    return std::string(strPtr);
}

std::string MemoryUtilities::GetString(const uintptr_t& memoryAddress, size_t maxLength)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::string();

    /* Read and return the string from the target address. */
    const char* strPtr = reinterpret_cast<const char*>(memoryAddress);
    return std::string(strPtr, strnlen_s(strPtr, maxLength));
}

bool MemoryUtilities::SetString(void* memoryPtr, const std::string& newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetString(memoryAddress, newValue);
}

bool MemoryUtilities::SetString(const uintptr_t& memoryAddress, const std::string& newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Cast the address to char* */
    char* targetStr = reinterpret_cast<char*>(memoryAddress);
    SIZE_T byteSize = newValue.size() + 1; // +1 for null terminator

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetStr, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new string value. */
    memcpy(targetStr, newValue.c_str(), byteSize);

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetStr, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchString(void* memoryPtr, const std::string& from, const std::string& to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchString(memoryAddress, from, to);
}

bool MemoryUtilities::PatchString(const uintptr_t& memoryAddress, const std::string& from, const std::string& to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    const char* currentStr = reinterpret_cast<const char*>(memoryAddress);
    if (strncmp(currentStr, from.c_str(), from.size()) != 0)
        return false;

    SIZE_T byteSize = to.size() + 1;

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(reinterpret_cast<void*>(memoryAddress), byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new string value */
    memcpy(reinterpret_cast<void*>(memoryAddress), to.c_str(), byteSize);

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(reinterpret_cast<void*>(memoryAddress), byteSize, oldProtect, &tmp);

    return true;
}




std::string MemoryUtilities::IndirectGetString(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetString(memoryAddress);
}

std::string MemoryUtilities::IndirectGetString(void* memoryPtr, size_t maxLength)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetString(memoryAddress, maxLength);
}

std::string MemoryUtilities::IndirectGetString(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::string();

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return std::string();

    /* Read and return the string from the target address. */
    const char* strPtr = reinterpret_cast<const char*>(dataAddress);
    return std::string(strPtr);
}

std::string MemoryUtilities::IndirectGetString(const uintptr_t& memoryAddress, size_t maxLength)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::string();

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return std::string();

    /* Read and return the string from the target address. */
    const char* strPtr = reinterpret_cast<const char*>(dataAddress);
    return std::string(strPtr, strnlen_s(strPtr, maxLength));
}

bool MemoryUtilities::IndirectSetString(void* memoryPtr, const std::string& newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetString(memoryAddress, newValue);
}

bool MemoryUtilities::IndirectSetString(const uintptr_t& memoryAddress, const std::string& newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Cast the address to char* */
    char* targetStr = reinterpret_cast<char*>(dataAddress);
    SIZE_T byteSize = newValue.size() + 1;

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetStr, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new string value. */
    memcpy(targetStr, newValue.c_str(), byteSize);

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetStr, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchString(void* memoryPtr, const std::string& from, const std::string& to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchString(memoryAddress, from, to);
}

bool MemoryUtilities::IndirectPatchString(const uintptr_t& memoryAddress, const std::string& from, const std::string& to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    const char* currentStr = reinterpret_cast<const char*>(dataAddress);
    if (strncmp(currentStr, from.c_str(), from.size()) != 0)
        return false;

    SIZE_T byteSize = to.size() + 1;

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(reinterpret_cast<void*>(dataAddress), byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new string value */
    memcpy(reinterpret_cast<void*>(dataAddress), to.c_str(), byteSize);

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(reinterpret_cast<void*>(dataAddress), byteSize, oldProtect, &tmp);

    return true;
}




std::wstring MemoryUtilities::GetWString(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetWString(memoryAddress);
}

std::wstring MemoryUtilities::GetWString(void* memoryPtr, size_t maxLength)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetWString(memoryAddress, maxLength);
}

std::wstring MemoryUtilities::GetWString(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::wstring();

    /* Read and return the wide string from the target address. */
    const wchar_t* wStrPtr = reinterpret_cast<const wchar_t*>(memoryAddress);
    return std::wstring(wStrPtr);
}

std::wstring MemoryUtilities::GetWString(const uintptr_t& memoryAddress, size_t maxLength)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::wstring();

    /* Read and return the wide string from the target address. */
    const wchar_t* wStrPtr = reinterpret_cast<const wchar_t*>(memoryAddress);
    return std::wstring(wStrPtr, wcsnlen_s(wStrPtr, maxLength));
}

bool MemoryUtilities::SetWString(void* memoryPtr, const std::wstring& newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetWString(memoryAddress, newValue);
}

bool MemoryUtilities::SetWString(const uintptr_t& memoryAddress, const std::wstring& newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Cast the address to wchar_t* */
    wchar_t* targetStr = reinterpret_cast<wchar_t*>(memoryAddress);
    SIZE_T byteSize = (newValue.size() + 1) * sizeof(wchar_t); // +1 for null terminator

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetStr, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new string value. */
    memcpy(targetStr, newValue.c_str(), byteSize);

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetStr, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchWString(void* memoryPtr, const std::wstring& from, const std::wstring& to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchWString(memoryAddress, from, to);
}

bool MemoryUtilities::PatchWString(const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    const wchar_t* currentStr = reinterpret_cast<const wchar_t*>(memoryAddress);
    if (wcsncmp(currentStr, from.c_str(), from.size()) != 0)
        return false;

    SIZE_T byteSize = (to.size() + 1) * sizeof(wchar_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(reinterpret_cast<void*>(memoryAddress), byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new string value */
    memcpy(reinterpret_cast<void*>(memoryAddress), to.c_str(), byteSize);

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(reinterpret_cast<void*>(memoryAddress), byteSize, oldProtect, &tmp);

    return true;
}




std::wstring MemoryUtilities::IndirectGetWString(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetWString(memoryAddress);
}

std::wstring MemoryUtilities::IndirectGetWString(void* memoryPtr, size_t maxLength)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetWString(memoryAddress, maxLength);
}

std::wstring MemoryUtilities::IndirectGetWString(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::wstring();

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return std::wstring();

    /* Read and return the wide string from the target address. */
    const wchar_t* wStrPtr = reinterpret_cast<const wchar_t*>(dataAddress);
    return std::wstring(wStrPtr);
}

std::wstring MemoryUtilities::IndirectGetWString(const uintptr_t& memoryAddress, size_t maxLength)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::wstring();

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (!IsValidAddress(dataAddress))
        return std::wstring();

    /* Read and return the wide string from the target address. */
    const wchar_t* wStrPtr = reinterpret_cast<const wchar_t*>(dataAddress);
    return std::wstring(wStrPtr, wcsnlen_s(wStrPtr, maxLength));
}

bool MemoryUtilities::IndirectSetWString(void* memoryPtr, const std::wstring& newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetWString(memoryAddress, newValue);
}

bool MemoryUtilities::IndirectSetWString(const uintptr_t& memoryAddress, const std::wstring& newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Cast the address to wchar_t* */
    wchar_t* targetStr = reinterpret_cast<wchar_t*>(dataAddress);
    SIZE_T byteSize = (newValue.size() + 1) * sizeof(wchar_t);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(targetStr, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new string value. */
    memcpy(targetStr, newValue.c_str(), byteSize);

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(targetStr, byteSize, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchWString(void* memoryPtr, const std::wstring& from, const std::wstring& to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchWString(memoryAddress, from, to);
}

bool MemoryUtilities::IndirectPatchWString(const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    const wchar_t* currentStr = reinterpret_cast<const wchar_t*>(dataAddress);
    if (wcsncmp(currentStr, from.c_str(), from.size()) != 0)
        return false;

    SIZE_T byteSize = (to.size() + 1) * sizeof(wchar_t);

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(reinterpret_cast<void*>(dataAddress), byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new string value */
    memcpy(reinterpret_cast<void*>(dataAddress), to.c_str(), byteSize);

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(reinterpret_cast<void*>(dataAddress), byteSize, oldProtect, &tmp);

    return true;
}




std::vector<uint8_t> MemoryUtilities::GetBytes(void* memoryPtr, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetBytes(memoryAddress, byteCount);
}

std::vector<uint8_t> MemoryUtilities::GetBytes(const uintptr_t& memoryAddress, size_t byteCount)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return {};

    std::vector<uint8_t> buffer(byteCount);
    /* Read and return the bytes from the target address. */
    memcpy(buffer.data(), reinterpret_cast<void*>(memoryAddress), byteCount);
    return buffer;
}

bool MemoryUtilities::SetBytes(void* memoryPtr, const uint8_t* newBytes, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetBytes(memoryAddress, newBytes, byteCount);
}

bool MemoryUtilities::SetBytes(const uintptr_t& memoryAddress, const uint8_t* newBytes, size_t byteCount)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    uint8_t* target = reinterpret_cast<uint8_t*>(memoryAddress);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(target, byteCount, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new bytes. */
    memcpy(target, newBytes, byteCount);

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(target, byteCount, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::PatchBytes(void* memoryPtr, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchBytes(memoryAddress, fromBytes, toBytes, byteCount);
}

bool MemoryUtilities::PatchBytes(const uintptr_t& memoryAddress, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    uint8_t* target = reinterpret_cast<uint8_t*>(memoryAddress);

    /* Verify that the current bytes match the expected ones. */
    if (memcmp(target, fromBytes, byteCount) != 0) // Only patch if the current bytes match 'fromBytes'.
        return false;

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(target, byteCount, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new bytes */
    memcpy(target, toBytes, byteCount);

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(target, byteCount, oldProtect, &tmp);

    return true;
}

std::vector<uint8_t> MemoryUtilities::IndirectGetBytes(void* memoryPtr, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetBytes(memoryAddress, byteCount);
}

std::vector<uint8_t> MemoryUtilities::IndirectGetBytes(const uintptr_t& memoryAddress, size_t byteCount)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return {};

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return {};

    std::vector<uint8_t> buffer(byteCount);
    /* Read and return the bytes from the target address. */
    memcpy(buffer.data(), reinterpret_cast<void*>(dataAddress), byteCount);
    return buffer;
}

bool MemoryUtilities::IndirectSetBytes(void* memoryPtr, const uint8_t* newBytes, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetBytes(memoryAddress, newBytes, byteCount);
}

bool MemoryUtilities::IndirectSetBytes(const uintptr_t& memoryAddress, const uint8_t* newBytes, size_t byteCount)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    uint8_t* target = reinterpret_cast<uint8_t*>(dataAddress);
    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtect(target, byteCount, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new bytes. */
    memcpy(target, newBytes, byteCount);

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtect(target, byteCount, oldProtect, &tmp);

    return true;
}

bool MemoryUtilities::IndirectPatchBytes(void* memoryPtr, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchBytes(memoryAddress, fromBytes, toBytes, byteCount);
}

bool MemoryUtilities::IndirectPatchBytes(const uintptr_t& memoryAddress, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = *reinterpret_cast<uintptr_t*>(memoryAddress);
    if (IsValidAddress(dataAddress) == false)
        return false;

    uint8_t* target = reinterpret_cast<uint8_t*>(dataAddress);

    /* Verify that the current bytes match the expected ones. */
    if (memcmp(target, fromBytes, byteCount) != 0) // Only patch if the current bytes match 'fromBytes'.
        return false;

    /* Make the memory region writable */
    DWORD oldProtect;
    if (VirtualProtect(target, byteCount, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new bytes */
    memcpy(target, toBytes, byteCount);

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtect(target, byteCount, oldProtect, &tmp);

    return true;
}