#include "UTBetrayal.h"
#include "UTBetrayalWeapon.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalGameMode.h"
#include "StatNames.h"

AUTBetrayalWeapon::AUTBetrayalWeapon(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	KillStatsName = NAME_InstagibKills;
	DeathStatsName = NAME_InstagibDeaths;
}

void AUTBetrayalWeapon::FireInstantHit(bool bDealDamage, FHitResult* OutHit)
{
	checkSlow(InstantHitInfo.IsValidIndex(CurrentFireMode));

	const FVector SpawnLocation = GetFireStartLoc();
	const FRotator SpawnRotation = GetAdjustedAim(SpawnLocation);
	const FVector FireDir = SpawnRotation.Vector();
	const FVector EndTrace = SpawnLocation + FireDir * InstantHitInfo[CurrentFireMode].TraceRange;

	FHitResult Hit;
	AUTPlayerController* UTPC = UTOwner ? Cast<AUTPlayerController>(UTOwner->Controller) : NULL;
	float PredictionTime = UTPC ? UTPC->GetPredictionTime() : 0.f;
	HitScanTrace(SpawnLocation, EndTrace, Hit, PredictionTime);

	if (bDealDamage && Role == ROLE_Authority && Instigator != NULL)
	{
		AUTBetrayalPlayerState* InstigatorPRI = Cast<AUTBetrayalPlayerState>(Instigator->PlayerState);
		if (InstigatorPRI != NULL)
		{
			APawn* HitPawn = Cast<APawn>(Hit.Actor.Get());
			if (HitPawn != NULL)
			{
				AUTBetrayalPlayerState* HitPRI = Cast<AUTBetrayalPlayerState>(HitPawn->PlayerState);
				if (HitPRI != NULL)
				{
					AUTGameState* GameState = GetWorld()->GetGameState<AUTGameState>();
					if (GameState != NULL && GameState->OnSameTeam(InstigatorPRI, HitPRI))
					{
						if (CurrentFireMode == 1)
						{
							AUTBetrayalGameMode* Game = GetWorld()->GetAuthGameMode<AUTBetrayalGameMode>();
							if (Game != NULL)
							{
								Game->ShotTeammate(InstigatorPRI, HitPRI, Instigator, HitPawn);
							}

							CurrentFireMode = 0; // TODO: Check if pending fire needs to be cleared and re-set
							Super::FireInstantHit(bDealDamage, OutHit);
						}
					}
					else if (CurrentFireMode == 0)
					{
						Super::FireInstantHit(bDealDamage, OutHit);
					}
				}
				return;
			}
			else
			{
				// TODO: add bot support
				/*
				// bots don't like being shot at by teammates
				AUTGameState* GameState = GetWorld()->GetGameState<AUTGameState>();
				AUTGameMode* Game = GetWorld()->GetAuthGameMode<AUTGameMode>();
				if ((Cast<APlayerController>(Instigator->Controller) != NULL)
					&& (Instigator->Controller->ShotTarget != NULL)
					&& (Cast<AUTBot>(Instigator->Controller->ShotTarget->Controller) != NULL)
					&& (GameState != NULL && GameState->OnSameTeam(Instigator, Instigator->Controller->ShotTarget))
					&& (Game != NULL && Cast<AUTBetrayalPlayerState>(Instigator->PlayerState)->CurrentTeam->TeamPot >= FMath::Min(6, Game->GoalScore - FMath::Max(Instigator->PlayerState->Score, Instigator->Controller->ShotTarget->PlayerState->Score))))
				{
					//UE_LOG(UT, Verbose, TEXT("%s betray shooter"), *Instigator->Controller->ShotTarget->Controller->PlayerState->PlayerName);
					Cast<AUTBot>(Instigator->Controller->ShotTarget->Controller).bBetrayTeam = true;
				}
				*/
			}
		}
	}

	Super::FireInstantHit(bDealDamage, OutHit);
}
