#include "MemoryUtilities.h"
#include <Psapi.h>






int16_t MemoryUtilities::Convertion::HEXChar_ToInt16(const char& c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;

    return -1;
}

char MemoryUtilities::Convertion::Int16_ToHEXChar(const int16_t& value)
{
    if (value >= 0 && value <= 9) // 0 -> '0', 1 -> '1', …, 9 -> '9'
    {
        return static_cast<char>('0' + value);
    }
    if (value >= 10 && value <= 15) // 10 -> 'A', 11 -> 'B', …, 15 -> 'F'
    {
        return static_cast<char>('A' + (value - 10));
    }

    /* Out-of-range value */
    return '?';
}




std::vector<std::optional<uint8_t>> MemoryUtilities::Convertion::MemoryPattern_ToBytesPattern(const std::string& memoryPattern)
{
    std::vector<std::optional<uint8_t>> outBytes;

    size_t i = 0;
    const size_t patternSize = memoryPattern.size();
    while (i < patternSize)
    {
        /* Skip whitespace between tokens. */
        if (std::isspace(static_cast<unsigned char>(memoryPattern[i])))
        {
            i++;
            continue;
        }

        /* If we see "??", treat it as a wildcard. */
        if (i + 1 < patternSize && memoryPattern[i] == '?' && memoryPattern[i + 1] == '?')
        {
            outBytes.push_back(std::nullopt);
            i += 2;
        }
        else if (i + 1 < patternSize) // Otherwise, expect two HEX digits.
        {
            int16_t highPart = Convertion::HEXChar_ToInt16(memoryPattern[i]);
            int16_t lowPart = Convertion::HEXChar_ToInt16(memoryPattern[i + 1]);

            // On invalid HEX digit, abort parsing and return an empty pattern.
            if (highPart < 0 || lowPart < 0)
            {
                return {};
            }

            /* Combine the high and low parts into a single byte. */
            uint8_t combinedByte = static_cast<uint8_t>((highPart << 4) | lowPart);
            outBytes.push_back(combinedByte);
            i += 2;
        }
        else // A single trailing character is not enough for a byte or wildcard.
        {
            return {};
        }
    }

    return outBytes;
}






// ========================================================
// |                      #INTERNAL                       |
// ========================================================
bool MemoryUtilities::Internal::IsValidPtr(void* memoryPtr)
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

bool MemoryUtilities::Internal::IsValidAddress(const uintptr_t& memoryAddress)
{
    void* memoryPtr = reinterpret_cast<void*>(memoryAddress);
    return IsValidPtr(memoryPtr);
}




void* MemoryUtilities::Internal::PtrAddOffset(const void* memoryPtr, size_t offset)
{
    /* Since void* doesn't support pointer arithmetics, we need to convert it to uintptr_t first. */
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PtrAddOffset(memoryAddress, offset);
}

void* MemoryUtilities::Internal::PtrAddOffset(const uintptr_t& memoryAddress, size_t offset)
{
    /* Calculate the new address by adding the offset. */
    uintptr_t newMemoryAddress = AddressAddOffset(memoryAddress, offset);
    if (newMemoryAddress == 0x0) // Return null pointer if memory region is invalid.
        return nullptr;

    return reinterpret_cast<void*>(newMemoryAddress);
}


uintptr_t MemoryUtilities::Internal::AddressAddOffset(const void* memoryPtr, size_t offset)
{
    /* Since void* doesn't support pointer arithmetics, we need to convert it to uintptr_t first. */
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return AddressAddOffset(memoryAddress, offset);
}

uintptr_t MemoryUtilities::Internal::AddressAddOffset(const uintptr_t& memoryAddress, size_t offset)
{
    /* Calculate the new address by adding the offset. */
    uintptr_t newMemoryAddress = memoryAddress + offset;

    /* Before dereferencing, verify we can read sizeof(uintptr_t) bytes at 'newMemoryAddress'. */
    /* Abort and return 0 if memory isn't readable */
    if (IsValidAddress(newMemoryAddress) == false || 
        IsValidAddress((newMemoryAddress + sizeof(uintptr_t) - 1)) == false)
        return 0x0;

    /* Return the valid address */
    return newMemoryAddress;
}




void* MemoryUtilities::Internal::PtrFollowPointerChain(const void* memoryPtr, const std::vector<uintptr_t>& memoryOffsets)
{
    /* Since void* doesn't support pointer arithmetics, we need to convert it to uintptr_t first. */
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);

    /* Calculate the new address by following the pointer chain. */
    uintptr_t newMemoryAddress = AddressFollowPointerChain(memoryAddress, memoryOffsets);
    if (newMemoryAddress == 0x0) // Return null pointer if memory region is invalid.
        return nullptr;

    return reinterpret_cast<void*>(newMemoryAddress);
}

uintptr_t MemoryUtilities::Internal::AddressFollowPointerChain(const uintptr_t& memoryAddress, const std::vector<uintptr_t>& memoryOffsets)
{
    uintptr_t newMemoryAddress = memoryAddress;
    size_t offsetsCount = memoryOffsets.size();

    for (size_t i = 0; i < offsetsCount; ++i)
    {
        /* Before dereferencing, verify we can read sizeof(uintptr_t) bytes at 'newMemoryAddress'. */
        /* Abort and return 0 if memory isn't readable */
        if (IsValidAddress(newMemoryAddress) == false || 
            IsValidAddress((newMemoryAddress + sizeof(uintptr_t) - 1)) == false)
            return 0x0;

        /* Read the next pointer value from memory and add the current offset to advance to the next address in the chain. */
        newMemoryAddress = *reinterpret_cast<uintptr_t*>(newMemoryAddress);
        newMemoryAddress += memoryOffsets[i];
    }

    /* After processing all offsets, 'ptr' should point to the final data. */
    return newMemoryAddress;
}




uintptr_t MemoryUtilities::Internal::ScanForBytesPattern(const uint8_t* startingAddress, size_t size, const std::vector<std::optional<uint8_t>>& bytesPattern)
{
    const size_t patternLength = bytesPattern.size();
    if (patternLength == 0 || size < patternLength) // If there's nothing to search for, or the region is too small, give up.
    {
        return 0x0;
    }

    /* Slide a window of patternLength across the region. */
    for (size_t offset = 0; offset <= size - patternLength; ++offset) 
    {
        bool match = true;
        for (size_t j = 0; j < patternLength; ++j) 
        {
            /* If this pattern position has a concrete byte, it must match exactly. */
            /* If it's std::nullopt, it's a wildcard - accept any byte. */
            if (bytesPattern[j].has_value() && startingAddress[offset + j] != bytesPattern[j].value())
            {
                match = false;
                break;
            }
        }
        if (match) 
        {
            return reinterpret_cast<uintptr_t>(startingAddress + offset);
        }
    }

    return 0x0;
}

uintptr_t MemoryUtilities::Internal::ScanForMemoryPattern(const uint8_t* startingAddress, size_t size, const std::string& memoryPattern)
{
    /* Parse the mask string into a byte-pattern vector. */
    auto bytesPattern = Convertion::MemoryPattern_ToBytesPattern(memoryPattern);
    if (bytesPattern.empty())
    {
        return 0x0;
    }

    return ScanForBytesPattern(startingAddress, size, bytesPattern);
}




uintptr_t MemoryUtilities::Internal::SearchForBytesPattern(const std::vector<std::optional<uint8_t>>& bytesPattern)
{
    /* Obtain the module handle for the current executable or DLL. */
    HMODULE hModule = GetModuleHandleW(nullptr);
    if (!hModule)
    {
        return 0x0;
    }

    /* Retrieve information about the module: base address and image size. */
    MODULEINFO modInfo;
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &modInfo, sizeof(modInfo)))
    {
        return 0x0;
    }

    const uint8_t* baseAddress = reinterpret_cast<const uint8_t*>(modInfo.lpBaseOfDll);
    const size_t imageSize = static_cast<size_t>(modInfo.SizeOfImage);

    /* Scan the entire module image for the pattern. */
    return ScanForBytesPattern(baseAddress, imageSize, bytesPattern);
}

uintptr_t MemoryUtilities::Internal::SearchForMemoryPattern(const std::string& memoryPattern)
{
    /* Parse the mask string into a byte-pattern vector. */
    auto bytesPattern = Convertion::MemoryPattern_ToBytesPattern(memoryPattern);
    if (bytesPattern.empty())
    {
        return 0x0;
    }

    return SearchForBytesPattern(bytesPattern);
}




bool MemoryUtilities::Internal::GetBool(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetBool(memoryAddress);
}

bool MemoryUtilities::Internal::GetBool(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return false;

    /* Read and return the boolean value from the target address. */
    return *reinterpret_cast<bool*>(memoryAddress);
}

bool MemoryUtilities::Internal::SetBool(const void* memoryPtr, bool newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetBool(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::SetBool(const uintptr_t& memoryAddress, bool newValue)
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

bool MemoryUtilities::Internal::PatchBool(const void* memoryPtr, bool from, bool to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchBool(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::PatchBool(const uintptr_t& memoryAddress, bool from, bool to)
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

bool MemoryUtilities::Internal::IndirectGetBool(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetBool(memoryAddress);
}

bool MemoryUtilities::Internal::IndirectGetBool(const uintptr_t& memoryAddress)
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

bool MemoryUtilities::Internal::IndirectSetBool(const void* memoryPtr, bool newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetBool(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::IndirectSetBool(const uintptr_t& memoryAddress, bool newValue)
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

bool MemoryUtilities::Internal::IndirectPatchBool(const void* memoryPtr, bool from, bool to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchBool(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::IndirectPatchBool(const uintptr_t& memoryAddress, bool from, bool to)
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




int8_t MemoryUtilities::Internal::GetInt8(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetInt8(memoryAddress);
}

int8_t MemoryUtilities::Internal::GetInt8(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read and return the 8-bit integer from the target address. */
    return *reinterpret_cast<int8_t*>(memoryAddress);
}

bool MemoryUtilities::Internal::SetInt8(const void* memoryPtr, int8_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetInt8(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::SetInt8(const uintptr_t& memoryAddress, int8_t newValue)
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

bool MemoryUtilities::Internal::PatchInt8(const void* memoryPtr, int8_t from, int8_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchInt8(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::PatchInt8(const uintptr_t& memoryAddress, int8_t from, int8_t to)
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

int8_t MemoryUtilities::Internal::IndirectGetInt8(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetInt8(memoryAddress);
}

int8_t MemoryUtilities::Internal::IndirectGetInt8(const uintptr_t& memoryAddress)
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

bool MemoryUtilities::Internal::IndirectSetInt8(const void* memoryPtr, int8_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetInt8(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::IndirectSetInt8(const uintptr_t& memoryAddress, int8_t newValue)
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

bool MemoryUtilities::Internal::IndirectPatchInt8(const void* memoryPtr, int8_t from, int8_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchInt8(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::IndirectPatchInt8(const uintptr_t& memoryAddress, int8_t from, int8_t to)
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





int16_t MemoryUtilities::Internal::GetInt16(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetInt16(memoryAddress);
}

int16_t MemoryUtilities::Internal::GetInt16(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read and return the 16-bit integer from the target address. */
    return *reinterpret_cast<int16_t*>(memoryAddress);
}

bool MemoryUtilities::Internal::SetInt16(const void* memoryPtr, int16_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetInt16(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::SetInt16(const uintptr_t& memoryAddress, int16_t newValue)
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

bool MemoryUtilities::Internal::PatchInt16(const void* memoryPtr, int16_t from, int16_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchInt16(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::PatchInt16(const uintptr_t& memoryAddress, int16_t from, int16_t to)
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

int16_t MemoryUtilities::Internal::IndirectGetInt16(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetInt16(memoryAddress);
}

int16_t MemoryUtilities::Internal::IndirectGetInt16(const uintptr_t& memoryAddress)
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

bool MemoryUtilities::Internal::IndirectSetInt16(const void* memoryPtr, int16_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetInt16(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::IndirectSetInt16(const uintptr_t& memoryAddress, int16_t newValue)
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

bool MemoryUtilities::Internal::IndirectPatchInt16(const void* memoryPtr, int16_t from, int16_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchInt16(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::IndirectPatchInt16(const uintptr_t& memoryAddress, int16_t from, int16_t to)
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





int32_t MemoryUtilities::Internal::GetInt32(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetInt32(memoryAddress);

}

int32_t MemoryUtilities::Internal::GetInt32(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read and return the 32-bit integer from the target address. */
    return *reinterpret_cast<int32_t*>(memoryAddress);
}

bool MemoryUtilities::Internal::SetInt32(const void* memoryPtr, int32_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetInt32(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::SetInt32(const uintptr_t& memoryAddress, int32_t newValue)
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

bool MemoryUtilities::Internal::PatchInt32(const void* memoryPtr, int32_t from, int32_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchInt32(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::PatchInt32(const uintptr_t& memoryAddress, int32_t from, int32_t to)
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




int32_t MemoryUtilities::Internal::IndirectGetInt32(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetInt32(memoryAddress);
}

int32_t MemoryUtilities::Internal::IndirectGetInt32(const uintptr_t& memoryAddress)
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

bool MemoryUtilities::Internal::IndirectSetInt32(const void* memoryPtr, int32_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetInt32(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::IndirectSetInt32(const uintptr_t& memoryAddress, int32_t newValue)
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

bool MemoryUtilities::Internal::IndirectPatchInt32(const void* memoryPtr, int32_t from, int32_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchInt32(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::IndirectPatchInt32(const uintptr_t& memoryAddress, int32_t from, int32_t to)
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




int64_t MemoryUtilities::Internal::GetInt64(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetInt64(memoryAddress);
}

int64_t MemoryUtilities::Internal::GetInt64(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1;

    /* Read and return the 64-bit integer from the target address. */
    return *reinterpret_cast<int64_t*>(memoryAddress);
}

bool MemoryUtilities::Internal::SetInt64(const void* memoryPtr, int64_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetInt64(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::SetInt64(const uintptr_t& memoryAddress, int64_t newValue)
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

bool MemoryUtilities::Internal::PatchInt64(const void* memoryPtr, int64_t from, int64_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchInt64(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::PatchInt64(const uintptr_t& memoryAddress, int64_t from, int64_t to)
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

int64_t MemoryUtilities::Internal::IndirectGetInt64(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetInt64(memoryAddress);
}

int64_t MemoryUtilities::Internal::IndirectGetInt64(const uintptr_t& memoryAddress)
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

bool MemoryUtilities::Internal::IndirectSetInt64(const void* memoryPtr, int64_t newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetInt64(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::IndirectSetInt64(const uintptr_t& memoryAddress, int64_t newValue)
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

bool MemoryUtilities::Internal::IndirectPatchInt64(const void* memoryPtr, int64_t from, int64_t to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchInt64(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::IndirectPatchInt64(const uintptr_t& memoryAddress, int64_t from, int64_t to)
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




float MemoryUtilities::Internal::GetFloat(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetFloat(memoryAddress);
}

float MemoryUtilities::Internal::GetFloat(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1.0f;

    /* Read and return the 32-bit float from the target address. */
    return *reinterpret_cast<float*>(memoryAddress);
}

bool MemoryUtilities::Internal::SetFloat(const void* memoryPtr, float newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetFloat(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::SetFloat(const uintptr_t& memoryAddress, float newValue)
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

bool MemoryUtilities::Internal::PatchFloat(const void* memoryPtr, float from, float to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchFloat(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::PatchFloat(const uintptr_t& memoryAddress, float from, float to)
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

float MemoryUtilities::Internal::IndirectGetFloat(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetFloat(memoryAddress);
}

float MemoryUtilities::Internal::IndirectGetFloat(const uintptr_t& memoryAddress)
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

bool MemoryUtilities::Internal::IndirectSetFloat(const void* memoryPtr, float newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetFloat(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::IndirectSetFloat(const uintptr_t& memoryAddress, float newValue)
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

bool MemoryUtilities::Internal::IndirectPatchFloat(const void* memoryPtr, float from, float to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchFloat(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::IndirectPatchFloat(const uintptr_t& memoryAddress, float from, float to)
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




double MemoryUtilities::Internal::GetDouble(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetDouble(memoryAddress);
}

double MemoryUtilities::Internal::GetDouble(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return -1.0;

    /* Read and return the double value from the target address. */
    return *reinterpret_cast<double*>(memoryAddress);
}

bool MemoryUtilities::Internal::SetDouble(const void* memoryPtr, double newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetDouble(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::SetDouble(const uintptr_t& memoryAddress, double newValue)
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

bool MemoryUtilities::Internal::PatchDouble(const void* memoryPtr, double from, double to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchDouble(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::PatchDouble(const uintptr_t& memoryAddress, double from, double to)
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

double MemoryUtilities::Internal::IndirectGetDouble(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetDouble(memoryAddress);
}

double MemoryUtilities::Internal::IndirectGetDouble(const uintptr_t& memoryAddress)
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

bool MemoryUtilities::Internal::IndirectSetDouble(const void* memoryPtr, double newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetDouble(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::IndirectSetDouble(const uintptr_t& memoryAddress, double newValue)
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

bool MemoryUtilities::Internal::IndirectPatchDouble(const void* memoryPtr, double from, double to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchDouble(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::IndirectPatchDouble(const uintptr_t& memoryAddress, double from, double to)
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




std::string MemoryUtilities::Internal::GetString(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetString(memoryAddress);
}

std::string MemoryUtilities::Internal::GetString(const void* memoryPtr, size_t maxLength)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetString(memoryAddress, maxLength);
}

std::string MemoryUtilities::Internal::GetString(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::string();

    /* Read and return the string from the target address. */
    const char* strPtr = reinterpret_cast<const char*>(memoryAddress);
    return std::string(strPtr);
}

std::string MemoryUtilities::Internal::GetString(const uintptr_t& memoryAddress, size_t maxLength)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::string();

    /* Read and return the string from the target address. */
    const char* strPtr = reinterpret_cast<const char*>(memoryAddress);
    return std::string(strPtr, strnlen_s(strPtr, maxLength));
}

bool MemoryUtilities::Internal::SetString(const void* memoryPtr, const std::string& newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetString(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::SetString(const uintptr_t& memoryAddress, const std::string& newValue)
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

bool MemoryUtilities::Internal::PatchString(const void* memoryPtr, const std::string& from, const std::string& to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchString(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::PatchString(const uintptr_t& memoryAddress, const std::string& from, const std::string& to)
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




std::string MemoryUtilities::Internal::IndirectGetString(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetString(memoryAddress);
}

std::string MemoryUtilities::Internal::IndirectGetString(const void* memoryPtr, size_t maxLength)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetString(memoryAddress, maxLength);
}

std::string MemoryUtilities::Internal::IndirectGetString(const uintptr_t& memoryAddress)
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

std::string MemoryUtilities::Internal::IndirectGetString(const uintptr_t& memoryAddress, size_t maxLength)
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

bool MemoryUtilities::Internal::IndirectSetString(const void* memoryPtr, const std::string& newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetString(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::IndirectSetString(const uintptr_t& memoryAddress, const std::string& newValue)
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

bool MemoryUtilities::Internal::IndirectPatchString(const void* memoryPtr, const std::string& from, const std::string& to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchString(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::IndirectPatchString(const uintptr_t& memoryAddress, const std::string& from, const std::string& to)
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




std::wstring MemoryUtilities::Internal::GetWString(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetWString(memoryAddress);
}

std::wstring MemoryUtilities::Internal::GetWString(const void* memoryPtr, size_t maxLength)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetWString(memoryAddress, maxLength);
}

std::wstring MemoryUtilities::Internal::GetWString(const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::wstring();

    /* Read and return the wide string from the target address. */
    const wchar_t* wStrPtr = reinterpret_cast<const wchar_t*>(memoryAddress);
    return std::wstring(wStrPtr);
}

std::wstring MemoryUtilities::Internal::GetWString(const uintptr_t& memoryAddress, size_t maxLength)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return std::wstring();

    /* Read and return the wide string from the target address. */
    const wchar_t* wStrPtr = reinterpret_cast<const wchar_t*>(memoryAddress);
    return std::wstring(wStrPtr, wcsnlen_s(wStrPtr, maxLength));
}

bool MemoryUtilities::Internal::SetWString(const void* memoryPtr, const std::wstring& newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetWString(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::SetWString(const uintptr_t& memoryAddress, const std::wstring& newValue)
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

bool MemoryUtilities::Internal::PatchWString(const void* memoryPtr, const std::wstring& from, const std::wstring& to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchWString(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::PatchWString(const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to)
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




std::wstring MemoryUtilities::Internal::IndirectGetWString(void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetWString(memoryAddress);
}

std::wstring MemoryUtilities::Internal::IndirectGetWString(const void* memoryPtr, size_t maxLength)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetWString(memoryAddress, maxLength);
}

std::wstring MemoryUtilities::Internal::IndirectGetWString(const uintptr_t& memoryAddress)
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

std::wstring MemoryUtilities::Internal::IndirectGetWString(const uintptr_t& memoryAddress, size_t maxLength)
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

bool MemoryUtilities::Internal::IndirectSetWString(const void* memoryPtr, const std::wstring& newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetWString(memoryAddress, newValue);
}

bool MemoryUtilities::Internal::IndirectSetWString(const uintptr_t& memoryAddress, const std::wstring& newValue)
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

bool MemoryUtilities::Internal::IndirectPatchWString(const void* memoryPtr, const std::wstring& from, const std::wstring& to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchWString(memoryAddress, from, to);
}

bool MemoryUtilities::Internal::IndirectPatchWString(const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to)
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




std::vector<uint8_t> MemoryUtilities::Internal::GetBytes(const void* memoryPtr, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetBytes(memoryAddress, byteCount);
}

std::vector<uint8_t> MemoryUtilities::Internal::GetBytes(const uintptr_t& memoryAddress, size_t byteCount)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(memoryAddress) == false)
        return {};

    std::vector<uint8_t> buffer(byteCount);
    /* Read and return the bytes from the target address. */
    memcpy(buffer.data(), reinterpret_cast<void*>(memoryAddress), byteCount);
    return buffer;
}

bool MemoryUtilities::Internal::SetBytes(const void* memoryPtr, const uint8_t* newBytes, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetBytes(memoryAddress, newBytes, byteCount);
}

bool MemoryUtilities::Internal::SetBytes(const uintptr_t& memoryAddress, const uint8_t* newBytes, size_t byteCount)
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

bool MemoryUtilities::Internal::PatchBytes(const void* memoryPtr, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchBytes(memoryAddress, fromBytes, toBytes, byteCount);
}

bool MemoryUtilities::Internal::PatchBytes(const uintptr_t& memoryAddress, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount)
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

std::vector<uint8_t> MemoryUtilities::Internal::IndirectGetBytes(const void* memoryPtr, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetBytes(memoryAddress, byteCount);
}

std::vector<uint8_t> MemoryUtilities::Internal::IndirectGetBytes(const uintptr_t& memoryAddress, size_t byteCount)
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

bool MemoryUtilities::Internal::IndirectSetBytes(const void* memoryPtr, const uint8_t* newBytes, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetBytes(memoryAddress, newBytes, byteCount);
}

bool MemoryUtilities::Internal::IndirectSetBytes(const uintptr_t& memoryAddress, const uint8_t* newBytes, size_t byteCount)
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

bool MemoryUtilities::Internal::IndirectPatchBytes(const void* memoryPtr, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchBytes(memoryAddress, fromBytes, toBytes, byteCount);
}

bool MemoryUtilities::Internal::IndirectPatchBytes(const uintptr_t& memoryAddress, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount)
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






// ========================================================
// |                      #EXTERNAL                       |
// ========================================================
bool MemoryUtilities::External::IsValidProcessHandle(const HANDLE& hProcess, const bool& requireQueryRights)
{
    if (hProcess == nullptr || hProcess == INVALID_HANDLE_VALUE) // Handle must be non-null and not INVALID_HANDLE_VALUE. Return False if it's not.
        return false;

    if (GetProcessId(hProcess) == 0) // Must be a process handle (GetProcessId returns 0 on failure). Return False if it isn't.
        return false;

    DWORD exitCode = 0;
    if (GetExitCodeProcess(hProcess, &exitCode) == false || exitCode != STILL_ACTIVE)
        return false;

    if (requireQueryRights)
    {
        MEMORY_BASIC_INFORMATION mbi{};
        if (VirtualQueryEx(hProcess, nullptr, &mbi, sizeof(mbi)) != sizeof(mbi))
            return false;
    }

    return true;
}

bool MemoryUtilities::External::IsValidProcessHandle(const HANDLE& hProcess)
{
    return IsValidProcessHandle(hProcess, false);
}




bool MemoryUtilities::External::IsValidPtr(const HANDLE& hProcess, const void* memoryPtr)
{
    if (!IsValidProcessHandle(hProcess)) // Process handle must be valid and suitable. Return False if it's not.
        return false;

    MEMORY_BASIC_INFORMATION mbi{};

    if (VirtualQueryEx(hProcess, memoryPtr, &mbi, sizeof(mbi)) != sizeof(mbi)) // Try to request (query) information about the given memory region, return False if attempt fails.
        return false;

    if (mbi.State != MEM_COMMIT) // Memory region must be committed (actually backed by physical memory). Return False if it's not.
        return false;

    /* Strip out guard and cache flags to examine the core protection flags. */
    DWORD protectionFlags = mbi.Protect & ~(PAGE_GUARD | PAGE_NOCACHE | PAGE_WRITECOMBINE);
    if ((protectionFlags & (PAGE_READONLY | PAGE_READWRITE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE)) == false) // See if we're able to request one of the read permissions (readable or executable+read), return False if we aren't.
        return false;

    return true;
}

bool MemoryUtilities::External::IsValidAddress(const HANDLE& hProcess, const uintptr_t& memoryAddress)
{
    void* memoryPtr = reinterpret_cast<void*>(memoryAddress);
    return IsValidPtr(hProcess, memoryPtr);
}


void* MemoryUtilities::External::PtrAddOffset(const HANDLE& hProcess, const void* memoryPtr, size_t offset)
{
    /* Since void* doesn't support pointer arithmetics, we need to convert it to uintptr_t first. */
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PtrAddOffset(hProcess, memoryAddress, offset);
}

void* MemoryUtilities::External::PtrAddOffset(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t offset)
{
    /* Calculate the new address by adding the offset. */
    uintptr_t newMemoryAddress = AddressAddOffset(hProcess, memoryAddress, offset);
    if (newMemoryAddress == 0x0) // Return null pointer if memory region is invalid.
        return nullptr;

    return reinterpret_cast<void*>(newMemoryAddress);
}


uintptr_t MemoryUtilities::External::AddressAddOffset(const HANDLE& hProcess, const void* memoryPtr, size_t offset)
{
    /* Since void* doesn't support pointer arithmetics, we need to convert it to uintptr_t first. */
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return AddressAddOffset(hProcess, memoryAddress, offset);
}

uintptr_t MemoryUtilities::External::AddressAddOffset(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t offset)
{
    /* Calculate the new address by adding the offset. */
    uintptr_t newMemoryAddress = memoryAddress + offset;

    /* Before dereferencing, verify we can read sizeof(uintptr_t) bytes at 'newMemoryAddress'. */
    /* Abort and return 0 if memory isn't readable */
    if (IsValidAddress(hProcess, newMemoryAddress) == false ||
        IsValidAddress(hProcess, (newMemoryAddress + sizeof(uintptr_t) - 1)) == false)
        return 0x0;

    /* Return the valid address */
    return newMemoryAddress;
}




void* MemoryUtilities::External::PtrFollowPointerChain(const HANDLE& hProcess, const void* memoryPtr, const std::vector<uintptr_t>& memoryOffsets)
{
    /* Since void* doesn't support pointer arithmetics, we need to convert it to uintptr_t first. */
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);

    /* Calculate the new address by following the pointer chain. */
    uintptr_t newMemoryAddress = AddressFollowPointerChain(hProcess, memoryAddress, memoryOffsets);
    if (newMemoryAddress == 0x0) // Return null pointer if memory region is invalid.
        return nullptr;

    return reinterpret_cast<void*>(newMemoryAddress);
}

uintptr_t MemoryUtilities::External::AddressFollowPointerChain(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::vector<uintptr_t>& memoryOffsets)
{
    uintptr_t newMemoryAddress = memoryAddress;
    size_t offsetsCount = memoryOffsets.size();

    for (size_t i = 0; i < offsetsCount; ++i)
    {
        /* Before dereferencing, verify we can read sizeof(uintptr_t) bytes at 'newMemoryAddress'. */
        /* Abort and return 0 if memory isn't readable */
        if (IsValidAddress(hProcess, newMemoryAddress) == false ||
            IsValidAddress(hProcess, (newMemoryAddress + sizeof(uintptr_t) - 1)) == false)
            return 0x0;

        /* Read the next pointer value from memory and add the current offset to advance to the next address in the chain. */
        uintptr_t nextPtr = 0;
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(hProcess,
            reinterpret_cast<LPCVOID>(newMemoryAddress),
            &nextPtr,
            sizeof(nextPtr),
            &bytesRead) || bytesRead != sizeof(nextPtr))
            return 0x0;

        newMemoryAddress = nextPtr;
        newMemoryAddress += memoryOffsets[i];
    }

    /* After processing all offsets, 'ptr' should point to the final data. */
    return newMemoryAddress;
}




bool MemoryUtilities::External::GetBool(const HANDLE& hProcess, const void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return GetBool(hProcess, memoryAddress);
}

bool MemoryUtilities::External::GetBool(const HANDLE& hProcess, const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(hProcess, memoryAddress) == false)
        return false;

    /* Read and return the boolean value from the target address. */
    bool value = false;
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(memoryAddress),
        &value, sizeof(value), &bytesRead) || bytesRead != sizeof(value))
        return false;

    return value;
}

bool MemoryUtilities::External::SetBool(const HANDLE& hProcess, const void* memoryPtr, bool newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return SetBool(hProcess, memoryAddress, newValue);
}

bool MemoryUtilities::External::SetBool(const HANDLE& hProcess, const uintptr_t& memoryAddress, bool newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(hProcess, memoryAddress) == false)
        return false;

    /* Cast the address to bool* */
    LPVOID targetBool = reinterpret_cast<LPVOID>(memoryAddress);
    SIZE_T byteSize = sizeof(bool);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtectEx(hProcess, targetBool, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new boolean value. */
    SIZE_T bytesWritten = 0;
    BOOL ok = WriteProcessMemory(hProcess, targetBool, &newValue, byteSize, &bytesWritten);

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtectEx(hProcess, targetBool, byteSize, oldProtect, &tmp);

    return ok && bytesWritten == byteSize;
}

bool MemoryUtilities::External::PatchBool(const HANDLE& hProcess, const void* memoryPtr, bool from, bool to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return PatchBool(hProcess, memoryAddress, from, to);
}

bool MemoryUtilities::External::PatchBool(const HANDLE& hProcess, const uintptr_t& memoryAddress, bool from, bool to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(hProcess, memoryAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    bool current = false;
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(memoryAddress),
        &current, sizeof(current), &bytesRead) || bytesRead != sizeof(current))
        return false;
    if (current != from) // Only patch if the current value matches 'from'.
        return false;

    SIZE_T byteSize = sizeof(bool);

    /* Make the memory region writable */
    DWORD oldProtect;
    LPVOID targetBool = reinterpret_cast<LPVOID>(memoryAddress);
    if (VirtualProtectEx(hProcess, targetBool, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new boolean value */
    SIZE_T bytesWritten = 0;
    BOOL ok = WriteProcessMemory(hProcess, targetBool, &to, byteSize, &bytesWritten);

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtectEx(hProcess, targetBool, byteSize, oldProtect, &tmp);

    return ok && bytesWritten == byteSize;
}

bool MemoryUtilities::External::IndirectGetBool(const HANDLE& hProcess, const void* memoryPtr)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectGetBool(hProcess, memoryAddress);
}

bool MemoryUtilities::External::IndirectGetBool(const HANDLE& hProcess, const uintptr_t& memoryAddress)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(hProcess, memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = 0;
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(memoryAddress),
        &dataAddress, sizeof(dataAddress), &bytesRead) || bytesRead != sizeof(dataAddress))
        return false;
    if (IsValidAddress(hProcess, dataAddress) == false)
        return false;

    /* Read and return the boolean value from the target address. */
    bool value = false;
    bytesRead = 0;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(dataAddress),
        &value, sizeof(value), &bytesRead) || bytesRead != sizeof(value))
        return false;

    return value;
}

bool MemoryUtilities::External::IndirectSetBool(const HANDLE& hProcess, const void* memoryPtr, bool newValue)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectSetBool(hProcess, memoryAddress, newValue);
}

bool MemoryUtilities::External::IndirectSetBool(const HANDLE& hProcess, const uintptr_t& memoryAddress, bool newValue)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(hProcess, memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = 0;
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(memoryAddress),
        &dataAddress, sizeof(dataAddress), &bytesRead) || bytesRead != sizeof(dataAddress))
        return false;
    if (IsValidAddress(hProcess, dataAddress) == false)
        return false;

    /* Cast the address to bool* */
    LPVOID targetBool = reinterpret_cast<LPVOID>(dataAddress);
    SIZE_T byteSize = sizeof(bool);

    /* Make the memory region writable. */
    DWORD oldProtect;
    if (VirtualProtectEx(hProcess, targetBool, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new boolean value. */
    SIZE_T bytesWritten = 0;
    BOOL ok = WriteProcessMemory(hProcess, targetBool, &newValue, byteSize, &bytesWritten);

    /* Restore the original protection. */
    DWORD tmp;
    VirtualProtectEx(hProcess, targetBool, byteSize, oldProtect, &tmp);

    return ok && bytesWritten == byteSize;
}

bool MemoryUtilities::External::IndirectPatchBool(const HANDLE& hProcess, const void* memoryPtr, bool from, bool to)
{
    uintptr_t memoryAddress = reinterpret_cast<uintptr_t>(memoryPtr);
    return IndirectPatchBool(hProcess, memoryAddress, from, to);
}

bool MemoryUtilities::External::IndirectPatchBool(const HANDLE& hProcess, const uintptr_t& memoryAddress, bool from, bool to)
{
    /* Verify that the address is valid. */
    if (IsValidAddress(hProcess, memoryAddress) == false)
        return false;

    /* Read the data pointer from memoryAddress */
    uintptr_t dataAddress = 0;
    SIZE_T bytesRead = 0;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(memoryAddress),
        &dataAddress, sizeof(dataAddress), &bytesRead) || bytesRead != sizeof(dataAddress))
        return false;
    if (IsValidAddress(hProcess, dataAddress) == false)
        return false;

    /* Verify that the current value matches the expected one. */
    bool current = false;
    if (!ReadProcessMemory(hProcess, reinterpret_cast<LPCVOID>(dataAddress),
        &current, sizeof(current), &bytesRead) || bytesRead != sizeof(current))
        return false;
    if (current != from) // Only patch if the current value matches 'from'.
        return false;
    SIZE_T byteSize = sizeof(bool);

    /* Make the memory region writable */
    DWORD oldProtect;
    LPVOID targetBool = reinterpret_cast<LPVOID>(dataAddress);
    if (VirtualProtectEx(hProcess, targetBool, byteSize, PAGE_EXECUTE_READWRITE, &oldProtect) == false)
        return false;

    /* Write the new boolean value */
    SIZE_T bytesWritten = 0;
    BOOL ok = WriteProcessMemory(hProcess, targetBool, &to, byteSize, &bytesWritten);

    /* Restore the original protection */
    DWORD tmp;
    VirtualProtectEx(hProcess, targetBool, byteSize, oldProtect, &tmp);

    return ok && bytesWritten == byteSize;
}