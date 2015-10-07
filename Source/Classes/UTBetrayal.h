#pragma once

//#include "Core.h"
//#include "Engine.h"

#include "UnrealTournament.h"
#include "Net/UnrealNetwork.h"

#include "UTDMGameMode.h"
#include "UTAnnouncer.h"

#include "UTBetrayal.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(Betrayal, Log, All);

#define BETRAYAL_DEBUG 0

UCLASS(CustomConstructor)
class UUTBetrayal : public UObject
{
	GENERATED_UCLASS_BODY()

	UUTBetrayal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	{}

	/**
	* Binary initialize object properties to zero or defaults.
	*
	* @param	Obj					object to initialize data for
	* @param	DefaultsClass		the class to use for initializing the data
	* @param	DefaultData			the buffer containing the source data for the initialization
	* @param	bCopyTransientsFromClassDefaults if true, copy the transients from the DefaultsClass defaults, otherwise copy the transients from DefaultData
	*/
	static void InitProperties(UObject* Obj, UClass* DefaultsClass, UObject* DefaultData, bool bCopyTransientsFromClassDefaults);

	/**
	* Initializes a non-native property, according to the initialization rules. If the property is non-native
	* and does not have a zero contructor, it is inialized with the default value.
	* @param	Property			Property to be initialized
	* @param	Data				Default data
	* @return	Returns true if that property was a non-native one, otherwise false
	*/
	static bool InitNonNativeProperty(UProperty* Property, UObject* Data);

};