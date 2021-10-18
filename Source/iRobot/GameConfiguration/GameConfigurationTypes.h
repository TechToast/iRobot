#pragma once

#include "CoreMinimal.h"
#include "GameConfigurationTypes.generated.h"


USTRUCT()
struct FGCOptional
{
	GENERATED_BODY()

public:

	bool IsSet() const { return bIsSet && !bInvalidated; }
	void Invalidate()  { bInvalidated = true; }

protected:
	bool bIsSet = false;
	bool bInvalidated = false;
};


USTRUCT()
struct FGCFloat : public FGCOptional
{
	GENERATED_BODY()

public:

	FGCFloat() {}
	FGCFloat(float InValue) { Value = InValue; }

	const float& GetValue() const { return Value; }

	/// Required for JSON Object Converter to work with this type
	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
	{
		if (FCString::IsNumeric(Buffer))
		{
			Value = FCString::Atof(Buffer);
			bIsSet = true;
			return true;
		}

		return false;
	}

private:
	float Value = 0;
};


USTRUCT()
struct FGCInt : public FGCOptional
{
	GENERATED_BODY()

public:

	FGCInt() {}
	FGCInt(int32 InValue) { Value = InValue; }

	const int32& GetValue() const { return Value; }

	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
	{
		if (FCString::IsNumeric(Buffer))
		{
			Value = FCString::Atoi(Buffer);
			bIsSet = true;
			return true;
		}

		return false;
	}

private:
	int32 Value = 0;
};


USTRUCT()
struct FGCBool : public FGCOptional
{
	GENERATED_BODY()

public:

	FGCBool() {}
	FGCBool(bool InValue) { Value = InValue; }

	const bool& GetValue() const { return Value; }

	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
	{
		if (FCString::Strlen(Buffer) > 0)
		{
			Value = FCString::ToBool(Buffer);
			bIsSet = true;
			return true;
		}

		return false;
	}

private:
	bool Value = false;
};


USTRUCT()
struct FGCString : public FGCOptional
{
	GENERATED_BODY()

public:

	FGCString() {}
	FGCString(FStringView InValue) { Value = InValue; }

	FStringView GetValue() const { return Value; }

	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
	{
		if (FCString::Strlen(Buffer) > 0)
		{
			Value = FString(Buffer);
			bIsSet = true;
			return true;
		}

		return false;
	}

private:
	FString Value;
};


USTRUCT()
struct FGCName : public FGCOptional
{
	GENERATED_BODY()

public:

	FGCName() {}
	FGCName(FStringView InValue) { Value = FName(InValue); }
	FGCName(FName InValue) { Value = InValue, bIsSet = true; }

	const FName& GetValue() const { return Value; }

	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
	{
		if (FCString::Strlen(Buffer) > 0)
		{
			Value = FName(Buffer);
			bIsSet = true;
			return true;
		}

		return false;
	}

	FORCEINLINE bool operator==(FGCName Other) const
	{
		return (IsSet() && Other.IsSet() && Value == Other.Value);
	}

private:
	FName Value = NAME_None;
};


FORCEINLINE uint32 GetTypeHash(FGCName Name)
{
	return GetTypeHash(Name.GetValue().GetComparisonIndex()) + Name.GetValue().GetNumber();
}


/// Struct serialiser specialisations
///
template<> struct TStructOpsTypeTraits<FGCFloat>  : public TStructOpsTypeTraitsBase2<FGCFloat>	{ enum { WithImportTextItem = true, }; };
template<> struct TStructOpsTypeTraits<FGCInt>    : public TStructOpsTypeTraitsBase2<FGCInt>	{ enum { WithImportTextItem = true, }; };
template<> struct TStructOpsTypeTraits<FGCBool>	  : public TStructOpsTypeTraitsBase2<FGCBool>	{ enum { WithImportTextItem = true, }; };
template<> struct TStructOpsTypeTraits<FGCString> : public TStructOpsTypeTraitsBase2<FGCString>	{ enum { WithImportTextItem = true, }; };
template<> struct TStructOpsTypeTraits<FGCName>   : public TStructOpsTypeTraitsBase2<FGCName>	{ enum { WithImportTextItem = true, }; };