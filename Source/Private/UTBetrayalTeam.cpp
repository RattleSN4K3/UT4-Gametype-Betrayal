#include "UTBetrayal.h"
#include "UTBetrayalGameMode.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalTeam.h"

AUTBetrayalTeam::AUTBetrayalTeam(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bAlwaysRelevant = true;
	bReplicateMovement = false;

	NetUpdateFrequency = 2.0f;

	// FIX: Not sure about this. It is set in PlayerState as well
	// TODO: Test Seamless travel
	bNetLoadOnClient = false;
}

void AUTBetrayalTeam::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUTBetrayalTeam, TeamPot);
	DOREPLIFETIME(AUTBetrayalTeam, Teammates);
}

bool AUTBetrayalTeam::AddTeammate(AUTBetrayalPlayerState* NewTeammate, int32 MaxTeamSize)
{
	AUTBetrayalGameMode* Game = GetWorld()->GetAuthGameMode<AUTBetrayalGameMode>();
	if (Game != NULL && TeamPot > Game->RogueValue / 2)
	{
		// don't add to teams that already have significant pots
		return false;
	}

	int32 NumTeammates = 0;

	//Count current team size
	for (int32 i = 0; i<ARRAY_COUNT(Teammates); i++)
	{
		if (Teammates[i] != NULL)
		{
			NumTeammates++;
		}
	}

	MaxTeamSize = FMath::Min(MaxTeamSize, MAX_TEAMMATES);
	if (NumTeammates >= MaxTeamSize)
	{
		return false;
	}

	for (int32 i = 0; i<ARRAY_COUNT(Teammates); i++)
	{
		if (Teammates[i] == NewTeammate)
		{
			// already added
			return true;
		}

		if (Teammates[i] == NULL || Teammates[i]->IsPendingKillPending())
		{
			NewTeammate->CurrentTeam = this;
			Teammates[i] = NewTeammate;
			return true;
		}
	}

	return false;
}

int32 AUTBetrayalTeam::LoseTeammate(AUTBetrayalPlayerState* OldTeammate)
{
	int32 NumTeammates = 0;

	OldTeammate->CurrentTeam = NULL;

	// TODO: Add bot support
	//AUTBot* B = Cast<AUTBot>(OldTeammate->GetOwner());
	//if (B != NULL)
	//{
	//	B->bBetrayTeam = false;
	//}

	ForceNetUpdate();
	for (int32 i = 0; i< ARRAY_COUNT(Teammates); i++)
	{
		if (Teammates[i] == NULL || Teammates[i] == OldTeammate || Teammates[i]->IsPendingKillPending())
		{
			Teammates[i] = NULL;
		}
		else
		{
			NumTeammates++;
		}
	}

	//Returns number of teammates left after removing a player
	return NumTeammates;
}
