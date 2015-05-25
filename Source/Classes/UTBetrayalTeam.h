#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalTeam.generated.h"

class AUTBetrayalPlayerState;

UCLASS()
class AUTBetrayalTeam : public AActor
{
	GENERATED_UCLASS_BODY()

	static const int MAX_TEAMMATES = 3;

	UPROPERTY(Replicated)
	AUTBetrayalPlayerState* Teammates[MAX_TEAMMATES];

	//Value of the shared pot
	UPROPERTY(Replicated)
	int32 TeamPot;

	virtual bool AddTeammate(AUTBetrayalPlayerState* NewTeammate, int32 MaxTeamSize);
	virtual int32 LoseTeammate(AUTBetrayalPlayerState* OldTeammate);

};