#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalTeamInfoStub.generated.h"

UCLASS(CustomConstructor)
class AUTBetrayalTeamInfoStub : public AUTTeamInfo
{
	GENERATED_UCLASS_BODY()

	AUTBetrayalTeamInfoStub(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	{
		SetReplicates(false);
	}

};