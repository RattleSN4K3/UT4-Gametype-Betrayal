#include "UTBetrayal.h"
#include "UTBetrayalGameState.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalTeamInfoStub.h"

AUTBetrayalGameState::AUTBetrayalGameState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AUTBetrayalGameState::BeginPlay()
{
	Super::BeginPlay();

	// create temp team to be used to use team materials for overriden materials in applying character data for Betrayal character
	TempTeamForHook = GetWorld()->SpawnActor<AUTTeamInfo>(AUTBetrayalTeamInfoStub::StaticClass());
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

bool AUTBetrayalGameState::HookTeam(AUTCharacter* Char, AUTPlayerState* PS)
{
	if (Char == NULL || PS == NULL)
		return false;

	if (PS->Team == NULL && TempTeamForHook != NULL)
	{
		PS->Team = TempTeamForHook;
	}

	return PS->Team != NULL;
}

void AUTBetrayalGameState::UnhookTeam(AUTCharacter* Char, AUTPlayerState* PS)
{
	if (Char != NULL && PS != NULL)
	{
		PS->Team = NULL;
	}
}
