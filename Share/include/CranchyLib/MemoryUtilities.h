#pragma once

#include <windows.h>
#include <string>
#include <vector>





class MemoryUtilities
{
public:
	static bool IsValidPtr(void* memoryPtr);
	static bool IsValidAddress(const uintptr_t& memoryAddress);

	static void* PtrAddOffset(void* memoryPtr, size_t offset);
	static void* PtrAddOffset(const uintptr_t& memoryAddress, size_t offset);

	static uintptr_t AddressAddOffset(void* memoryPtr, size_t offset);
	static uintptr_t AddressAddOffset(const uintptr_t& memoryAddress, size_t offset);

	static void*	 PtrFollowPointerChain(void* memoryPtr, const std::vector<uintptr_t>& memoryOffsets);
	static uintptr_t AddressFollowPointerChain(const uintptr_t& memoryAddress, const std::vector<uintptr_t>& memoryOffsets);




	/* For when 'memoryAddress' contains the actual value */
	static bool GetBool(void* memoryPtr);
	static bool GetBool(const uintptr_t& memoryAddress);

	static bool SetBool(void* memoryPtr, bool newValue);
	static bool SetBool(const uintptr_t& memoryAddress, bool newValue);

	static bool PatchBool(void* memoryPtr, bool from, bool to);
	static bool PatchBool(const uintptr_t& memoryAddress, bool from, bool to);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static bool IndirectGetBool(void* memoryPtr);
	static bool IndirectGetBool(const uintptr_t& memoryAddress);

	static bool IndirectSetBool(void* memoryPtr, bool newValue);
	static bool IndirectSetBool(const uintptr_t& memoryAddress, bool newValue);

	static bool IndirectPatchBool(void* memoryPtr, bool from, bool to);
	static bool IndirectPatchBool(const uintptr_t& memoryAddress, bool from, bool to);




	/* For when 'memoryAddress' contains the actual value */
	static int8_t GetInt8(void* memoryPtr);
	static int8_t GetInt8(const uintptr_t& memoryAddress);

	static bool   SetInt8(void* memoryPtr, int8_t newValue);
	static bool   SetInt8(const uintptr_t& memoryAddress, int8_t newValue);

	static bool   PatchInt8(void* memoryPtr, int8_t from, int8_t to);
	static bool   PatchInt8(const uintptr_t& memoryAddress, int8_t from, int8_t to);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static int8_t IndirectGetInt8(void* memoryPtr);
	static int8_t IndirectGetInt8(const uintptr_t& memoryAddress);

	static bool   IndirectSetInt8(void* memoryPtr, int8_t newValue);
	static bool   IndirectSetInt8(const uintptr_t& memoryAddress, int8_t newValue);

	static bool   IndirectPatchInt8(void* memoryPtr, int8_t from, int8_t to);
	static bool   IndirectPatchInt8(const uintptr_t& memoryAddress, int8_t from, int8_t to);




	/* For when 'memoryAddress' contains the actual value */
	static int16_t GetInt16(void* memoryPtr);
	static int16_t GetInt16(const uintptr_t& memoryAddress);

	static bool    SetInt16(void* memoryPtr, int16_t newValue);
	static bool    SetInt16(const uintptr_t& memoryAddress, int16_t newValue);

	static bool    PatchInt16(void* memoryPtr, int16_t from, int16_t to);
	static bool    PatchInt16(const uintptr_t& memoryAddress, int16_t from, int16_t to);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static int16_t IndirectGetInt16(void* memoryPtr);
	static int16_t IndirectGetInt16(const uintptr_t& memoryAddress);

	static bool    IndirectSetInt16(void* memoryPtr, int16_t newValue);
	static bool    IndirectSetInt16(const uintptr_t& memoryAddress, int16_t newValue);

	static bool    IndirectPatchInt16(void* memoryPtr, int16_t from, int16_t to);
	static bool    IndirectPatchInt16(const uintptr_t& memoryAddress, int16_t from, int16_t to);




	/* For when 'memoryAddress' contains the actual value */
	static int32_t GetInt32(void* memoryPtr);
	static int32_t GetInt32(const uintptr_t& memoryAddress);

	static bool    SetInt32(void* memoryPtr, int32_t newValue);
	static bool    SetInt32(const uintptr_t& memoryAddress, int32_t newValue);

	static bool    PatchInt32(void* memoryPtr, int32_t from, int32_t to);
	static bool    PatchInt32(const uintptr_t& memoryAddress, int32_t from, int32_t to);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static int32_t IndirectGetInt32(void* memoryPtr);
	static int32_t IndirectGetInt32(const uintptr_t& memoryAddress);

	static bool	   IndirectSetInt32(void* memoryPtr, int32_t newValue);
	static bool	   IndirectSetInt32(const uintptr_t& memoryAddress, int32_t newValue);

	static bool    IndirectPatchInt32(void* memoryPtr, int32_t from, int32_t to);
	static bool    IndirectPatchInt32(const uintptr_t& memoryAddress, int32_t from, int32_t to);




	/* For when 'memoryAddress' contains the actual value */
	static int64_t GetInt64(void* memoryPtr);
	static int64_t GetInt64(const uintptr_t& memoryAddress);

	static bool    SetInt64(void* memoryPtr, int64_t newValue);
	static bool    SetInt64(const uintptr_t& memoryAddress, int64_t newValue);

	static bool    PatchInt64(void* memoryPtr, int64_t from, int64_t to);
	static bool    PatchInt64(const uintptr_t& memoryAddress, int64_t from, int64_t to);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static int64_t IndirectGetInt64(void* memoryPtr);
	static int64_t IndirectGetInt64(const uintptr_t& memoryAddress);

	static bool    IndirectSetInt64(void* memoryPtr, int64_t newValue);
	static bool    IndirectSetInt64(const uintptr_t& memoryAddress, int64_t newValue);

	static bool    IndirectPatchInt64(void* memoryPtr, int64_t from, int64_t to);
	static bool    IndirectPatchInt64(const uintptr_t& memoryAddress, int64_t from, int64_t to);




	/* For when 'memoryAddress' contains the actual value */
	static float GetFloat(void* memoryPtr);
	static float GetFloat(const uintptr_t& memoryAddress);

	static bool  SetFloat(void* memoryPtr, float newValue);
	static bool  SetFloat(const uintptr_t& memoryAddress, float newValue);

	static bool  PatchFloat(void* memoryPtr, float from, float to);
	static bool  PatchFloat(const uintptr_t& memoryAddress, float from, float to);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static float IndirectGetFloat(void* memoryPtr);
	static float IndirectGetFloat(const uintptr_t& memoryAddress);

	static bool  IndirectSetFloat(void* memoryPtr, float newValue);
	static bool  IndirectSetFloat(const uintptr_t& memoryAddress, float newValue);

	static bool  IndirectPatchFloat(void* memoryPtr, float from, float to);
	static bool  IndirectPatchFloat(const uintptr_t& memoryAddress, float from, float to);




	/* For when 'memoryAddress' contains the actual value */
	static double GetDouble(void* memoryPtr);
	static double GetDouble(const uintptr_t& memoryAddress);

	static bool   SetDouble(void* memoryPtr, double newValue);
	static bool   SetDouble(const uintptr_t& memoryAddress, double newValue);

	static bool   PatchDouble(void* memoryPtr, double from, double to);
	static bool   PatchDouble(const uintptr_t& memoryAddress, double from, double to);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static double IndirectGetDouble(void* memoryPtr);
	static double IndirectGetDouble(const uintptr_t& memoryAddress);

	static bool   IndirectSetDouble(void* memoryPtr, double newValue);
	static bool   IndirectSetDouble(const uintptr_t& memoryAddress, double newValue);

	static bool   IndirectPatchDouble(void* memoryPtr, double from, double to);
	static bool   IndirectPatchDouble(const uintptr_t& memoryAddress, double from, double to);




	/* For when 'memoryAddress' contains the actual value */
	static std::string GetString(void* memoryPtr);
	static std::string GetString(void* memoryPtr, size_t maxLength);
	static std::string GetString(const uintptr_t& memoryAddress);
	static std::string GetString(const uintptr_t& memoryAddress, size_t maxLength);

	static bool		   SetString(void* memoryPtr, const std::string& newValue);
	static bool		   SetString(const uintptr_t& memoryAddress, const std::string& newValue);

	static bool		   PatchString(void* memoryPtr, const std::string& from, const std::string& to);
	static bool		   PatchString(const uintptr_t& memoryAddress, const std::string& from, const std::string& to);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static std::string IndirectGetString(void* memoryPtr);
	static std::string IndirectGetString(void* memoryPtr, size_t maxLength);
	static std::string IndirectGetString(const uintptr_t& memoryAddress);
	static std::string IndirectGetString(const uintptr_t& memoryAddress, size_t maxLength);

	static bool		   IndirectSetString(void* memoryPtr, const std::string& newValue);
	static bool		   IndirectSetString(const uintptr_t& memoryAddress, const std::string& newValue);

	static bool		   IndirectPatchString(void* memoryPtr, const std::string& from, const std::string& to);
	static bool		   IndirectPatchString(const uintptr_t& memoryAddress, const std::string& from, const std::string& to);


	/* For when 'memoryAddress' contains the actual value */
	static std::wstring GetWString(void* memoryPtr);
	static std::wstring GetWString(void* memoryPtr, size_t maxLength);
	static std::wstring GetWString(const uintptr_t& memoryAddress);
	static std::wstring GetWString(const uintptr_t& memoryAddress, size_t maxLength);

	static bool			SetWString(void* memoryPtr, const std::wstring& newValue);
	static bool			SetWString(const uintptr_t& memoryAddress, const std::wstring& newValue);

	static bool			PatchWString(void* memoryPtr, const std::wstring& from, const std::wstring& to);
	static bool			PatchWString(const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static std::wstring IndirectGetWString(void* memoryPtr);
	static std::wstring IndirectGetWString(void* memoryPtr, size_t maxLength);
	static std::wstring IndirectGetWString(const uintptr_t& memoryAddress);
	static std::wstring IndirectGetWString(const uintptr_t& memoryAddress, size_t maxLength);

	static bool			IndirectSetWString(void* memoryPtr, const std::wstring& newValue);
	static bool			IndirectSetWString(const uintptr_t& memoryAddress, const std::wstring& newValue);

	static bool			IndirectPatchWString(void* memoryPtr, const std::wstring& from, const std::wstring& to);
	static bool			IndirectPatchWString(const uintptr_t& memoryAddress, const std::wstring& from, const std::wstring& to);




	/* For when 'memoryAddress' contains the actual value */
	static std::vector<uint8_t> GetBytes(void* memoryPtr, size_t byteCount);
	static std::vector<uint8_t> GetBytes(const uintptr_t& memoryAddress, size_t byteCount);

	static bool					SetBytes(void* memoryPtr, const uint8_t* newBytes, size_t byteCount);
	static bool					SetBytes(const uintptr_t& memoryAddress, const uint8_t* newBytes, size_t byteCount);

	static bool					PatchBytes(void* memoryPtr, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount);
	static bool					PatchBytes(const uintptr_t& memoryAddress, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount);


	/* For when 'memoryAddress' contains the address that leads to the value */
	static std::vector<uint8_t> IndirectGetBytes(void* memoryPtr, size_t byteCount);
	static std::vector<uint8_t> IndirectGetBytes(const uintptr_t& memoryAddress, size_t byteCount);

	static bool					IndirectSetBytes(void* memoryPtr, const uint8_t* newBytes, size_t byteCount);
	static bool					IndirectSetBytes(const uintptr_t& memoryAddress, const uint8_t* newBytes, size_t byteCount);

	static bool					IndirectPatchBytes(void* memoryPtr, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount);
	static bool					IndirectPatchBytes(const uintptr_t& memoryAddress, const uint8_t* fromBytes, const uint8_t* toBytes, size_t byteCount);
};