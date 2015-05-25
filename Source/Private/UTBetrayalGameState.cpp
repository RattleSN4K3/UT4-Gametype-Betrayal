#include "UTBetrayal.h"
#include "UTBetrayalGameState.h"
#include "UTBetrayalPlayerState.h"

AUTBetrayalGameState::AUTBetrayalGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool AUTBetrayalGameState::OnSameTeam(const AActor* Actor1, const AActor* Actor2)
{
	const AUTBetrayalPlayerState* Actor1PRI = GetBetrayalPRIFor(Actor1);
	const AUTBetrayalPlayerState* Actor2PRI = GetBetrayalPRIFor(Actor2);

	if (Actor1PRI != NULL && Actor2PRI != NULL)
	{
		return Actor1PRI != Actor2PRI && Actor1PRI->CurrentTeam != NULL && Actor1PRI->CurrentTeam == Actor2PRI->CurrentTeam;
	}

	// FIXME: Shoud return false?
	return Super::OnSameTeam(Actor1, Actor2);
}

AUTBetrayalPlayerState* AUTBetrayalGameState::GetBetrayalPRIFor(const AActor* A) const
{
	AUTBetrayalPlayerState* PRI = NULL;
	if (Cast<AUTBetrayalPlayerState>(A) != NULL)
	{
		PRI = (AUTBetrayalPlayerState*)A;
	}
	if (PRI == NULL && A != NULL)
	{
		if (Cast<AController>(A) != NULL)
		{
			PRI = Cast<AUTBetrayalPlayerState>(Cast<AController>(A)->PlayerState);
		}
		if (PRI == NULL && Cast<APawn>(A) != NULL)
		{
			PRI = Cast<AUTBetrayalPlayerState>(Cast<APawn>(A)->PlayerState);
		}
	}

	return PRI;
}
