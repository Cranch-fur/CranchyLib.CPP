#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <optional>
#include <Psapi.h>






namespace MemoryUtilities
{
	class Convertion
	{
	public:
		/**
		* @brief Converts a single hexadecimal character to its int16 value.
		* @param c - A character in a ['0'..'9','A'..'F','a'..'f'] range.
		* @return 0–15 on success, or -1 if `c` is not a valid hex digit.
		*/
		static int16_t HEXChar_ToInt16(const char& c);
		/**
		* @brief Converts a int16 value to its single hexadecimal character.
		* @param value - A signed 16-bit integer expected to be in the range 0–15.
		* @return The hex character ('0'–'9', 'A'–'F'), or '?' if out of range.
		*/
		static char Int16_ToHEXChar(const int16_t& value);


		/**
		* @brief Converts a hexadecimal string to vector of bytes.  
		* @param memoryPattern - A string mask of hexadecimal byte values (with wildcards). 
		* @return A vector of optional<uint8_t>, where wildcards ("??") are represented by std::nullopt. Any parsing error results in an empty vector.
		*/
		static std::vector<std::optional<uint8_t>> MemoryPattern_ToBytesPattern(const std::string& memoryPattern);
	};






	class Internal
	{
		// Description: Functions within the class allows to manipulate memory of process program is running in.
		// Search Tags: #internal, #dll, #asi, #process, #readprocessmemory, #writeprocessmemory.
	public:
		/**
		* @brief Determines whether a given pointer refers to a valid, committed, and readable (or executable) memory region.
		* @param memoryPtr - Pointer to the memory location to check.
		* @return true if the region is committed, and has read or execute+read permissions; false otherwise.
		*/
		static bool IsValidPtr(const void* memoryPtr);
		/**
		* @brief Determines whether a given memory address refers to a valid, committed, and readable (or executable) memory region.
		* @param memoryAddress - Address of the memory location to check.
		* @return true if the region is committed, and has read or execute+read permissions; false otherwise.
		*/
		static bool IsValidAddress(const uintptr_t& memoryAddress);

		/**
		* @brief Adds a byte offset to a base memory address and verifies that the resulting address range is readable.
		* @param memoryPtr - Pointer to the starting memory location to which the offset will be added.
		* @param offset - The number of bytes to add to the base address.
		* @return The new valid pointer to the memory location if both the beginning and end of the target region are readable;
		*         otherwise returns 'nullptr' to indicate an invalid or unreadable address.
		*/
		static void* PtrAddOffset(const void* memoryPtr, size_t offset);
		/**
		* @brief Adds a byte offset to a base memory address and verifies that the resulting address range is readable.
		* @param memoryAddress - Address of the memory location to which the offset will be added.
		* @param offset - The number of bytes to add to the base address.
		* @return The new valid pointer to the memory location if both the beginning and end of the target region are readable;
		*         otherwise returns 'nullptr' to indicate an invalid or unreadable address.
		*/
		static void* PtrAddOffset(const uintptr_t& memoryAddress, size_t offset);

		/**
		* @brief Adds a byte offset to a base memory address and verifies that the resulting address range is readable.
		* @param memoryPtr - Pointer to the starting memory location to which the offset will be added.
		* @param offset - The number of bytes to add to the base address.
		* @return The new valid memory address if both the beginning and end of the target region are readable;
		*         otherwise returns '0x0' to indicate an invalid or unreadable address.
		*/
		static uintptr_t AddressAddOffset(const void* memoryPtr, size_t offset);
		/**
		* @brief Adds a byte offset to a base memory address and verifies that the resulting address range is readable.
		* @param memoryAddress - Address of the memory location to which the offset will be added.
		* @param offset - The number of bytes to add to the base address.
		* @return The new valid memory address if both the beginning and end of the target region are readable;
		*         otherwise returns '0x0' to indicate an invalid or unreadable address.
		*/
		static uintptr_t AddressAddOffset(const uintptr_t& memoryAddress, size_t offset);

		/**
		* @brief Follows a chain of pointers starting from a base address, applying a sequence of offsets.
		* @param memoryPtr - Pointer to the starting memory location from which the pointer chain will be resolved.
		* @return The new valid memory address if pointer chain was successfully resolved and final destination was reached;
		*         otherwise returns 'nullptr' to indicate an invalid or unreadable address.
		*/
		static void* PtrFollowPointerChain(const void* memoryPtr, const std::vector<uintptr_t>& memoryOffsets);
		/**
		* @brief Follows a chain of pointers starting from a base address, applying a sequence of offsets.
		* @param memoryAddress - Address of the memory location from which the pointer chain will be resolved.
		* @return The new valid memory address if pointer chain was successfully resolved and final destination was reached;
		*         otherwise returns '0x0' to indicate an invalid or unreadable address.
		*/
		static uintptr_t AddressFollowPointerChain(const uintptr_t& memoryAddress, const std::vector<uintptr_t>& memoryOffsets);




		/**
		* @brief [EXPERIMENTAL] Function wasn't properly tested just yet and is more of an theoretical idea.
		*/
		static uintptr_t ScanForBytesPattern(const uint8_t* startingAddress, size_t size, const std::vector<std::optional<uint8_t>>& bytesPattern);
		/**
		* @brief [EXPERIMENTAL] Function wasn't properly tested just yet and is more of an theoretical idea.
		*/
		static uintptr_t ScanForMemoryPattern(const uint8_t* startingAddress, size_t size, const std::string& memoryPattern);


		/**
		* @brief [EXPERIMENTAL] Function wasn't properly tested just yet and is more of an theoretical idea.
		*/
		static uintptr_t SearchForBytesPattern(const std::vector<std::optional<uint8_t>>& bytesPattern);
		/**
		* @brief [EXPERIMENTAL] Function wasn't properly tested just yet and is more of an theoretical idea.
		*/
		static uintptr_t SearchForMemoryPattern(const std::string& memoryPattern);




		/* For when 'memoryAddress' contains the actual value. */
		static bool GetBool(const void* memoryPtr);
		static bool GetBool(const uintptr_t& memoryAddress);

		static bool SetBool(const void* memoryPtr, bool newValue);
		static bool SetBool(const uintptr_t& memoryAddress, bool newValue);

		static bool PatchBool(const void* memoryPtr, bool from, bool to);
		static bool PatchBool(const uintptr_t& memoryAddress, bool from, bool to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static bool IndirectGetBool(const void* memoryPtr);
		static bool IndirectGetBool(const uintptr_t& memoryAddress);

		static bool IndirectSetBool(const void* memoryPtr, bool newValue);
		static bool IndirectSetBool(const uintptr_t& memoryAddress, bool newValue);

		static bool IndirectPatchBool(const void* memoryPtr, bool from, bool to);
		static bool IndirectPatchBool(const uintptr_t& memoryAddress, bool from, bool to);




		/* For when 'memoryAddress' contains the actual value. */
		static int8_t GetInt8(const void* memoryPtr);
		static int8_t GetInt8(const uintptr_t& memoryAddress);

		static bool   SetInt8(const void* memoryPtr, int8_t newValue);
		static bool   SetInt8(const uintptr_t& memoryAddress, int8_t newValue);

		static bool   PatchInt8(const void* memoryPtr, int8_t from, int8_t to);
		static bool   PatchInt8(const uintptr_t& memoryAddress, int8_t from, int8_t to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static int8_t IndirectGetInt8(const void* memoryPtr);
		static int8_t IndirectGetInt8(const uintptr_t& memoryAddress);

		static bool   IndirectSetInt8(const void* memoryPtr, int8_t newValue);
		static bool   IndirectSetInt8(const uintptr_t& memoryAddress, int8_t newValue);

		static bool   IndirectPatchInt8(const void* memoryPtr, int8_t from, int8_t to);
		static bool   IndirectPatchInt8(const uintptr_t& memoryAddress, int8_t from, int8_t to);




		/* For when 'memoryAddress' contains the actual value. */
		static int16_t GetInt16(const void* memoryPtr);
		static int16_t GetInt16(const uintptr_t& memoryAddress);

		static bool    SetInt16(const void* memoryPtr, int16_t newValue);
		static bool    SetInt16(const uintptr_t& memoryAddress, int16_t newValue);

		static bool    PatchInt16(const void* memoryPtr, int16_t from, int16_t to);
		static bool    PatchInt16(const uintptr_t& memoryAddress, int16_t from, int16_t to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static int16_t IndirectGetInt16(const void* memoryPtr);
		static int16_t IndirectGetInt16(const uintptr_t& memoryAddress);

		static bool    IndirectSetInt16(const void* memoryPtr, int16_t newValue);
		static bool    IndirectSetInt16(const uintptr_t& memoryAddress, int16_t newValue);

		static bool    IndirectPatchInt16(const void* memoryPtr, int16_t from, int16_t to);
		static bool    IndirectPatchInt16(const uintptr_t& memoryAddress, int16_t from, int16_t to);




		/* For when 'memoryAddress' contains the actual value. */
		static int32_t GetInt32(const void* memoryPtr);
		static int32_t GetInt32(const uintptr_t& memoryAddress);

		static bool    SetInt32(const void* memoryPtr, int32_t newValue);
		static bool    SetInt32(const uintptr_t& memoryAddress, int32_t newValue);

		static bool    PatchInt32(const void* memoryPtr, int32_t from, int32_t to);
		static bool    PatchInt32(const uintptr_t& memoryAddress, int32_t from, int32_t to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static int32_t IndirectGetInt32(const void* memoryPtr);
		static int32_t IndirectGetInt32(const uintptr_t& memoryAddress);

		static bool	   IndirectSetInt32(const void* memoryPtr, int32_t newValue);
		static bool	   IndirectSetInt32(const uintptr_t& memoryAddress, int32_t newValue);

		static bool    IndirectPatchInt32(const void* memoryPtr, int32_t from, int32_t to);
		static bool    IndirectPatchInt32(const uintptr_t& memoryAddress, int32_t from, int32_t to);




		/* For when 'memoryAddress' contains the actual value. */
		static int64_t GetInt64(const void* memoryPtr);
		static int64_t GetInt64(const uintptr_t& memoryAddress);

		static bool    SetInt64(const void* memoryPtr, int64_t newValue);
		static bool    SetInt64(const uintptr_t& memoryAddress, int64_t newValue);

		static bool    PatchInt64(const void* memoryPtr, int64_t from, int64_t to);
		static bool    PatchInt64(const uintptr_t& memoryAddress, int64_t from, int64_t to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static int64_t IndirectGetInt64(const void* memoryPtr);
		static int64_t IndirectGetInt64(const uintptr_t& memoryAddress);

		static bool    IndirectSetInt64(const void* memoryPtr, int64_t newValue);
		static bool    IndirectSetInt64(const uintptr_t& memoryAddress, int64_t newValue);

		static bool    IndirectPatchInt64(const void* memoryPtr, int64_t from, int64_t to);
		static bool    IndirectPatchInt64(const uintptr_t& memoryAddress, int64_t from, int64_t to);




		/* For when 'memoryAddress' contains the actual value. */
		static float GetFloat(const void* memoryPtr);
		static float GetFloat(const uintptr_t& memoryAddress);

		static bool  SetFloat(const void* memoryPtr, float newValue);
		static bool  SetFloat(const uintptr_t& memoryAddress, float newValue);

		static bool  PatchFloat(const void* memoryPtr, float from, float to);
		static bool  PatchFloat(const uintptr_t& memoryAddress, float from, float to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static float IndirectGetFloat(const void* memoryPtr);
		static float IndirectGetFloat(const uintptr_t& memoryAddress);

		static bool  IndirectSetFloat(const void* memoryPtr, float newValue);
		static bool  IndirectSetFloat(const uintptr_t& memoryAddress, float newValue);

		static bool  IndirectPatchFloat(const void* memoryPtr, float from, float to);
		static bool  IndirectPatchFloat(const uintptr_t& memoryAddress, float from, float to);




		/* For when 'memoryAddress' contains the actual value. */
		static double GetDouble(const void* memoryPtr);
		static double GetDouble(const uintptr_t& memoryAddress);

		static bool   SetDouble(const void* memoryPtr, double newValue);
		static bool   SetDouble(const uintptr_t& memoryAddress, double newValue);

		static bool   PatchDouble(const void* memoryPtr, double from, double to);
		static bool   PatchDouble(const uintptr_t& memoryAddress, double from, double to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static double IndirectGetDouble(const void* memoryPtr);
		static double IndirectGetDouble(const uintptr_t& memoryAddress);

		static bool   IndirectSetDouble(const void* memoryPtr, double newValue);
		static bool   IndirectSetDouble(const uintptr_t& memoryAddress, double newValue);

		static bool   IndirectPatchDouble(const void* memoryPtr, double from, double to);
		static bool   IndirectPatchDouble(const uintptr_t& memoryAddress, double from, double to);




		/* For when 'memoryAddress' contains the actual value. */
		static std::string GetString(const void* memoryPtr);
		static std::string GetString(const void* memoryPtr, size_t maxLength);
		static std::string GetString(const uintptr_t& memoryAddress);
		static std::string GetString(const uintptr_t& memoryAddress, size_t maxLength);

		static bool		   SetString(const void* memoryPtr, const std::string& newValue);
		static bool		   SetString(const uintptr_t& memoryAddress, const std::string& newValue);

		static bool		   PatchString(const void* memoryPtr, const std::string& from, const std::string& to);
		static bool		   PatchString(const uintptr_t& memoryAddress, const std::string& from, const std::string& to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static std::string IndirectGetString(const void* memoryPtr);
		static std::string IndirectGetString(const void* memoryPtr, size_t maxLength);
		static std::string IndirectGetString(const uintptr_t& memoryAddress);
		static std::string IndirectGetString(const uintptr_t& memoryAddress, size_t maxLength);

		static bool		   IndirectSetString(const void* memoryPtr, const std::string& newValue);
		static bool		   IndirectSetString(const uintptr_t& memoryAddress, const std::string& newValue);

		static bool		   IndirectPatchString(const void* memoryPtr, const std::string& from, const std::string& to);
		static bool		   IndirectPatchString(const uintptr_t& memoryAddress, const std::string& from, const std::string& to);


		/* For when 'memoryAddress' contains the actual value. */
		static std::wstring GetWString(const void* memoryPtr);
		static std::wstring GetWString(const void* memoryPtr, size_t maxLength);
		static std::wstring GetWString(const uintptr_t& memoryAddress);
		static std::wstring GetWString(const uintptr_t& memoryAddress, size_t maxLength);

		static bool			SetWString(const void* memoryPtr, const std::wstring& newValue);
		static bool			SetWString(const uintptr_t& memoryAddress, const std::wstring& newValue);

		static bool			PatchWString(const void* memoryPtr, const std::wstring& from, const std::wstring& to);
		static bool			PatchWString(const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static std::wstring IndirectGetWString(const void* memoryPtr);
		static std::wstring IndirectGetWString(const void* memoryPtr, size_t maxLength);
		static std::wstring IndirectGetWString(const uintptr_t& memoryAddress);
		static std::wstring IndirectGetWString(const uintptr_t& memoryAddress, size_t maxLength);

		static bool			IndirectSetWString(const void* memoryPtr, const std::wstring& newValue);
		static bool			IndirectSetWString(const uintptr_t& memoryAddress, const std::wstring& newValue);

		static bool			IndirectPatchWString(const void* memoryPtr, const std::wstring& from, const std::wstring& to);
		static bool			IndirectPatchWString(const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to);




		/* For when 'memoryAddress' contains the actual value. */
		static std::vector<uint8_t> GetBytes(const void* memoryPtr, size_t byteCount);
		static std::vector<uint8_t> GetBytes(const uintptr_t& memoryAddress, size_t byteCount);

		static bool					SetBytes(const void* memoryPtr, const std::vector<uint8_t>& newBytes);
		static bool					SetBytes(const uintptr_t& memoryAddress, const std::vector<uint8_t>& newBytes);

		static bool					PatchBytes(const void* memoryPtr, const std::vector<uint8_t>& fromBytes, const std::vector<uint8_t>& toBytes);
		static bool					PatchBytes(const uintptr_t& memoryAddress, const std::vector<uint8_t>& fromBytes, const std::vector<uint8_t>& toBytes);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static std::vector<uint8_t> IndirectGetBytes(const void* memoryPtr, size_t byteCount);
		static std::vector<uint8_t> IndirectGetBytes(const uintptr_t& memoryAddress, size_t byteCount);

		static bool					IndirectSetBytes(const void* memoryPtr, const std::vector<uint8_t>& newBytes);
		static bool					IndirectSetBytes(const uintptr_t& memoryAddress, const std::vector<uint8_t>& newBytes);

		static bool					IndirectPatchBytes(const void* memoryPtr, const std::vector<uint8_t>& fromBytes, const std::vector<uint8_t>& toBytes);
		static bool					IndirectPatchBytes(const uintptr_t& memoryAddress, const std::vector<uint8_t>& fromBytes, const std::vector<uint8_t>& toBytes);
	};






	class External
	{
		// Description: Functions within the class allows to manipulate memory of 3'rd party processes.
		// Search Tags: #external, #exe, #pid, #process, #readprocessmemory, #writeprocessmemory, #handle.
	private:
		static std::string ReadRemoteString(const HANDLE& hProcess, const uintptr_t memoryAddress, size_t maxLength = 256);
		static std::wstring ReadRemoteWString(const HANDLE& hProcess, const uintptr_t memoryAddress, size_t maxLength = 256);




	public:
		/**
		* @brief Determines whether a given process HANDLE is valid and suitable for memory queries.
		* @param hProcess - Process HANDLE to check.
		* @param requireQueryRights - If true, additionally tries to query any region in the target
        *                             process (VirtualQueryEx) to ensure basic query rights. If false,
        *                             only basic handle validity is checked.
		* @return true if the handle is valid and can be used to query memory; false otherwise.
		*/
		static bool IsValidProcessHandle(const HANDLE& hProcess, const bool& requireQueryRights);
		/**
		* @brief Determines whether a given process HANDLE is valid and suitable for memory queries.
		* @param hProcess - Process HANDLE to check.
		* @return true if the handle is valid and can be used to query memory; false otherwise.
		*/
		static bool IsValidProcessHandle(const HANDLE& hProcess);

		/**
		* @brief Determines whether a given pointer refers to a valid, committed, and readable (or executable) memory region.
		* @param hProcess - Process HANDLE in whose address space to check.
		* @param memoryPtr - Pointer to the memory location to check.
		* @return true if the region is committed, and has read or execute+read permissions; false otherwise.
		*/
		static bool IsValidPtr(const HANDLE& hProcess, const void* memoryPtr);
		/**
		* @brief Determines whether a given memory address refers to a valid, committed, and readable (or executable) memory region.
		* @param hProcess - Process HANDLE in whose address space to check.
		* @param memoryAddress - Address of the memory location to check.
		* @return true if the region is committed, and has read or execute+read permissions; false otherwise.
		*/
		static bool IsValidAddress(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		/**
		* @brief Adds a byte offset to a base memory address and verifies that the resulting address range is readable.
		* @param hProcess - Process HANDLE in whose address space to operate.
		* @param memoryPtr - Pointer to the starting memory location to which the offset will be added.
		* @param offset - The number of bytes to add to the base address.
		* @return The new valid pointer to the memory location if both the beginning and end of the target region are readable;
		*         otherwise returns 'nullptr' to indicate an invalid or unreadable address.
		*/
		static void* PtrAddOffset(const HANDLE& hProcess, const void* memoryPtr, size_t offset);
		/**
		* @brief Adds a byte offset to a base memory address and verifies that the resulting address range is readable.
		* @param hProcess - Process HANDLE in whose address space to operate.
		* @param memoryAddress - Address of the memory location to which the offset will be added.
		* @param offset - The number of bytes to add to the base address.
		* @return The new valid pointer to the memory location if both the beginning and end of the target region are readable;
		*         otherwise returns 'nullptr' to indicate an invalid or unreadable address.
		*/
		static void* PtrAddOffset(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t offset);

		/**
		* @brief Adds a byte offset to a base memory address and verifies that the resulting address range is readable.
		* @param hProcess - Process HANDLE in whose address space to operate.
		* @param memoryPtr - Pointer to the starting memory location to which the offset will be added.
		* @param offset - The number of bytes to add to the base address.
		* @return The new valid memory address if both the beginning and end of the target region are readable;
		*         otherwise returns '0x0' to indicate an invalid or unreadable address.
		*/
		static uintptr_t AddressAddOffset(const HANDLE& hProcess, const void* memoryPtr, size_t offset);
		/**
		* @brief Adds a byte offset to a base memory address and verifies that the resulting address range is readable.
		* @param hProcess - Process HANDLE in whose address space to operate.
		* @param memoryAddress - Address of the memory location to which the offset will be added.
		* @param offset - The number of bytes to add to the base address.
		* @return The new valid memory address if both the beginning and end of the target region are readable;
		*         otherwise returns '0x0' to indicate an invalid or unreadable address.
		*/
		static uintptr_t AddressAddOffset(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t offset);

		/**
		* @brief Follows a chain of pointers starting from a base address, applying a sequence of offsets.
		* @param hProcess - Process HANDLE in whose address space to operate.
		* @param memoryPtr - Pointer to the starting memory location from which the pointer chain will be resolved.
		* @return The new valid memory address if pointer chain was successfully resolved and final destination was reached;
		*         otherwise returns 'nullptr' to indicate an invalid or unreadable address.
		*/
		static void* PtrFollowPointerChain(const HANDLE& hProcess, const void* memoryPtr, const std::vector<uintptr_t>& memoryOffsets);
		/**
		* @brief Follows a chain of pointers starting from a base address, applying a sequence of offsets.
		* @param hProcess - Process HANDLE in whose address space to operate.
		* @param memoryAddress - Address of the memory location from which the pointer chain will be resolved.
		* @return The new valid memory address if pointer chain was successfully resolved and final destination was reached;
		*         otherwise returns '0x0' to indicate an invalid or unreadable address.
		*/
		static uintptr_t AddressFollowPointerChain(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::vector<uintptr_t>& memoryOffsets);




		/* For when 'memoryAddress' contains the actual value. */
		static bool GetBool(const HANDLE& hProcess, const void* memoryPtr);
		static bool GetBool(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool SetBool(const HANDLE& hProcess, const void* memoryPtr, bool newValue);
		static bool SetBool(const HANDLE& hProcess, const uintptr_t& memoryAddress, bool newValue);

		static bool PatchBool(const HANDLE& hProcess, const void* memoryPtr, bool from, bool to);
		static bool PatchBool(const HANDLE& hProcess, const uintptr_t& memoryAddress, bool from, bool to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static bool IndirectGetBool(const HANDLE& hProcess, const void* memoryPtr);
		static bool IndirectGetBool(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool IndirectSetBool(const HANDLE& hProcess, const void* memoryPtr, bool newValue);
		static bool IndirectSetBool(const HANDLE& hProcess, const uintptr_t& memoryAddress, bool newValue);

		static bool IndirectPatchBool(const HANDLE& hProcess, const void* memoryPtr, bool from, bool to);
		static bool IndirectPatchBool(const HANDLE& hProcess, const uintptr_t& memoryAddress, bool from, bool to);




		/* For when 'memoryAddress' contains the actual value. */
		static int8_t GetInt8(const HANDLE& hProcess, const void* memoryPtr);
		static int8_t GetInt8(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool   SetInt8(const HANDLE& hProcess, const void* memoryPtr, int8_t newValue);
		static bool   SetInt8(const HANDLE& hProcess, const uintptr_t& memoryAddress, int8_t newValue);

		static bool   PatchInt8(const HANDLE& hProcess, const void* memoryPtr, int8_t from, int8_t to);
		static bool   PatchInt8(const HANDLE& hProcess, const uintptr_t& memoryAddress, int8_t from, int8_t to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static int8_t IndirectGetInt8(const HANDLE& hProcess, const void* memoryPtr);
		static int8_t IndirectGetInt8(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool   IndirectSetInt8(const HANDLE& hProcess, const void* memoryPtr, int8_t newValue);
		static bool   IndirectSetInt8(const HANDLE& hProcess, const uintptr_t& memoryAddress, int8_t newValue);

		static bool   IndirectPatchInt8(const HANDLE& hProcess, const void* memoryPtr, int8_t from, int8_t to);
		static bool   IndirectPatchInt8(const HANDLE& hProcess, const uintptr_t& memoryAddress, int8_t from, int8_t to);




		/* For when 'memoryAddress' contains the actual value. */
		static int16_t GetInt16(const HANDLE& hProcess, const void* memoryPtr);
		static int16_t GetInt16(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool    SetInt16(const HANDLE& hProcess, const void* memoryPtr, int16_t newValue);
		static bool    SetInt16(const HANDLE& hProcess, const uintptr_t& memoryAddress, int16_t newValue);

		static bool    PatchInt16(const HANDLE& hProcess, const void* memoryPtr, int16_t from, int16_t to);
		static bool    PatchInt16(const HANDLE& hProcess, const uintptr_t& memoryAddress, int16_t from, int16_t to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static int16_t IndirectGetInt16(const HANDLE& hProcess, const void* memoryPtr);
		static int16_t IndirectGetInt16(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool    IndirectSetInt16(const HANDLE& hProcess, const void* memoryPtr, int16_t newValue);
		static bool    IndirectSetInt16(const HANDLE& hProcess, const uintptr_t& memoryAddress, int16_t newValue);

		static bool    IndirectPatchInt16(const HANDLE& hProcess, const void* memoryPtr, int16_t from, int16_t to);
		static bool    IndirectPatchInt16(const HANDLE& hProcess, const uintptr_t& memoryAddress, int16_t from, int16_t to);




		/* For when 'memoryAddress' contains the actual value. */
		static int32_t GetInt32(const HANDLE& hProcess, const void* memoryPtr);
		static int32_t GetInt32(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool    SetInt32(const HANDLE& hProcess, const void* memoryPtr, int32_t newValue);
		static bool    SetInt32(const HANDLE& hProcess, const uintptr_t& memoryAddress, int32_t newValue);

		static bool    PatchInt32(const HANDLE& hProcess, const void* memoryPtr, int32_t from, int32_t to);
		static bool    PatchInt32(const HANDLE& hProcess, const uintptr_t& memoryAddress, int32_t from, int32_t to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static int32_t IndirectGetInt32(const HANDLE& hProcess, const void* memoryPtr);
		static int32_t IndirectGetInt32(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool	   IndirectSetInt32(const HANDLE& hProcess, const void* memoryPtr, int32_t newValue);
		static bool	   IndirectSetInt32(const HANDLE& hProcess, const uintptr_t& memoryAddress, int32_t newValue);

		static bool    IndirectPatchInt32(const HANDLE& hProcess, const void* memoryPtr, int32_t from, int32_t to);
		static bool    IndirectPatchInt32(const HANDLE& hProcess, const uintptr_t& memoryAddress, int32_t from, int32_t to);




		/* For when 'memoryAddress' contains the actual value. */
		static int64_t GetInt64(const HANDLE& hProcess, const void* memoryPtr);
		static int64_t GetInt64(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool    SetInt64(const HANDLE& hProcess, const void* memoryPtr, int64_t newValue);
		static bool    SetInt64(const HANDLE& hProcess, const uintptr_t& memoryAddress, int64_t newValue);

		static bool    PatchInt64(const HANDLE& hProcess, const void* memoryPtr, int64_t from, int64_t to);
		static bool    PatchInt64(const HANDLE& hProcess, const uintptr_t& memoryAddress, int64_t from, int64_t to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static int64_t IndirectGetInt64(const HANDLE& hProcess, const void* memoryPtr);
		static int64_t IndirectGetInt64(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool	   IndirectSetInt64(const HANDLE& hProcess, const void* memoryPtr, int64_t newValue);
		static bool	   IndirectSetInt64(const HANDLE& hProcess, const uintptr_t& memoryAddress, int64_t newValue);

		static bool    IndirectPatchInt64(const HANDLE& hProcess, const void* memoryPtr, int64_t from, int64_t to);
		static bool    IndirectPatchInt64(const HANDLE& hProcess, const uintptr_t& memoryAddress, int64_t from, int64_t to);




		/* For when 'memoryAddress' contains the actual value. */
		static float GetFloat(const HANDLE& hProcess, const void* memoryPtr);
		static float GetFloat(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool  SetFloat(const HANDLE& hProcess, const void* memoryPtr, float newValue);
		static bool  SetFloat(const HANDLE& hProcess, const uintptr_t& memoryAddress, float newValue);

		static bool  PatchFloat(const HANDLE& hProcess, const void* memoryPtr, float from, float to);
		static bool  PatchFloat(const HANDLE& hProcess, const uintptr_t& memoryAddress, float from, float to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static float IndirectGetFloat(const HANDLE& hProcess, const void* memoryPtr);
		static float IndirectGetFloat(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool	 IndirectSetFloat(const HANDLE& hProcess, const void* memoryPtr, float newValue);
		static bool	 IndirectSetFloat(const HANDLE& hProcess, const uintptr_t& memoryAddress, float newValue);

		static bool  IndirectPatchFloat(const HANDLE& hProcess, const void* memoryPtr, float from, float to);
		static bool  IndirectPatchFloat(const HANDLE& hProcess, const uintptr_t& memoryAddress, float from, float to);




		/* For when 'memoryAddress' contains the actual value. */
		static double GetDouble(const HANDLE& hProcess, const void* memoryPtr);
		static double GetDouble(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool  SetDouble(const HANDLE& hProcess, const void* memoryPtr, double newValue);
		static bool  SetDouble(const HANDLE& hProcess, const uintptr_t& memoryAddress, double newValue);

		static bool  PatchDouble(const HANDLE& hProcess, const void* memoryPtr, double from, double to);
		static bool  PatchDouble(const HANDLE& hProcess, const uintptr_t& memoryAddress, double from, double to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static double IndirectGetDouble(const HANDLE& hProcess, const void* memoryPtr);
		static double IndirectGetDouble(const HANDLE& hProcess, const uintptr_t& memoryAddress);

		static bool	 IndirectSetDouble(const HANDLE& hProcess, const void* memoryPtr, double newValue);
		static bool	 IndirectSetDouble(const HANDLE& hProcess, const uintptr_t& memoryAddress, double newValue);

		static bool  IndirectPatchDouble(const HANDLE& hProcess, const void* memoryPtr, double from, double to);
		static bool  IndirectPatchDouble(const HANDLE& hProcess, const uintptr_t& memoryAddress, double from, double to);




		/* For when 'memoryAddress' contains the actual value. */
		static std::string GetString(const HANDLE& hProcess, const void* memoryPtr);
		static std::string GetString(const HANDLE& hProcess, const void* memoryPtr, size_t maxLength);
		static std::string GetString(const HANDLE& hProcess, const uintptr_t& memoryAddress);
		static std::string GetString(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t maxLength);

		static bool		   SetString(const HANDLE& hProcess, const void* memoryPtr, const std::string& newValue);
		static bool		   SetString(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::string& newValue);

		static bool		   PatchString(const HANDLE& hProcess, const void* memoryPtr, const std::string& from, const std::string& to);
		static bool		   PatchString(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::string& from, const std::string& to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static std::string IndirectGetString(const HANDLE& hProcess, const void* memoryPtr);
		static std::string IndirectGetString(const HANDLE& hProcess, const void* memoryPtr, size_t maxLength);
		static std::string IndirectGetString(const HANDLE& hProcess, const uintptr_t& memoryAddress);
		static std::string IndirectGetString(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t maxLength);

		static bool		   IndirectSetString(const HANDLE& hProcess, const void* memoryPtr, const std::string& newValue);
		static bool		   IndirectSetString(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::string& newValue);

		static bool		   IndirectPatchString(const HANDLE& hProcess, const void* memoryPtr, const std::string& from, const std::string& to);
		static bool		   IndirectPatchString(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::string& from, const std::string& to);




		/* For when 'memoryAddress' contains the actual value. */
		static std::wstring GetWString(const HANDLE& hProcess, const void* memoryPtr);
		static std::wstring GetWString(const HANDLE& hProcess, const void* memoryPtr, size_t maxLength);
		static std::wstring GetWString(const HANDLE& hProcess, const uintptr_t& memoryAddress);
		static std::wstring GetWString(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t maxLength);

		static bool		   SetWString(const HANDLE& hProcess, const void* memoryPtr, const std::wstring& newValue);
		static bool		   SetWString(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::wstring& newValue);

		static bool		   PatchWString(const HANDLE& hProcess, const void* memoryPtr, const std::wstring& from, const std::wstring& to);
		static bool		   PatchWString(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static std::wstring IndirectGetWString(const HANDLE& hProcess, const void* memoryPtr);
		static std::wstring IndirectGetWString(const HANDLE& hProcess, const void* memoryPtr, size_t maxLength);
		static std::wstring IndirectGetWString(const HANDLE& hProcess, const uintptr_t& memoryAddress);
		static std::wstring IndirectGetWString(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t maxLength);

		static bool		    IndirectSetWString(const HANDLE& hProcess, const void* memoryPtr, const std::wstring& newValue);
		static bool		    IndirectSetWString(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::wstring& newValue);

		static bool		    IndirectPatchWString(const HANDLE& hProcess, const void* memoryPtr, const std::wstring& from, const std::wstring& to);
		static bool		    IndirectPatchWString(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to);




		/* For when 'memoryAddress' contains the actual value. */
		static std::vector<uint8_t> GetBytes(const HANDLE& hProcess, const void* memoryPtr, size_t byteCount);
		static std::vector<uint8_t> GetBytes(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t byteCount);

		static bool					SetBytes(const HANDLE& hProcess, const void* memoryPtr, const std::vector<uint8_t>& newBytes);
		static bool					SetBytes(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::vector<uint8_t>& newBytes);

		static bool					PatchBytes(const HANDLE& hProcess, const void* memoryPtr, const std::vector<uint8_t>& fromBytes, const std::vector<uint8_t>& toBytes);
		static bool					PatchBytes(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::vector<uint8_t>& fromBytes, const std::vector<uint8_t>& toBytes);


		/* For when 'memoryAddress' contains the address that leads to the value. */
		static std::vector<uint8_t> IndirectGetBytes(const HANDLE& hProcess, const void* memoryPtr, size_t byteCount);
		static std::vector<uint8_t> IndirectGetBytes(const HANDLE& hProcess, const uintptr_t& memoryAddress, size_t byteCount);

		static bool					IndirectSetBytes(const HANDLE& hProcess, const void* memoryPtr, const std::vector<uint8_t>& newBytes);
		static bool					IndirectSetBytes(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::vector<uint8_t>& newBytes);

		static bool					IndirectPatchBytes(const HANDLE& hProcess, const void* memoryPtr, const std::vector<uint8_t>& fromBytes, const std::vector<uint8_t>& toBytes);
		static bool					IndirectPatchBytes(const HANDLE& hProcess, const uintptr_t& memoryAddress, const std::vector<uint8_t>& fromBytes, const std::vector<uint8_t>& toBytes);
	};
};