#include "UTBetrayal.h"
#include "UTBetrayalTeam.h"
#include "UTBetrayalGameMode.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalBot.h"

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

	//Count current team size
	int32 NumTeammates = 0;
	for (int32 i = 0; i<ARRAY_COUNT(Teammates); i++)
	{
		if (Teammates[i] != NULL)
		{
			NumTeammates++;
		}
	}

	// abort adding player to the team if team already has the maximum amount of players
	MaxTeamSize = FMath::Min(MaxTeamSize, MAX_TEAMMATES);
	if (NumTeammates >= MaxTeamSize)
	{
		return false;
	}

	for (int32 i = 0; i<ARRAY_COUNT(Teammates); i++)
	{
		if (Teammates[i] == NewTeammate)
		{
			// update team, just in case
			NewTeammate->UpdateTeam(this);

			// already added
			return true;
		}

		if (Teammates[i] == NULL || Teammates[i]->IsPendingKillPending())
		{
			NewTeammate->CurrentTeam = this;
			Teammates[i] = NewTeammate;

			// new team, call notify functions
			NewTeammate->UpdateTeam(this);

			return true;
		}
	}

	return false;
}

int32 AUTBetrayalTeam::LoseTeammate(AUTBetrayalPlayerState* OldTeammate)
{
	OldTeammate->CurrentTeam = NULL;
	OldTeammate->UpdateTeam(NULL);

	// TODO: store flag else where FIXME: do not use subclassed UTBot
	if (AUTBetrayalBot* B = Cast<AUTBetrayalBot>(OldTeammate->GetOwner()))
	{
		B->bBetrayTeam = false;
	}

	ForceNetUpdate();

	int32 NumTeammates = 0;
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

void AUTBetrayalTeam::DisperseTeam()
{
	for (int32 i = 0; i < ARRAY_COUNT(Teammates); i++)
	{
		if (Teammates[i] != NULL)
		{
			LoseTeammate(Teammates[i]);
		}
	}
}
