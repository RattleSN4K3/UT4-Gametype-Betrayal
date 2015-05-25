#pragma once

//#include "Core.h"
//#include "Engine.h"

#include "UnrealTournament.h"
#include "Net/UnrealNetwork.h"

#include "UTDMGameMode.h"
#include "UTAnnouncer.h"

#include "UTBetrayal.generated.h"

UCLASS(CustomConstructor)
class UUTBetrayal : public UObject
{
	GENERATED_UCLASS_BODY()

	UUTBetrayal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	{}

};