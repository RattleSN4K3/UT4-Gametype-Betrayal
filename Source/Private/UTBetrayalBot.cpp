#include "UTBetrayal.h"
#include "UTBetrayalBot.h"
#include "UTBetrayalGameState.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalTeam.h"
#include "UTBot.h"

// TODO: rate enemy if bBetrayteam and teammate

AUTBetrayalBot::AUTBetrayalBot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool AUTBetrayalBot::IsTeammate(AActor* TestActor)
{
	// shoot at everyone if on betray-mode
	if (bBetrayTeam)
	{
		return false;
	}

	return Super::IsTeammate(TestActor);
}
