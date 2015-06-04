#include "UTBetrayal.h"
#include "UTBetrayalWeapon.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalGameMode.h"
#include "UTBetrayalBot.h"
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
			else if (AUTPlayerController* PC = Cast<AUTPlayerController>(Instigator->Controller))
			{
				// bots don't like being shot at by teammates
				
				if (PC->LastShotTargetGuess != NULL && PC->LastShotTargetGuess->Controller != NULL)
				{
					if (AUTBetrayalBot* Bot = Cast<AUTBetrayalBot>(PC->LastShotTargetGuess->Controller))
					{
						AUTBetrayalPlayerState* BotPRI = Cast<AUTBetrayalPlayerState>(Bot->PlayerState);

						AUTGameState* GameState = GetWorld()->GetGameState<AUTGameState>();
						AUTGameMode* Game = GetWorld()->GetAuthGameMode<AUTGameMode>();

						if (BotPRI != NULL && GameState != NULL && Game != NULL && InstigatorPRI->CurrentTeam != NULL && GameState->OnSameTeam(Instigator, Bot) && 
							InstigatorPRI->CurrentTeam->TeamPot >= FMath::Min<int32>(6, Game->GoalScore - FMath::Max<int32>(InstigatorPRI->Score, BotPRI->Score)))
						{
							UE_LOG(Betrayal, Log, TEXT("%s betray shooter"), *InstigatorPRI->PlayerName);
							Bot->bBetrayTeam = true;
						}
					}
				}
			}
		}
	}

	Super::FireInstantHit(bDealDamage, OutHit);
}

bool AUTBetrayalWeapon::CanAttack_Implementation(AActor* Target, const FVector& TargetLoc, bool bDirectOnly, bool bPreferCurrentMode, UPARAM(ref) uint8& BestFireMode, UPARAM(ref) FVector& OptimalTargetLoc)
{
	if (Super::CanAttack_Implementation(Target, TargetLoc, bDirectOnly, bPreferCurrentMode, BestFireMode, OptimalTargetLoc))
	{
		AUTGameState* GameState = GetWorld()->GetGameState<AUTGameState>();
		if (GameState != NULL)
		{
			BestFireMode = GameState->OnSameTeam(Target, UTOwner) ? 1 : 0;
		}
		return true;
	}

	return false;
}
