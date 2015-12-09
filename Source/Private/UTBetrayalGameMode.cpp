#include "UTBetrayal.h"
#include "UTBetrayalGameMode.h"
//#include "UTBetrayalCharacter.h"
#include "UTBetrayalGameState.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalHUD.h"
#include "UTBetrayalMessage.h"
#include "UTBetrayalBot.h"
#include "UTBetrayalCharacterPostRenderer.h"
#include "UTBetrayalCharacterTeamColor.h"

#include "UTMutator_WeaponArena.h"
#include "UTMutator_WeaponReplacement.h"

#include "UTPlayerState.h"
#include "UTArmor.h"
#include "UTTimedPowerup.h"

#include "SNumericEntryBox.h"
#include "Private/Slate/Widgets/SUTTabWidget.h"
#include "StatNames.h"

AUTBetrayalGameMode::AUTBetrayalGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UClass> PawnClass;
		ConstructorHelpers::FObjectFinder<UClass> InstaGibRifleClass;

		ConstructorHelpers::FObjectFinder<USoundCue> BetrayingSoundAsset;
		ConstructorHelpers::FObjectFinder<USoundCue> BetrayedSoundAsset;
		ConstructorHelpers::FObjectFinder<USoundCue> JoinTeamSoundAsset;

		ConstructorHelpers::FObjectFinder<UClass> Mutator_AddTrans;
		ConstructorHelpers::FObjectFinder<UClass> Mutator_FriendlyFire;
		ConstructorHelpers::FObjectFinder<UClass> Mutator_Instagib;
		ConstructorHelpers::FObjectFinder<UClass> Mutator_WeaponArena;
		ConstructorHelpers::FObjectFinder<UClass> Mutator_WeaponReplacement;

		FConstructorStatics()
			: PawnClass(TEXT("Class'/UTBetrayal/DefaultCharacter_Betrayal.DefaultCharacter_Betrayal_C'"))
			, InstaGibRifleClass(TEXT("Class'/UTBetrayal/BP_InstagibRifle_Betrayal.BP_InstagibRifle_Betrayal_C'")) // Class'/Game/RestrictedAssets/Weapons/ShockRifle/BP_InstagibRifle.BP_InstagibRifle_C'

			, Mutator_AddTrans(TEXT("Class'/Game/RestrictedAssets/Blueprints/Mutator_AddTrans.Mutator_AddTrans_C'"))
			, Mutator_FriendlyFire(TEXT("Class'/Game/RestrictedAssets/Blueprints/Mutator_FriendlyFire.Mutator_FriendlyFire_C'"))
			, Mutator_Instagib(TEXT("Class'/Game/RestrictedAssets/Blueprints/Mutator_Instagib.Mutator_Instagib_C'"))
			, Mutator_WeaponArena(TEXT("Class'/Script/UnrealTournament.UTMutator_WeaponArena'"))
			, Mutator_WeaponReplacement(TEXT("Class'/Script/UnrealTournament.UTMutator_WeaponReplacement'"))

			, BetrayingSoundAsset(TEXT("SoundCue'/UTBetrayal/Sounds/A_Gameplay_CTF_EnemyFlagGrab_Cue.A_Gameplay_CTF_EnemyFlagGrab_Cue'"))
			, BetrayedSoundAsset(TEXT("SoundCue'/UTBetrayal/Sounds/A_Gameplay_CTF_EnemyFlagReturn_Cue.A_Gameplay_CTF_EnemyFlagReturn_Cue'"))
			, JoinTeamSoundAsset(TEXT("SoundCue'/UTBetrayal/Sounds/A_Gameplay_CTF_TeamFlagReturn_Cue.A_Gameplay_CTF_TeamFlagReturn_Cue'"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;


	DisplayName = NSLOCTEXT("UTBetrayal", "GameName", "Betrayal");

	PlayerPawnObject.Reset(); // clearing out PlayerPawnObject to prevent InitGame overriding DefaultPawnClass, allowing Blueprint sub-classes specifiying Pawn class
	DefaultPawnClass = ConstructorStatics.PawnClass.Object;
	//DefaultPawnClass = AUTBetrayalCharacter::StaticClass();

	InstagibRifleClass = ConstructorStatics.InstaGibRifleClass.Object;

	GameStateClass = AUTBetrayalGameState::StaticClass();
	PlayerStateClass = AUTBetrayalPlayerState::StaticClass();
	HUDClass = AUTBetrayalHUD::StaticClass();

	TeamClass = AUTBetrayalTeam::StaticClass();
	AnnouncerMessageClass = UUTBetrayalMessage::StaticClass();

	// Workaround for bots to betray team members
	// TODO: Option to have PickNewEnemy/IsTeammate as Squad/BotDecisionComponent (Pull request?)
	BotClass = AUTBetrayalBot::StaticClass();

	DisallowedMutators.Add(ConstructorStatics.Mutator_AddTrans.Object);
	DisallowedMutators.Add(ConstructorStatics.Mutator_FriendlyFire.Object);
	DisallowedMutators.Add(ConstructorStatics.Mutator_Instagib.Object);
	DisallowedMutators.Add(ConstructorStatics.Mutator_WeaponArena.Object);
	DisallowedMutators.Add(ConstructorStatics.Mutator_WeaponReplacement.Object);

	DisallowedPickupFactories.Add(AUTPickupWeapon::StaticClass());

	DisallowedInventories.Add(AUTTimedPowerup::StaticClass());
	DisallowedInventories.Add(AUTArmor::StaticClass());
	DisallowedInventories.Add(AUTWeapon::StaticClass()); // just in case

	BetrayingSound = ConstructorStatics.BetrayingSoundAsset.Object;
	BetrayedSound = ConstructorStatics.BetrayedSoundAsset.Object;
	JoinTeamSound = ConstructorStatics.JoinTeamSoundAsset.Object;

	bForceRespawn = true;
	ForceRespawnTime = RespawnWaitTime;
	bPlayPlayerIntro = false;

	RogueValue = 6;
	RogueTimePenalty = 30.0;
	bAllowPickups = false;
}

void AUTBetrayalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
#if ENGINE_VERSION <= 2564698
// COMPAT: Random number generator is fixed as of commit cc4dce6f27e80af481e5e4b087baf57ff6fe17ec

#if WITH_EDITOR || UE_BUILD_DEBUG || BETRAYAL_DEBUG
	// Rand is used for random values to the HUD which is only setup when compiled for the editor
	double secs = FTimespan(FDateTime::Now().GetTicks()).GetTotalSeconds();
	int32 seed = (int32)(((int64)secs) % INT_MAX);
	FMath::RandInit(seed);
#endif
#endif

	Super::InitGame(MapName, Options, ErrorMessage);

	// force force-respawn
	bForceRespawn = true;
	ForceRespawnTime = RespawnWaitTime; // force respawn exactly after the waiting time

	bClearPlayerInventory = true;
	DefaultInventory.Empty();

	if (InstagibRifleClass != NULL)
	{
		DefaultInventory.Add(InstagibRifleClass);
	}

	bForcePlayerIntro = HasOption(Options, TEXT("PlayPlayerIntro"));

	RogueValue = FMath::Max(1, GetIntOption(Options, TEXT("RogueValue"), RogueValue));
	RogueTimePenalty = FMath::Max(0, GetIntOption(Options, TEXT("RogueTimePenalty"), RogueTimePenalty));

	FString InOpt = ParseOption(Options, TEXT("AllowPickups"));
	bAllowPickups = EvalBoolOptions(InOpt, bAllowPickups);
}

void AUTBetrayalGameMode::BeginGame()
{
	Super::BeginGame();
	ConditionallyStartTeamTimer();
}

void AUTBetrayalGameMode::StartMatch()
{
	uint8 bOldPlayPlayerIntro = 0;
	if (!HasMatchStarted() && !bForcePlayerIntro)
	{
		// only play intro if enough real players are playing
		// TODO: Check replication of bPlayPlayerIntro if it's working correctly
		bOldPlayPlayerIntro = bPlayPlayerIntro ? 1 : 255;
		bPlayPlayerIntro = NumPlayers > 2;
	}

	Super::StartMatch();

	// revert play-intro flag
	if (bOldPlayPlayerIntro > 0)
	{
		bPlayPlayerIntro = (bPlayPlayerIntro == 1);
	}

	// in general, BeginGame handles starting the timer (once the count down has ended)
	// but the countdown in not existent in PIE sessions, therefore we reliably start the
	// timer in here as well (checking if already running)
	if (GetWorld()->IsPlayInEditor())
	{
		ConditionallyStartTeamTimer();
	}
}

void AUTBetrayalGameMode::ConditionallyStartTeamTimer()
{
	if (!GetWorldTimerManager().IsTimerActive(TimerHandle_MaybeStartTeam))
	{
		GetWorldTimerManager().SetTimer(TimerHandle_MaybeStartTeam, this, &AUTBetrayalGameMode::MaybeStartTeam, 1.0f, true);
	}
}

bool AUTBetrayalGameMode::AllowMutator(TSubclassOf<AUTMutator> MutClass)
{
	if (MutClass != NULL)
	{
		// Original/core mutators were:
		// UTMutator_Handicap, UTMutator_NoPowerups, UTMutator_NoTranslocator, 
		// UTMutator_NoOrbs, UTMutator_Survival, UTMutator_Instagib, 
		// UTMutator_WeaponArena, UTMutator_WeaponReplacement, UTMutator_WeaponsRespawn,
		// UTMutator_Hero

		// TODO: disallow some more stock mutators
		// TODO: disallow some custom/3rd-party mutators

		for (auto Mutator : DisallowedMutators)
		{
			if (MutClass->IsChildOf(Mutator))
			{
				return false;
			}
		}
	}

	return Super::AllowMutator(MutClass);
}

bool AUTBetrayalGameMode::CheckRelevance_Implementation(AActor* Other)
{
	// TODO: Add Vehicle support
	if (Other->IsA(AUTWeapon::StaticClass())/* && !Other->IsA(AUTVehicleWeapon::StaticClass())*/)
	{
		return Other->GetClass() == InstagibRifleClass;
	}
	else if (Other->IsA(AUTPickup::StaticClass()))
	{
		if (bAllowPickups)
		{
			bool bAllowed = true;
			for (auto DisallowdFac : DisallowedPickupFactories)
			{
				if (Other->GetClass()->IsChildOf(DisallowdFac))
				{
					bAllowed = false;
					break;
				}
			}

			if (bAllowed)
			{
				AUTPickupInventory* InvFac = Cast<AUTPickupInventory>(Other);
				if (InvFac && InvFac->GetInventoryType())
				{
					auto InvClass = InvFac->GetInventoryType();
					for (auto DisallowdInv : DisallowedInventories)
					{
						if (InvClass->IsChildOf(DisallowdInv))
						{
							bAllowed = false;
							break;
						}
					}
				}
			}

			if (bAllowed)
			{
				return true;
			}
		}

		return false;
	}
	return Super::CheckRelevance_Implementation(Other);
}

void AUTBetrayalGameMode::ShotTeammate(AUTBetrayalPlayerState* InstigatorPRI, AUTBetrayalPlayerState* HitPRI, APawn* ShotInstigator, APawn* HitPawn)
{
	AUTCharacter* HitChar = Cast<AUTCharacter>(HitPawn);
	if (InstigatorPRI == NULL || HitChar == NULL || HitChar->IsSpawnProtected())
	{
		return;
	}

	AUTBetrayalTeam* Team = InstigatorPRI->CurrentTeam;
	if (Team == NULL || HitPRI == NULL)
		return;

	InstigatorPRI->Score += Team->TeamPot;
	InstigatorPRI->BetrayalPot += Team->TeamPot;
	HitPRI->BetrayedPot += Team->TeamPot;

	// track highest/lowest pot for killing player
	InstigatorPRI->BetrayalLowestPot = InstigatorPRI->BetrayalLowestPot < 0 ? Team->TeamPot : FMath::Min<int32>(InstigatorPRI->BetrayalLowestPot, Team->TeamPot);
	InstigatorPRI->BetrayalHighestPot = FMath::Max<int32>(InstigatorPRI->BetrayalHighestPot, Team->TeamPot);

	// track highest/lowest pot for killed player
	HitPRI->BetrayedLowestPot = HitPRI->BetrayedLowestPot < 0 ? Team->TeamPot : FMath::Min<int32>(HitPRI->BetrayedLowestPot, Team->TeamPot);
	HitPRI->BetrayedHighestPot = FMath::Max<int32>(HitPRI->BetrayedHighestPot, Team->TeamPot);

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

	// increase betray count and update player killed by the most
	InstigatorPRI->UpdateNemesis(HitPRI);
	HitPRI->UpdateNemesis(InstigatorPRI);

	if (!Team->IsPendingKillPending())
	{
		// give betrayer to other teammate
		for (int32 i = 0; i<ARRAY_COUNT(Team->Teammates); i++)
		{
			if (Team->Teammates[i] != NULL)
			{
				Team->Teammates[i]->Betrayer = InstigatorPRI;
				if (auto OtherOwner = Team->Teammates[i]->GetOwner())
				{
					if (auto OtherPC = Cast<AUTPlayerController>(OtherOwner))
					{
						OtherPC->ClientPlaySound(BetrayedSound);
					}
					else if (auto OtherBot = Cast<AUTBetrayalBot>(OtherOwner))
					{
						// clear betray flag otherwise, teams with more players remaining would
						// still try to betray each other even if the Pot is now empty
						OtherBot->bBetrayTeam = false;
					}
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

	int32 TotalPlayerCount = GetTotalPlayingPlayers();
	if (TotalPlayerCount < 3)
		return;

	int32 Count = 0;
	int32 TeamCount = 0;
	int32 MaxTeamSize = (TotalPlayerCount > 6) ? 3 : 2;

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

		// no need for teams if there are only 3 player left. un-team all players
		// Note: the current bot is still counted, check for 3 players or less
		if (GetTotalPlayingPlayers() < 4 && Teams.Num() == 1 && !Teams[0]->IsPendingKillPending())
		{
			// TODO: what to do with the existing pot? Split pot to existing players?
			//       what if adding these points would end the game? Overtime with sudden death?

			if (Teams.IsValidIndex(0) && Teams[0] != NULL)
			{
				auto Team = Teams[0];
				Team->DisbandTeam();
				RemoveTeam(Team);	
				if (Team != NULL)
				{
					Team->Destroy();
				}
			}
			
			Teams.Reset();
		}
	}

	Super::Logout(Exiting);
}

void AUTBetrayalGameMode::ScoreKill_Implementation(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType)
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

				// TODO: add proper bot support
				if (KillerPRI->CurrentTeam->TeamPot > 2)
				{
					for (int32 i = 0; i< ARRAY_COUNT(KillerPRI->CurrentTeam->Teammates); i++)
					{
						if (AUTBetrayalPlayerState* TeamMatePRI = KillerPRI->CurrentTeam->Teammates[i])
						{
							// TODO: store flag else where FIXME: do not use subclassed UTBot
							AUTBetrayalBot* B = Cast<AUTBetrayalBot>(TeamMatePRI->GetOwner());
							if (B != NULL && !B->bBetrayTeam)
							{
								float BetrayalValue = (float)(KillerPRI->CurrentTeam->TeamPot) + 0.3f*(float)TeamMatePRI->ScoreValueFor(KillerPRI);
								float BetrayalRandomness = 1.5 + RogueValue - B->CurrentAggression + TeamMatePRI->GetTrustWorthiness();

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

	Super::ScoreKill_Implementation(Killer, Other, KilledPawn, DamageType);
}

#if !UE_SERVER

void AUTBetrayalGameMode::CreateConfigWidgets(TSharedPtr<class SVerticalBox> MenuSpace, bool bCreateReadOnly, TArray< TSharedPtr<TAttributePropertyBase> >& ConfigProps)
{
	Super::CreateConfigWidgets(MenuSpace, bCreateReadOnly, ConfigProps);

	TSharedPtr< TAttributeProperty<int32> > RogueValueAttr = StaticCastSharedPtr<TAttributeProperty<int32>>(FindGameURLOption(ConfigProps, TEXT("RogueValue")));
	TSharedPtr< TAttributeProperty<int32> > RogueTimePenaltyAttr = StaticCastSharedPtr<TAttributeProperty<int32>>(FindGameURLOption(ConfigProps, TEXT("RogueTimePenalty")));
	TSharedPtr< TAttributePropertyBool_TEMP > AllowPickupsAttr = StaticCastSharedPtr<TAttributePropertyBool_TEMP>(FindGameURLOption(ConfigProps, TEXT("AllowPickups")));

	if (RogueValueAttr.IsValid())
	{
		MenuSpace->AddSlot()
		.Padding(0.0f, 0.0f, 0.0f, 5.0f)
		.AutoHeight()
		.VAlign(VAlign_Top)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(350)
				[
					SNew(STextBlock)
					.TextStyle(SUWindowsStyle::Get(), "UT.Common.NormalText")
					.Text(NSLOCTEXT("AUTBetrayalGameMode", "RogueValue", "Rogue Value"))
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(20.0f, 0.0f, 0.0f, 0.0f)
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(300)
				[
					bCreateReadOnly ?
						StaticCastSharedRef<SWidget>(
							SNew(STextBlock)
							.TextStyle(SUWindowsStyle::Get(), "UT.Common.ButtonText.White")
							.Text(RogueValueAttr.ToSharedRef(), &TAttributeProperty<int32>::GetAsText)
					) :
						StaticCastSharedRef<SWidget>(
							SNew(SNumericEntryBox<int32>)
							.Value(RogueValueAttr.ToSharedRef(), &TAttributeProperty<int32>::GetOptional)
							.OnValueChanged(RogueValueAttr.ToSharedRef(), &TAttributeProperty<int32>::Set)
							.AllowSpin(true)
							.Delta(1)
							.MinValue(1)
							.MaxValue(999)
							.MinSliderValue(1)
							.MaxSliderValue(99)
							.EditableTextBoxStyle(SUWindowsStyle::Get(), "UT.Common.NumEditbox.White")
					)
				]
			]
		];
	}

	if (RogueTimePenaltyAttr.IsValid())
	{
		MenuSpace->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.Padding(0.0f, 0.0f, 0.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(350)
				[
					SNew(STextBlock)
					.TextStyle(SUWindowsStyle::Get(), "UT.Common.NormalText")
					.Text(NSLOCTEXT("AUTBetrayalGameMode", "RogueTimePenalty", "Rogue Time Penalty"))
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(20.0f, 0.0f, 0.0f, 0.0f)
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(300)
				[
					bCreateReadOnly ?
						StaticCastSharedRef<SWidget>(
							SNew(STextBlock)
							.TextStyle(SUWindowsStyle::Get(), "UT.Common.ButtonText.White")
							.Text(RogueTimePenaltyAttr.ToSharedRef(), &TAttributeProperty<int32>::GetAsText)
					) :
						StaticCastSharedRef<SWidget>(
							SNew(SNumericEntryBox<int32>)
							.Value(RogueTimePenaltyAttr.ToSharedRef(), &TAttributeProperty<int32>::GetOptional)
							.OnValueChanged(RogueTimePenaltyAttr.ToSharedRef(), &TAttributeProperty<int32>::Set)
							.AllowSpin(true)
							.Delta(1)
							.MinValue(0)
							.MaxValue(999)
							.MinSliderValue(0)
							.MaxSliderValue(99)
							.EditableTextBoxStyle(SUWindowsStyle::Get(), "UT.Common.NumEditbox.White")
					)
				]
			]
		];
	}
	
	if (AllowPickupsAttr.IsValid())
	{
		MenuSpace->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.Padding(0.0f, 0.0f, 0.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.WidthOverride(350)
				[
					SNew(STextBlock)
					.TextStyle(SUWindowsStyle::Get(), "UT.Common.NormalText")
					.Text(NSLOCTEXT("AUTBetrayalGameMode", "AllowPickups", "Allow Pickups"))
				]
			]
			+ SHorizontalBox::Slot()
			.Padding(20.0f, 0.0f, 0.0f, 10.0f)
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(300)
				[
					bCreateReadOnly ?
					StaticCastSharedRef<SWidget>(
						SNew(SCheckBox)
						.IsChecked(AllowPickupsAttr.ToSharedRef(), &TAttributePropertyBool_TEMP::GetAsCheckBox)
						.Style(SUWindowsStyle::Get(), "UT.Common.CheckBox")
						.ForegroundColor(FLinearColor::White)
						.Type(ESlateCheckBoxType::CheckBox)
					) :
					StaticCastSharedRef<SWidget>(
						SNew(SCheckBox)
						.IsChecked(AllowPickupsAttr.ToSharedRef(), &TAttributePropertyBool_TEMP::GetAsCheckBox)
						.OnCheckStateChanged(AllowPickupsAttr.ToSharedRef(), &TAttributePropertyBool_TEMP::SetFromCheckBox)
						.Style(SUWindowsStyle::Get(), "UT.Common.CheckBox")
						.ForegroundColor(FLinearColor::White)
						.Type(ESlateCheckBoxType::CheckBox)
					)
				]
			]
		];
	}
	

	// TODO: add menu widgets for changing additional game options
}

void AUTBetrayalGameMode::BuildPlayerInfo(AUTPlayerState* PlayerState, TSharedPtr<SUTTabWidget> TabWidget, TArray<TSharedPtr<TAttributeStat> >& StatList)
{
	Super::BuildPlayerInfo(PlayerState, TabWidget, StatList);

	// TODO: re-implement if player mesh preview isn't that big
	//BuildBetrayalInfo(PlayerState, TabWidget, StatList);
}

void AUTBetrayalGameMode::BuildScoreInfo(AUTPlayerState* PlayerState, TSharedPtr<class SUTTabWidget> TabWidget, TArray<TSharedPtr<TAttributeStat> >& StatList)
{
	// Copied from UTGameMode
	// /**
	TAttributeStat::StatValueTextFunc TwoDecimal = [](const AUTPlayerState* PS, const TAttributeStat* Stat) -> FText
	{
		return FText::FromString(FString::Printf(TEXT("%8.2f"), Stat->GetValue()));
	};

	TSharedPtr<SVerticalBox> LeftPane;
	TSharedPtr<SVerticalBox> RightPane;
	TSharedPtr<SHorizontalBox> HBox;
	BuildPaneHelper(HBox, LeftPane, RightPane);

	TabWidget->AddTab(NSLOCTEXT("AUTGameMode", "Score", "Score"), HBox);

	NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTGameMode", "Kills", "Kills"), MakeShareable(new TAttributeStat(PlayerState, NAME_None, [](const AUTPlayerState* PS, const TAttributeStat* Stat) -> float { return PS->Kills;	})), StatList);
	NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTGameMode", "Deaths", "Deaths"), MakeShareable(new TAttributeStat(PlayerState, NAME_None, [](const AUTPlayerState* PS, const TAttributeStat* Stat) -> float {	return PS->Deaths; })), StatList);
	NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTGameMode", "Suicides", "Suicides"), MakeShareable(new TAttributeStat(PlayerState, NAME_Suicides)), StatList);
	NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTGameMode", "ScorePM", "Score Per Minute"), MakeShareable(new TAttributeStat(PlayerState, NAME_None, [](const AUTPlayerState* PS, const TAttributeStat* Stat) -> float
	{
		return (PS->StartTime <  PS->GetWorld()->GameState->ElapsedTime) ? PS->Score * 60.f / (PS->GetWorld()->GameState->ElapsedTime - PS->StartTime) : 0.f;
	}, TwoDecimal)), StatList);
	NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTGameMode", "KDRatio", "K/D Ratio"), MakeShareable(new TAttributeStat(PlayerState, NAME_None, [](const AUTPlayerState* PS, const TAttributeStat* Stat) -> float
	{
		return (PS->Deaths > 0) ? float(PS->Kills) / PS->Deaths : 0.f;
	}, TwoDecimal)), StatList);

	NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTGameMode", "BeltPickups", "Shield Belt Pickups"), MakeShareable(new TAttributeStat(PlayerState, NAME_ShieldBeltCount)), StatList);
	NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTGameMode", "VestPickups", "Armor Vest Pickups"), MakeShareable(new TAttributeStat(PlayerState, NAME_ArmorVestCount)), StatList);
	NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTGameMode", "PadPickups", "Thigh Pad Pickups"), MakeShareable(new TAttributeStat(PlayerState, NAME_ArmorPadsCount)), StatList);
	NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTGameMode", "HelmetPickups", "Helmet Pickups"), MakeShareable(new TAttributeStat(PlayerState, NAME_HelmetCount)), StatList);
	NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTGameMode", "JumpBootJumps", "JumpBoot Jumps"), MakeShareable(new TAttributeStat(PlayerState, NAME_BootJumps)), StatList);

	LeftPane->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(40.0f)];
	NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTGameMode", "UDamagePickups", "UDamage Pickups"), MakeShareable(new TAttributeStat(PlayerState, NAME_UDamageCount)), StatList);
	NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTGameMode", "BerserkPickups", "Berserk Pickups"), MakeShareable(new TAttributeStat(PlayerState, NAME_BerserkCount)), StatList);
	NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTGameMode", "InvisibilityPickups", "Invisibility Pickups"), MakeShareable(new TAttributeStat(PlayerState, NAME_InvisibilityCount)), StatList);
	NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTGameMode", "KegPickups", "Keg Pickups"), MakeShareable(new TAttributeStat(PlayerState, NAME_KegCount)), StatList);

	TAttributeStat::StatValueTextFunc ToTime = [](const AUTPlayerState* PS, const TAttributeStat* Stat) -> FText
	{
		int32 Seconds = (int32)Stat->GetValue();
		int32 Mins = Seconds / 60;
		Seconds -= Mins * 60;
		return FText::FromString(FString::Printf(TEXT("%d:%02d"), Mins, Seconds));
	};

	RightPane->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(40.0f)];
	NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTGameMode", "UDamageControl", "UDamage Control"), MakeShareable(new TAttributeStat(PlayerState, NAME_UDamageTime, nullptr, ToTime)), StatList);
	NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTGameMode", "BerserkControl", "Berserk Control"), MakeShareable(new TAttributeStat(PlayerState, NAME_BerserkTime, nullptr, ToTime)), StatList);
	NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTGameMode", "InvisibilityControl", "Invisibility Control"), MakeShareable(new TAttributeStat(PlayerState, NAME_InvisibilityTime, nullptr, ToTime)), StatList);
	// **/

	LeftPane->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(40.0f)];
	RightPane->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(40.0f)];
	NewInfoHeader(LeftPane, DisplayName);
	NewInfoHeader(RightPane, DisplayName);
	AddBetrayalInfo(PlayerState, TabWidget, StatList, HBox, LeftPane, RightPane);
}

void AUTBetrayalGameMode::NewInfoHeader(TSharedPtr<SVerticalBox> VBox, FText DisplayName)
{
	VBox->AddSlot()
	.AutoHeight()
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		[
			SNew(STextBlock)
			.Text(DisplayName)
			.TextStyle(SUWindowsStyle::Get(), "UT.Common.NormalText")
			.ColorAndOpacity(FLinearColor::White)
		]
	];
}

void AUTBetrayalGameMode::BuildBetrayalInfo(AUTPlayerState* PlayerState, TSharedPtr<class SUTTabWidget> TabWidget, TArray<TSharedPtr<TAttributeStat> >& StatList)
{
	if (AUTBetrayalPlayerState* BPRI = Cast<AUTBetrayalPlayerState>(PlayerState))
	{
		TSharedPtr<SVerticalBox> LeftPane;
		TSharedPtr<SVerticalBox> RightPane;
		TSharedPtr<SHorizontalBox> HBox;
		BuildPaneHelper(HBox, LeftPane, RightPane);

		AddBetrayalInfo(PlayerState, TabWidget, StatList, HBox, LeftPane, RightPane);
	}
}

void AUTBetrayalGameMode::AddBetrayalInfo(AUTPlayerState* PlayerState, TSharedPtr<class SUTTabWidget> TabWidget, TArray<TSharedPtr<TAttributeStat> >& StatList, TSharedPtr<SHorizontalBox> HBox, TSharedPtr<SVerticalBox> LeftPane, TSharedPtr<SVerticalBox> RightPane)
{
	if (AUTBetrayalPlayerState* BPRI = Cast<AUTBetrayalPlayerState>(PlayerState))
	{
		TAttributeStatBetrayal::StatValueTextFuncBet TwoDecimal = [](const AUTBetrayalPlayerState* PS, const float Value) -> FText
		{
			return FText::FromString(FString::Printf(TEXT("%8.2f"), Value));
		};

		TAttributeStatBetrayal::StatValueTextFuncBet CondInt_Betrayal = [](const AUTBetrayalPlayerState* PS, const float Value) -> FText
		{
			return PS->BetrayalCount > 0 ? FText::FromString(FString::FromInt((int)Value)) : FText::FromString(FString(TEXT("-")));
		};

		TAttributeStatBetrayal::StatValueTextFuncBet CondInt_Betrayed = [](const AUTBetrayalPlayerState* PS, const float Value) -> FText
		{
			return PS->BetrayedCount > 0 ? FText::FromString(FString::FromInt((int)Value)) : FText::FromString(FString(TEXT("-")));
		};

		// TODO: temporarily removed due to workaround of adding stats to score tab instead of creating custom one
		//TSharedPtr<SVerticalBox> LeftPane;
		//TSharedPtr<SVerticalBox> RightPane;
		//TSharedPtr<SHorizontalBox> HBox;
		//BuildPaneHelper(HBox, LeftPane, RightPane);

		//TabWidget->AddTab(DisplayName, HBox);

		NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTBetrayalGameMode", "Betrayals", "Betrayals"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float { return PS->BetrayalCount; })), StatList);
		NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTBetrayalGameMode", "Victim", "Victim"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float { return PS->BetrayedCount; })), StatList);
		NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTBetrayalGameMode", "Retributions", "Retributions"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float { return PS->RetributionCount; })), StatList);
		NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTBetrayalGameMode", "Paybacks", "Paybacks"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float { return PS->PaybackCount; })), StatList);


		NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTBetrayalGameMode", "AverageBetrayalPot", "Average Betrayal Pot"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float
		{
			return PS->BetrayalCount > 0.f ? PS->BetrayalPot / PS->BetrayalCount : 0.f;
		}, TwoDecimal)), StatList);

		NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTBetrayalGameMode", "AverageVictimPot", "Average Victim Pot"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float
		{
			return PS->BetrayedCount > 0.f ? PS->BetrayedPot / PS->BetrayedCount : 0.f;
		}, TwoDecimal)), StatList);

		NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTBetrayalGameMode", "BetrayalLowestPot", "Lowest Betrayal Pot"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float { return PS->BetrayalLowestPot; }, CondInt_Betrayal)), StatList);
		NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTBetrayalGameMode", "BetrayalHighestPot", "Highest Betrayal Pot"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float { return PS->BetrayalHighestPot; }, CondInt_Betrayal)), StatList);

		NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTBetrayalGameMode", "VictimLowestPot", "Lowest Victim Pot"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float { return PS->BetrayedLowestPot; }, CondInt_Betrayed)), StatList);
		NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTBetrayalGameMode", "VictimHighestPot", "Highest Victim Pot"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float { return PS->BetrayedHighestPot; }, CondInt_Betrayed)), StatList);

		// this method is called from default object context, can't rely on instanced properties
		// check GameState whether the Game has ended, or rely on the owner of the PlayerState
		APlayerController* PC = Cast<APlayerController>(PlayerState->GetOwner());
		AUTGameState* GS = PlayerState->GetWorld() ? PlayerState->GetWorld()->GetGameState<AUTGameState>() : NULL;
		if ((PC != NULL && PC->IsLocalPlayerController()) || (GS && (GS->WinnerPlayerState || GS->WinningTeam)))
		{
			LeftPane->AddSlot()[SNew(SSpacer).Size(FVector2D(0.0f, 20.0f))];
			NewPlayerInfoLine(LeftPane, NSLOCTEXT("AUTBetrayalGameMode", "Nemesis", "Nemesis"), MakeShareable(new TAttributeStatBetrayal(BPRI, nullptr, [](const AUTBetrayalPlayerState* PS, const float Value) -> FText
			{
				return PS->CurrentNemesis.IsEmpty() ? FText::FromString(FString(TEXT("-"))) : FText::FromString(PS->CurrentNemesis);
			})), StatList);
		}
	}
}

#endif

FText AUTBetrayalGameMode::BuildServerRules(AUTGameState* GameState)
{
	FFormatNamedArguments Args;
	Args.Add("Rules", Super::BuildServerRules(GameState));
	Args.Add("RogueValue", FText::AsNumber(RogueValue));
	Args.Add("RogueTimePenalty", FText::AsNumber(RogueTimePenalty));
	Args.Add("AllowPickups", bAllowPickups ? NSLOCTEXT("AUTBetrayalGameMode", "General", "True") : NSLOCTEXT("AUTBetrayalGameMode", "General", "False"));

	return FText::Format(NSLOCTEXT("UTBetrayalGameMode", "GameRules", "{Rules}  Rogue Value: {RogueValue}  Rogue time penalty: {RogueTimePenalty}s  Allow Pickups: {AllowPickups}"), Args);
}

void AUTBetrayalGameMode::BuildServerResponseRules(FString& OutRules)
{
	// TODO: proper order
	OutRules += FString::Printf(TEXT("Rogue Value\t%i\t"), RogueValue);
	OutRules += FString::Printf(TEXT("Rogue time penalty\t%i\t"), RogueTimePenalty);
	OutRules += FString::Printf(TEXT("Allow Pickups\t%s\t"), bAllowPickups ? TEXT("True") : TEXT("False"));

	Super::BuildServerResponseRules(OutRules);
}

void AUTBetrayalGameMode::CreateGameURLOptions(TArray<TSharedPtr<TAttributePropertyBase>>& MenuProps)
{
	Super::CreateGameURLOptions(MenuProps);

	// Remove ForceRespawn option
	for (int32 i = MenuProps.Num() - 1; i >= 0; i--)
	{
		if (MenuProps[i].IsValid() && MenuProps[i]->GetURLKey().Equals(TEXT("ForceRespawn"), ESearchCase::IgnoreCase))
		{
			MenuProps.RemoveAt(i);
		}
	}

	MenuProps.Add(MakeShareable(new TAttributeProperty<int32>(this, &RogueValue, TEXT("RogueValue"))));
	MenuProps.Add(MakeShareable(new TAttributeProperty<int32>(this, &RogueTimePenalty, TEXT("RogueTimePenalty"))));
	MenuProps.Add(MakeShareable(new TAttributePropertyBool_TEMP(this, &bAllowPickups, TEXT("AllowPickups"))));
}

void AUTBetrayalGameMode::GetGameURLOptions(const TArray<TSharedPtr<TAttributePropertyBase>>& MenuProps, TArray<FString>& OptionsList, int32& DesiredPlayerCount)
{
	Super::GetGameURLOptions(MenuProps, OptionsList, DesiredPlayerCount);

	// TODO: remove as already handled in CreateGameURLOptions
	//// Remove ForceRespawn option
	//for (int32 i = 0; i < OptionsList.Num(); i++)
	//{
	//	if (OptionsList[i].StartsWith(TEXT("ForceRespawn")) || OptionsList[i].StartsWith(TEXT("bForceRespawn")))
	//	{
	//		OptionsList.RemoveAt(i);
	//		i--;
	//	}
	//}

	// TODO: parameterize additional game options (like allowing Boots)
}

void AUTBetrayalGameMode::BETKillbot(const FString& NameOrUIDStr)
{
	BETServerKillbot(NameOrUIDStr);
}

bool AUTBetrayalGameMode::BETServerKillbot_Validate(const FString& NameOrUIDStr) { return true; }
void AUTBetrayalGameMode::BETServerKillbot_Implementation(const FString& NameOrUIDStr)
{
#if WITH_EDITOR

	AUTGameState* GS = GetWorld()->GetGameState<AUTGameState>();
	AUTGameMode* GM = GetWorld()->GetAuthGameMode<AUTGameMode>();
	if (GS != NULL && GM != NULL)
	{
		for (int32 i = 0; i < GS->PlayerArray.Num(); i++)
		{
			if (GS->PlayerArray[i] == NULL || GS->PlayerArray[i]->IsPendingKill())
			{
				// skip pending or already destroyed players
				continue;
			}

			if ((GS->PlayerArray[i]->PlayerName.ToLower() == NameOrUIDStr.ToLower()) ||
				(GS->PlayerArray[i]->UniqueId.ToString() == NameOrUIDStr))
			{
				if (AUTBot* Bot = Cast<AUTBot>(GS->PlayerArray[i]->GetOwner()))
				{
					uint8 BotCount = GM->NumBots;
					Bot->Destroy();

					if (AUTGameMode* UTGM = GetWorld()->GetAuthGameMode<AUTGameMode>())
					{
						UTGM->SetBotCount(FMath::Max<uint8>(0, BotCount - 1));
					}
				}
			}
		}
	}
#endif
}
