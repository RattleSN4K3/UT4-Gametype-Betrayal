#include "UTBetrayal.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalMessage.h"

#include "UTMutator_WeaponArena.h"
#include "UTMutator_WeaponReplacement.h"

#include "UTBetrayalGameMode.h"

AUTBetrayalGameMode::AUTBetrayalGameMode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("UTGameMode", "BET", "Betrayal");

	//GameStateClass = AUTBetrayal_::StaticClass();
	PlayerStateClass = AUTBetrayalPlayerState::StaticClass();
	TeamClass = AUTBetrayalTeam::StaticClass();
	AnnouncerMessageClass = UUTBetrayalMessage::StaticClass();

	static ConstructorHelpers::FObjectFinder<UClass> InstagibRifle(TEXT("Class'/Game/RestrictedAssets/Weapons/ShockRifle/BP_InstagibRifle.BP_InstagibRifle_C'"));
	InstagibRifleClass = InstagibRifle.Object;

	bForceRespawn = true;

	RogueValue = 6;

	static ConstructorHelpers::FObjectFinder<USoundCue> BetrayingSoundAsset(TEXT("SoundCue'/UTBetrayal/Sounds/A_Gameplay_CTF_EnemyFlagGrab_Cue.A_Gameplay_CTF_EnemyFlagGrab_Cue'"));
	static ConstructorHelpers::FObjectFinder<USoundCue> BetrayedSoundAsset(TEXT("SoundCue'/UTBetrayal/Sounds/A_Gameplay_CTF_EnemyFlagReturn_Cue.A_Gameplay_CTF_EnemyFlagReturn_Cue'"));
	static ConstructorHelpers::FObjectFinder<USoundCue> JoinTeamSoundAsset(TEXT("SoundCue'/UTBetrayal/Sounds/A_Gameplay_CTF_TeamFlagReturn_Cue.A_Gameplay_CTF_TeamFlagReturn_Cue'"));
	BetrayingSound = BetrayingSoundAsset.Object;
	BetrayedSound = BetrayedSoundAsset.Object;
	JoinTeamSound = JoinTeamSoundAsset.Object;
}

void AUTBetrayalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	bClearPlayerInventory = true;
	DefaultInventory.Empty();

	if (InstagibRifleClass != NULL)
	{
		DefaultInventory.Add(InstagibRifleClass);
	}
}

void AUTBetrayalGameMode::BeginGame()
{
	Super::BeginGame();

	GetWorldTimerManager().SetTimer(TimerHandle_MaybeStartTeam, this, &AUTBetrayalGameMode::MaybeStartTeam, 1.0f, true);
}

bool AUTBetrayalGameMode::AllowMutator(TSubclassOf<AUTMutator> MutClass)
{
	if (MutClass != NULL)
	{
	//	if (/*(MutClass == AUTMutator_Handicap::StaticClass()) ||
	//		(MutClass == AUTMutator_NoPowerups::StaticClass()) ||
	//		(MutClass == AUTMutator_NoTranslocator::StaticClass()) ||
	//		(MutClass == AUTMutator_NoOrbs::StaticClass()) ||
	//		(MutClass == AUTMutator_Survival::StaticClass()) ||
	//		(MutClass == AUTMutator_Instagib::StaticClass()) ||*/
	//		(MutClass == AUTMutator_WeaponArena::StaticClass()) ||
	//		(MutClass == AUTMutator_WeaponReplacement::StaticClass()) /*||
	//		(MutClass == AUTMutator_WeaponsRespawn::StaticClass()) ||
	//		(MutClass == AUTMutator_Hero::StaticClass()) */)
	//	{
	//		return false;
	//	}
	}

	return Super::AllowMutator(MutClass);
}

bool AUTBetrayalGameMode::CheckRelevance_Implementation(AActor* Other)
{
	// TODO: Add Vehicle support
	if (Other->IsA(AUTWeapon::StaticClass())/* && !Other->IsA(AUTVehicleWeapon::StaticClass())*/)
	{
		if (Other->GetClass() == InstagibRifleClass)
		{
			// TODO: Check for Instagib Rifle and set Instagib flag
			//UTWeap_InstagibRifle(Other).bBetrayalMode = true;
			return true;
		}
		return false;
	}
	else if (Other->IsA(AUTPickup::StaticClass()))
	{
		return false;
	}
	return Super::CheckRelevance_Implementation(Other);
}

void AUTBetrayalGameMode::ShotTeammate(AUTBetrayalPlayerState* InstigatorPRI, AUTBetrayalPlayerState* HitPRI, AUTCharacter* ShotInstigator, AUTCharacter* HitPawn)
{
	if (HitPawn != NULL && HitPawn->IsSpawnProtected())
	{
		return;
	}

	AUTBetrayalTeam* Team = InstigatorPRI->CurrentTeam;
	if (Team == NULL)
		return;

	InstigatorPRI->Score += Team->TeamPot;

	// TODO: Add stats tracking
	//Increment pot stat
	//InstigatorPRI->AddToEventStat('EVENT_POOLPOINTS', Team.TeamPot);
	//AUTPlayerController* PC = Cast<AUTPlayerController>(InstigatorPRI->GetOwner());
	//if (PC != NULL)
	//{
	//	PC->ClientUpdateAchievement(EUTA_UT3GOLD_CantBeTrusted, Team.TeamPot);
	//}

	Team->TeamPot = 0;
	InstigatorPRI->SetRogueTimer();
	InstigatorPRI->BetrayalCount++;
	InstigatorPRI->BetrayedTeam = Team;
	HitPRI->Betrayer = InstigatorPRI;
	// TODO: Add play sound
	//InstigatorPRI->PlaySound(BetrayingSound);
	UUTGameplayStatics::UTPlaySound(GetWorld(), BetrayingSound, InstigatorPRI);

	for (APlayerState* PS : GameState->PlayerArray)
	{
		AUTBetrayalPlayerState* PRI = Cast<AUTBetrayalPlayerState>(PS);
		if (PRI == NULL) continue;

		AUTPlayerController* OtherPC = Cast<AUTPlayerController>(PRI->GetOwner());
		if (OtherPC != NULL)
		{
			if (PRI->CurrentTeam == Team)
			{
				// big, with "assassin"
				OtherPC->ClientReceiveLocalizedMessage(AnnouncerMessageClass, 0, InstigatorPRI, HitPRI, Team);
			}
			else
			{
				// smaller, no announcement
				OtherPC->ClientReceiveLocalizedMessage(AnnouncerMessageClass, 4, InstigatorPRI, HitPRI, Team);
			}
		}
	}

	//Record a betrayal stat?

	RemoveFromTeam(InstigatorPRI);

	if (!Team->IsPendingKillPending())
	{
		// give betrayer to other teammate
		for (int32 i = 0; i<ARRAY_COUNT(Team->Teammates); i++)
		{
			if (Team->Teammates[i] != NULL)
			{
				Team->Teammates[i]->Betrayer = InstigatorPRI;
				AUTPlayerController* OtherPC = Cast<AUTPlayerController>(Team->Teammates[i]->GetOwner());
				if (OtherPC != NULL)
				{
					OtherPC->ClientPlaySound(BetrayedSound);
				}
			}
		}
	}
}

void AUTBetrayalGameMode::RemoveFromTeam(AUTBetrayalPlayerState* PRI)
{
	if (PRI == NULL)
		return;

	//Drop the PRI from the team
	AUTBetrayalTeam* Team = PRI->CurrentTeam;
	if (Team == NULL)
		return;

	int32 NumTeammates = Team->LoseTeammate(PRI);
	if (NumTeammates == 1)
	{
		for (int32 i=0; i<ARRAY_COUNT(Team->Teammates); i++)
		{
			if (Team->Teammates[i] != NULL)
			{
				Team->Teammates[i]->Score += Team->TeamPot;

				// TODO: Add stats tracking
				//Increment pot stat
				//Team->Teammates[i].AddToEventStat('EVENT_POOLPOINTS', Team->TeamPot);

				//Disband the team
				NumTeammates = Team->LoseTeammate(Team->Teammates[i]);
				break;
			}
		}
	}

	//Destroy the team completely
	if (NumTeammates == 0)
	{
		Team->Destroy();
		RemoveTeam(Team);
	}
}

void AUTBetrayalGameMode::RemoveTeam(AUTBetrayalTeam* Team)
{
	for (int32 i=0; i<Teams.Num(); i++)
	{
		//Remove the team we're looking for
		if (Teams[i] == Team)
		{
			Teams.RemoveAt(i, 1);
			break;
		}

		//Remove any extraneous teams (empty or deleted)
		if ((Teams[i] == NULL) || Teams[i]->IsPendingKillPending())
		{
			Teams.RemoveAt(i, 1);
			i--;
		}
	}
}

void AUTBetrayalGameMode::MaybeStartTeam()
{
	if (HasMatchEnded())
	{
		GetWorldTimerManager().ClearTimer(TimerHandle_MaybeStartTeam);
		return;
	}

	int32 Count = 0;
	int32 TeamCount = 0;
	int32 MaxTeamSize = (NumPlayers + NumBots > 6) ? 3 : 2;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		AUTBetrayalPlayerState* PRI = Cast<AUTBetrayalPlayerState>(PS);
		if ((PRI != NULL) && (PRI->CurrentTeam == NULL) && !PRI->bIsRogue && !PRI->bIsSpectator)
		{
			// first try to place on existing team - but not the one you've betrayed before, or one that has too big a pot
			for (int32 j=0; j<Teams.Num(); j++)
			{
				if (Teams[j] != PRI->BetrayedTeam)
				{
					if (Teams[j]->AddTeammate(PRI, MaxTeamSize))
					{
						//Successfully added to a team
						// TODO: Add play sound
						//PRI->PlaySound(JoinTeamSound);
						UUTGameplayStatics::UTPlaySound(GetWorld(), JoinTeamSound, PRI);
						return;
					}
				}
			}

			//Number of people not placed on a team during existing team forming
			Count++;
		}
	}

	// maybe form team from freelancers
	if (Count > 1)
	{
		AUTBetrayalTeam* NewTeam = GetWorld()->SpawnActor<AUTBetrayalTeam>(TeamClass);
		if (NewTeam != NULL)
		{
			Teams.Add(NewTeam);

			for (APlayerState* PS : GameState->PlayerArray)
			{
				AUTBetrayalPlayerState* PRI = Cast<AUTBetrayalPlayerState>(PS);
				if ((PRI != NULL) && (PRI->CurrentTeam == NULL) && !PRI->bIsRogue && !PRI->bIsSpectator)
				{
					if (NewTeam->AddTeammate(PRI, MaxTeamSize))
					{
						//Successfully added to a team
						// TODO: Add play sound
						//PRI->PlaySound(JoinTeamSound);
						UUTGameplayStatics::UTPlaySound(GetWorld(), JoinTeamSound, PRI);

						AUTPlayerController* PC = Cast<AUTPlayerController>(PRI->GetOwner());
						if (PC != NULL)
						{
							PC->ClientReceiveLocalizedMessage(AnnouncerMessageClass, 1);
						}
						TeamCount++;
					}

					if (TeamCount == MaxTeamSize)
					{
						break;
					}
				}
			}
		}
	}
}

void AUTBetrayalGameMode::Logout(AController* Exiting)
{
	if (Exiting != NULL)
	{
		AUTBetrayalPlayerState* PRI = Cast<AUTBetrayalPlayerState>(Exiting);
		if (PRI != NULL && PRI->CurrentTeam != NULL)
		{
			RemoveFromTeam(PRI);
		}
	}

	Super::Logout(Exiting);
}

void AUTBetrayalGameMode::ScoreKill(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType)
{
	AUTBetrayalPlayerState* KillerPRI = NULL;
	AUTBetrayalPlayerState* OtherPRI = NULL;

	if (Killer != NULL)
	{
		KillerPRI = Cast<AUTBetrayalPlayerState>(Killer->PlayerState);
	}

	if ((Killer == Other) || (Killer == NULL))
	{
		if ((Other != NULL) && (Other->PlayerState != NULL))
		{
			Other->PlayerState->Score -= 1;
			Other->PlayerState->ForceNetUpdate();
		}
	}
	else if (KillerPRI != NULL)
	{
		OtherPRI = Cast<AUTBetrayalPlayerState>(Other->PlayerState);
		if (OtherPRI != NULL)
		{
			KillerPRI->Score += OtherPRI->ScoreValueFor(KillerPRI);
			if (OtherPRI->bIsRogue && (OtherPRI == KillerPRI->Betrayer))
			{
				AUTPlayerController* KillerPC = Cast<AUTPlayerController>(KillerPRI->GetOwner());
				if (KillerPC != NULL)
				{
					KillerPC->ClientReceiveLocalizedMessage(AnnouncerMessageClass, 2);
				}
				AUTPlayerController* OtherPC = Cast<AUTPlayerController>(OtherPRI->GetOwner());
				if (OtherPC != NULL)
				{
					OtherPC->ClientReceiveLocalizedMessage(AnnouncerMessageClass, 3);
				}

				// TODO: Add stats tracking
				//Retribution stat
				//KillerPRI->IncrementEventStat('EVENT_RETRIBUTIONS');
				//if (KillerPC != NULL)
				//{
				//	KillerPC.ClientUpdateAchievement(EUTA_UT3GOLD_Avenger, 1);
				//}

				OtherPRI->RogueExpired();
			}

			KillerPRI->ForceNetUpdate();
			KillerPRI->Kills++;
			if (KillerPRI->CurrentTeam != NULL)
			{
				KillerPRI->CurrentTeam->TeamPot++;

				// TODO: add bot support
				//if (KillerPRI->CurrentTeam->TeamPot > 2)
				//{
				//	for (int32 i = 0; i< ARRAY_COUNT(KillerPRI->CurrentTeam->Teammates); i++)
				//	{
				//		if (KillerPRI->CurrentTeam->Teammates[i] != NULL)
				//		{
				//			AUTBot* B = Cast<AUTBot>(KillerPRI->CurrentTeam->Teammates[i]->GetOwner());
				//			if ((B != NULL) && !B->bBetrayTeam)
				//			{
				//				int32 BetrayalValue = KillerPRI->CurrentTeam->TeamPot + 0.3*Cast<AUTBetrayalPlayerState>(B->PlayerState)->ScoreValueFor(KillerPRI);
				//				if ((BetrayalValue > 1.5 + RogueValue - B->CurrentAggression + Cast<AUTBetrayalPlayerState>(B->PlayerState)->GetTrustWorthiness()) && (FMath::FRand() < 0.2))
				//				{
				//					//UE_LOG(UT, Verbose, TEXT("Beacon %s lost connection. Attempting to recreate."), *GetNameSafe(this));
				//					// `log(Instigator.Controller.ShotTarget.Controller->PlayerState.PlayerName$" betrayal value "$BetrayalValue$" vs "$(1.5 + RogueValue - B.Aggressiveness + Cast<AUTBetrayalPlayerState>(B->PlayerState).GetTrustWorthiness()));
				//					B.bBetrayTeam = true;
				//				}
				//			}
				//		}
				//	}
				//}
			}
		}
	}

	if (BaseMutator != NULL)
	{
		BaseMutator->ScoreKill(Killer, Other, DamageType);
	}

	// TODO: Add support for MaxLives
	if ((Killer != NULL)/* || (MaxLives > 0)*/)
	{
		CheckScore(KillerPRI);
	}
}
