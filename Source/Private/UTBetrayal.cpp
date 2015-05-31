#include "UTBetrayal.h"

DEFINE_LOG_CATEGORY(Betrayal);

// COPIED FROM UObjectGlobals.cpp
// Binary initialize object properties to zero or defaults.
void UUTBetrayal::InitProperties(UObject* Obj, UClass* DefaultsClass, UObject* DefaultData, bool bCopyTransientsFromClassDefaults)
{
	//SCOPE_CYCLE_COUNTER(STAT_InitProperties);

	check(DefaultsClass && Obj);

	UClass* Class = Obj->GetClass();
	// bool to indicate that we need to initialize any non-native properties (native ones were done when the native constructor was called by the code that created and passed in a FObjectInitializer object)
	bool bNeedInitialize = !Class->HasAnyClassFlags(CLASS_Native | CLASS_Intrinsic);

	if (Obj->HasAnyFlags(RF_NeedLoad))
	{
		bCopyTransientsFromClassDefaults = false;
	}

	if (!bNeedInitialize && !bCopyTransientsFromClassDefaults && DefaultsClass == Class)
	{
		// This is just a fast path for the below in the common case that we are not doing a duplicate or initializing a CDO and this is all native.
		// We only do it if the DefaultData object is NOT a CDO of the object that's being initialized. CDO data is already initialized in the
		// object's constructor.
		if (DefaultData)
		{
			if (Class->GetDefaultObject(false) != DefaultData)
			{
				for (UProperty* P = Class->PropertyLink; P; P = P->PropertyLinkNext)
				{
					P->CopyCompleteValue_InContainer(Obj, DefaultData);
				}
			}
			else
			{
				// Copy all properties that require additional initialization (CPF_Config, CPF_Localized).
				for (UProperty* P = Class->PostConstructLink; P; P = P->PostConstructLinkNext)
				{
					P->CopyCompleteValue_InContainer(Obj, DefaultData);
				}
			}
		}
	}
	else
	{
		UObject* ClassDefaults = bCopyTransientsFromClassDefaults ? DefaultsClass->GetDefaultObject() : NULL;
		for (UProperty* P = Class->PropertyLink; P; P = P->PropertyLinkNext)
		{
			if (bNeedInitialize)
			{
				bNeedInitialize = InitNonNativeProperty(P, Obj);
			}

			if (bCopyTransientsFromClassDefaults && P->HasAnyPropertyFlags(CPF_Transient | CPF_DuplicateTransient | CPF_NonPIEDuplicateTransient))
			{
				// This is a duplicate. The value for all transient or non-duplicatable properties should be copied
				// from the source class's defaults.
				P->CopyCompleteValue_InContainer(Obj, ClassDefaults);
			}
			else if (P->IsInContainer(DefaultsClass))
			{
				P->CopyCompleteValue_InContainer(Obj, DefaultData);
			}
		}
	}
}

// COPIED FROM BlueprintSupport.cpp
/**
* Initializes a non-native property, according to the initialization rules. If the property is non-native
* and does not have a zero constructor, it is initialized with the default value.
* @param	Property			Property to be initialized
* @param	Data				Default data
* @return	Returns true if that property was a non-native one, otherwise false
*/
bool UUTBetrayal::InitNonNativeProperty(UProperty* Property, UObject* Data)
{
	if (!Property->GetOwnerClass()->HasAnyClassFlags(CLASS_Native | CLASS_Intrinsic)) // if this property belongs to a native class, it was already initialized by the class constructor
	{
		if (!Property->HasAnyPropertyFlags(CPF_ZeroConstructor)) // this stuff is already zero
		{
			Property->InitializeValue_InContainer(Data);
		}
		return true;
	}
	else
	{
		// we have reached a native base class, none of the rest of the properties will need initialization
		return false;
	}
}
