#include "UTBetrayal.h"
#include "UTBetrayalBot.h"
#include "UTBetrayalGameState.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalTeam.h"
#include "UTBot.h"

AUTBetrayalBot::AUTBetrayalBot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AUTBetrayalBot::PickNewEnemy()
{
	if (bBetrayTeam && GetPawn() != NULL)
	{
		if (AUTBetrayalGameState* GS = GetWorld()->GetGameState<AUTBetrayalGameState>())
		{
			AUTBetrayalPlayerState* BPRI = Cast<AUTBetrayalPlayerState>(PlayerState);
			if (BPRI != NULL && BPRI->CurrentTeam != NULL)
			{
				// as we call UpdateEnemyInfo currently, we need to skip the team check
				GS->bSkipCheck = true;
				bPickProcessing = true;

				for (int32 i = 0; i < ARRAY_COUNT(BPRI->CurrentTeam->Teammates); i++)
				{
					APawn* Enemy = NULL;
					if (BPRI != BPRI->CurrentTeam->Teammates[i] && GetPawnByPRI(BPRI->CurrentTeam->Teammates[i], Enemy))
					{
						// condtionally add team member to enemy list
						UpdateEnemyInfo(Enemy, EUT_Seen);
					}
				}

				// revert
				GS->bSkipCheck = false;
				bPickProcessing = false;
			}
		}
	}

	// Pick enemy based on the current (and possibly modified) list of enemies
	Super::PickNewEnemy();
}

bool AUTBetrayalBot::IsImportantEnemyUpdate(APawn* TestEnemy, EAIEnemyUpdateType UpdateType)
{
	// prevent recursion in UpdateEnemyInfo query for importance and calling PickNewEnemy again
	if (bPickProcessing)
	{
		return false;
	}

	bool bIsImportant = Super::IsImportantEnemyUpdate(TestEnemy, UpdateType);

	if (bBetrayTeam && !bIsImportant && TestEnemy != NULL)
	{
		if (AUTBetrayalGameState* GS = GetWorld()->GetGameState<AUTBetrayalGameState>())
		{
			if (GS->OnSameTeam(this, TestEnemy))
			{
				bIsImportant = true;
			}
		}
	}

	return bIsImportant;
}

bool AUTBetrayalBot::GetPawnByPRI(AUTPlayerState* PRI, APawn*& P)
{
	P = NULL;
	if (PRI != NULL)
	{
		if (AController* C = Cast<AController>(PRI->GetOwner()))
		{
			P = C->GetPawn();
			return P != NULL;
		}
	}

	return false;
}
