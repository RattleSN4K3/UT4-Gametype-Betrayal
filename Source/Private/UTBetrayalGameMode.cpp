#include "UTBetrayal.h"
#include "UTBetrayalGameMode.h"
#include "UTBetrayalCharacter.h"
#include "UTBetrayalGameState.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalHUD.h"
#include "UTBetrayalMessage.h"
#include "UTBetrayalBot.h"
#include "UTBetrayalCharacterPostRenderer.h"
#include "UTBetrayalCharacterTeamColor.h"

#include "UTFirstBloodMessage.h"
#include "UTMutator_WeaponArena.h"
#include "UTMutator_WeaponReplacement.h"

AUTBetrayalGameMode::AUTBetrayalGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	DisplayName = NSLOCTEXT("UTGameMode", "BET", "Betrayal");

	static ConstructorHelpers::FObjectFinder<UClass> PlayerPawnObject(TEXT("Class'/UTBetrayal/DefaultCharacter_Betrayal.DefaultCharacter_Betrayal_C'"));
	DefaultPawnClass = PlayerPawnObject.Object;
	//DefaultPawnClass = AUTBetrayalCharacter::StaticClass();

	GameStateClass = AUTBetrayalGameState::StaticClass();
	PlayerStateClass = AUTBetrayalPlayerState::StaticClass();
	HUDClass = AUTBetrayalHUD::StaticClass();

	TeamClass = AUTBetrayalTeam::StaticClass();
	AnnouncerMessageClass = UUTBetrayalMessage::StaticClass();

	// Workaround for bots to betray team members
	// TODO: Option to have PickNewEnemy/IsTeammate as Squad/BotDecisionComponent (Pull request?)
	BotClass = AUTBetrayalBot::StaticClass();

	//static ConstructorHelpers::FObjectFinder<UClass> InstagibRifle(TEXT("Class'/Game/RestrictedAssets/Weapons/ShockRifle/BP_InstagibRifle.BP_InstagibRifle_C'"));
	static ConstructorHelpers::FObjectFinder<UClass> InstagibRifle(TEXT("Class'/UTBetrayal/BP_InstagibRifle_Betrayal.BP_InstagibRifle_Betrayal_C'"));
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

	// force force-respawn
	bForceRespawn = true;

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
		// TODO: disallow some stock mutators
		// TODO: disallow some custom/3rd-party mutators

		if (/*(MutClass == AUTMutator_Handicap::StaticClass()) ||
			(MutClass == AUTMutator_NoPowerups::StaticClass()) ||
			(MutClass == AUTMutator_NoTranslocator::StaticClass()) ||
			(MutClass == AUTMutator_NoOrbs::StaticClass()) ||
			(MutClass == AUTMutator_Survival::StaticClass()) ||
			(MutClass == AUTMutator_Instagib::StaticClass()) ||*/
			(MutClass->IsChildOf<AUTMutator_WeaponArena>()) ||
			(MutClass->IsChildOf<AUTMutator_WeaponReplacement>()) /*||
			(MutClass == AUTMutator_WeaponsRespawn::StaticClass()) ||
			(MutClass == AUTMutator_Hero::StaticClass()) */)
		{
			return false;
		}
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

void AUTBetrayalGameMode::ShotTeammate(AUTBetrayalPlayerState* InstigatorPRI, AUTBetrayalPlayerState* HitPRI, APawn* ShotInstigator, APawn* HitPawn)
{
	AUTCharacter* HitChar = Cast<AUTCharacter>(HitPawn);
	if (HitChar != NULL && HitChar->IsSpawnProtected())
	{
		return;
	}

	AUTBetrayalTeam* Team = InstigatorPRI->CurrentTeam;
	if (Team == NULL)
		return;

	InstigatorPRI->Score += Team->TeamPot;
	InstigatorPRI->BetrayalPot += Team->TeamPot;
	HitPRI->BetrayedPot += Team->TeamPot;

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
	HitPRI->BetrayedCount++;
	HitPRI->Betrayer = InstigatorPRI;
	UUTGameplayStatics::UTPlaySound(GetWorld(), BetrayingSound, InstigatorPRI->GetOwner());

	InstigatorPRI->UpdateNemesis(HitPRI);

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
		for (int32 i = 0; i<ARRAY_COUNT(Team->Teammates); i++)
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
	for (int32 i = 0; i<Teams.Num(); i++)
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
			for (int32 j = 0; j<Teams.Num(); j++)
			{
				if (Teams[j] != PRI->BetrayedTeam)
				{
					if (Teams[j]->AddTeammate(PRI, MaxTeamSize))
					{
						PRI->UpdateTeam(Teams[j]);

						//Successfully added to a team
						UUTGameplayStatics::UTPlaySound(GetWorld(), JoinTeamSound, PRI->GetOwner());
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
						PRI->UpdateTeam(NewTeam);

						//Successfully added to a team
						UUTGameplayStatics::UTPlaySound(GetWorld(), JoinTeamSound, PRI->GetOwner());

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
		AUTBetrayalPlayerState* PRI = Cast<AUTBetrayalPlayerState>(Exiting->PlayerState);
		if (PRI != NULL && PRI->CurrentTeam != NULL)
		{
			RemoveFromTeam(PRI);
		}
	}

	Super::Logout(Exiting);
}

void AUTBetrayalGameMode::ScoreKill(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType)
{
	if (Killer != NULL && Killer != Other)
	{
		AUTBetrayalPlayerState* KillerPRI = Cast<AUTBetrayalPlayerState>(Killer->PlayerState);
		AUTBetrayalPlayerState* OtherPRI = Cast<AUTBetrayalPlayerState>(Other->PlayerState);
		if (KillerPRI != NULL && OtherPRI != NULL)
		{
			// only add additional score // Score - 1
			KillerPRI->AdjustScore(OtherPRI->ScoreValueFor(KillerPRI) - 1);

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

				KillerPRI->RetributionCount++;
				OtherPRI->PaybackCount++;

				// TODO: Add stats tracking
				//Retribution stat
				//KillerPRI->IncrementEventStat('EVENT_RETRIBUTIONS');
				//if (KillerPC != NULL)
				//{
				//	KillerPC.ClientUpdateAchievement(EUTA_UT3GOLD_Avenger, 1);
				//}

				OtherPRI->RogueExpired();
			}

			if (KillerPRI->CurrentTeam != NULL)
			{
				KillerPRI->CurrentTeam->TeamPot++;
				KillerPRI->HighestPot = FMath::Max<int32>(KillerPRI->HighestPot, KillerPRI->CurrentTeam->TeamPot);

				// TODO: add proper bot support
				if (KillerPRI->CurrentTeam->TeamPot > 2)
				{
					for (int32 i = 0; i< ARRAY_COUNT(KillerPRI->CurrentTeam->Teammates); i++)
					{
						if (KillerPRI->CurrentTeam->Teammates[i] != NULL)
						{
							AUTBetrayalPlayerState* BotPRI = KillerPRI->CurrentTeam->Teammates[i];

							// TODO: store flag else where FIXME: do not use subclassed UTBot
							AUTBetrayalBot* B = Cast<AUTBetrayalBot>(KillerPRI->CurrentTeam->Teammates[i]->GetOwner());
							if ((B != NULL) && !B->bBetrayTeam)
							{
								float BetrayalValue = (float)(KillerPRI->CurrentTeam->TeamPot) + 0.3f*(float)BotPRI->ScoreValueFor(KillerPRI);
								float BetrayalRandomness = 1.5 + RogueValue - B->CurrentAggression + BotPRI->GetTrustWorthiness();

								UE_LOG(Betrayal, Verbose, TEXT("%s betrayal value %d vs. %d"), *KillerPRI->PlayerName, BetrayalValue, BetrayalRandomness);
								if ((BetrayalValue > BetrayalRandomness) && (FMath::FRand() < 0.2))
								{
									B->BetrayAggressiveness = BetrayalRandomness != 0.0 ? BetrayalValue / BetrayalRandomness : 0.5;
									B->bBetrayTeam = true;
								}
							}
						}
					}
				}
			}
		}
	}

	Super::ScoreKill(Killer, Other, KilledPawn, DamageType);
}

void AUTBetrayalGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	// FIXME: workaround for JumpBoots calling SetPlayerDefaults
	// TODO: Pull Request/Forum thread update to get this fixed
	if (!AlreadySpawnedPlayers.Contains(PlayerPawn))
	{
		AlreadySpawnedPlayers.Add(PlayerPawn);
		AlreadySpawnedPlayers.Remove(NULL);

		// Workaround for PostRender routed to HUD
		// TODO: FIXME: Route PostRender to UTHUD class

		FActorSpawnParameters Params;
		Params.bNoCollisionFail = true;
		Params.Owner = PlayerPawn;
		GetWorld()->SpawnActor<AUTBetrayalCharacterPostRenderer>(AUTBetrayalCharacterPostRenderer::StaticClass(), Params);

		// END Workaround for PostRender routed to HUD

		// Workaround for applying TeamColor to players

		Params = FActorSpawnParameters();
		Params.bNoCollisionFail = true;
		Params.Owner = PlayerPawn;
		GetWorld()->SpawnActor<AUTBetrayalCharacterTeamColor>(AUTBetrayalCharacterTeamColor::StaticClass(), Params);

		// END Workaround for applying TeamColor to players

	}

	Super::SetPlayerDefaults(PlayerPawn);
}

#if !UE_SERVER

void AUTBetrayalGameMode::BuildPlayerInfo(TSharedPtr<SVerticalBox> Panel, AUTPlayerState* PlayerState)
{
	Super::BuildPlayerInfo(Panel, PlayerState);

	if (AUTBetrayalPlayerState* BPRI = Cast<AUTBetrayalPlayerState>(PlayerState))
	{
		// TODO: Localization
		Panel->AddSlot().Padding(30.0, 5.0, 30.0, 0.0)
		[
			NewPlayerInfoLine(FString("Betrayals"), FString::Printf(TEXT("%i"), BPRI->BetrayalCount))
		];
		Panel->AddSlot().Padding(30.0, 5.0, 30.0, 0.0)
		[
			NewPlayerInfoLine(FString("Victim"), FString::Printf(TEXT("%i"), BPRI->BetrayedCount))
		];
		Panel->AddSlot().Padding(30.0, 5.0, 30.0, 0.0)
		[
			NewPlayerInfoLine(FString("Retributions"), FString::Printf(TEXT("%i"), BPRI->RetributionCount))
		];
		Panel->AddSlot().Padding(30.0, 5.0, 30.0, 0.0)
		[
			NewPlayerInfoLine(FString("Paybacks"), FString::Printf(TEXT("%i"), BPRI->PaybackCount))
		];

		Panel->AddSlot().Padding(30.0, 5.0, 30.0, 0.0)
		[
			NewPlayerInfoLine(FString("Average Betrayal Pot"), RoundPerc(BPRI->BetrayalPot, BPRI->BetrayalCount).ToString())
		];
		Panel->AddSlot().Padding(30.0, 5.0, 30.0, 0.0)
		[
			NewPlayerInfoLine(FString("Average Victim Pot"), RoundPerc(BPRI->BetrayedPot, BPRI->BetrayedCount).ToString())
		];
		Panel->AddSlot().Padding(30.0, 5.0, 30.0, 0.0)
		[
			NewPlayerInfoLine(FString("Highest Pot"), FString::Printf(TEXT("%i"), BPRI->HighestPot))
		];

		APlayerController* PC = Cast<APlayerController>(PlayerState->GetOwner());
		if (PC != NULL && PC->IsLocalPlayerController())
		{
			Panel->AddSlot().Padding(30.0, 15.0, 30.0, 0.0)
			[
				NewPlayerInfoLine(FString("Nemesis"), BPRI->CurrentNemesis.IsEmpty() ? FString(TEXT("-")) : BPRI->CurrentNemesis)
			];
		}
	}
}

#endif

FText AUTBetrayalGameMode::BuildServerRules(AUTGameState* GameState)
{
	FFormatNamedArguments Args;
	Args.Add("Rules", Super::BuildServerRules(GameState));
	Args.Add("RogueValue", FText::AsNumber(RogueValue));
	Args.Add("RogueTimePenalty", FText::AsNumber(AUTBetrayalPlayerState::StaticClass()->GetDefaultObject<AUTBetrayalPlayerState>()->RogueTimePenalty));

	return FText::Format(NSLOCTEXT("UTBetrayalGameMode", "GameRules", "{Rules}  Rogue Value: {RogueValue}  Rogue time penalty: {RogueTimePenalty} s"), Args);
}

void AUTBetrayalGameMode::BuildServerResponseRules(FString& OutRules)
{
	// TODO: Re-order
	OutRules += FString::Printf(TEXT("Rogue Value\t%i\t"), RogueValue);
	OutRules += FString::Printf(TEXT("Rogue time penalty\t%i\t"), AUTBetrayalPlayerState::StaticClass()->GetDefaultObject<AUTBetrayalPlayerState>()->RogueTimePenalty);
}
