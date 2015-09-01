#include "UTBetrayal.h"
#include "UTBetrayalBot.h"
#include "UTBetrayalGameState.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalTeam.h"
#include "UTBot.h"

AUTBetrayalBot::AUTBetrayalBot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BetrayAggressiveness = 1.0;
}

bool AUTBetrayalBot::IsTeammate(AActor* TestActor)
{
	// shoot at everyone if on betray-mode
	if (bBetrayTeam)
	{
		return false;
	}

	// fallback solution if bots are in team and only 2 players in the game, shoot at anyone
	AUTGameMode* Game = GetWorld()->GetAuthGameMode<AUTGameMode>();
	if (Game != NULL && Game->NumPlayers + Game->NumBots < 3)
	{
		return false;
	}

	return Super::IsTeammate(TestActor);
}

float AUTBetrayalBot::RateEnemy(const FBotEnemyInfo& EnemyInfo)
{
	// retrieve the base rating
	float ThreatValue = Super::RateEnemy(EnemyInfo);

	AUTCharacter* P = EnemyInfo.GetUTChar();
	if (ThreatValue > 0.0 && P != NULL)
	{
		AUTBetrayalPlayerState* EnemyPRI = Cast<AUTBetrayalPlayerState>(P->PlayerState);
		AUTBetrayalPlayerState* PRI = Cast<AUTBetrayalPlayerState>(PlayerState);
		if (PRI != NULL && EnemyPRI != NULL)
		{
			if (PRI->Betrayer == EnemyPRI)
			{
				// payback kill
				ThreatValue += 2.0;
			}
			else if ((EnemyPRI->bIsRogue && EnemyPRI->BetrayedTeam != NULL && EnemyPRI->BetrayedTeam == PRI->CurrentTeam) ||
				(PRI->bIsRogue && PRI->BetrayedTeam != NULL && PRI->BetrayedTeam == EnemyPRI->CurrentTeam))
			{
				// retribution kill
				ThreatValue += 1.2;
			}
			else if (bBetrayTeam && PRI->CurrentTeam != NULL && PRI->CurrentTeam == EnemyPRI->CurrentTeam)
			{
				// maybe betray team mates
				ThreatValue += 0.6 * BetrayAggressiveness;
			}
		}
		if (P->IsSpawnProtected())
		{
			// slightly ignore spawned players due to instant weapon
			ThreatValue -= 0.4;
		}
	}

	UE_LOG(Betrayal, Verbose, TEXT("Bot::RateEnemy %s: %f"), *EnemyInfo.GetUTChar()->GetName(), ThreatValue);
	return ThreatValue;
}
