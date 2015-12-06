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

	/** Value of the shared pot */
	UPROPERTY(Replicated)
	int32 TeamPot;

	/**
	 * Adds the given player to the team
	 * @param NewTeammate the player to add
	 * @param MaxTeamSize the new maximum team size
	 */
	virtual bool AddTeammate(AUTBetrayalPlayerState* NewTeammate, int32 MaxTeamSize);

	/** Removes the given player from te team */
	virtual int32 LoseTeammate(AUTBetrayalPlayerState* OldTeammate);

	/** Removes any player from the team */
	virtual void DisbandTeam();

};