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

#include "Private/Slate/Widgets/SUTTabWidget.h"
#include "SNumericEntryBox.h"

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

		FConstructorStatics()
			: PawnClass(TEXT("Class'/UTBetrayal/DefaultCharacter_Betrayal.DefaultCharacter_Betrayal_C'"))
			, InstaGibRifleClass(TEXT("Class'/UTBetrayal/BP_InstagibRifle_Betrayal.BP_InstagibRifle_Betrayal_C'")) // Class'/Game/RestrictedAssets/Weapons/ShockRifle/BP_InstagibRifle.BP_InstagibRifle_C'

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

	BetrayingSound = ConstructorStatics.BetrayingSoundAsset.Object;
	BetrayedSound = ConstructorStatics.BetrayedSoundAsset.Object;
	JoinTeamSound = ConstructorStatics.JoinTeamSoundAsset.Object;

	bForceRespawn = true;
	ForceRespawnTime = RespawnWaitTime;

	RogueValue = 6;

	// TEMP: prevent spawning "No class"
	bNoDefaultLeaderHat = true;
}

void AUTBetrayalGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
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
}

void AUTBetrayalGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorldTimerManager().IsTimerActive(TimerHandle_MaybeStartTeam))
	{
		GetWorldTimerManager().SetTimer(TimerHandle_MaybeStartTeam, this, &AUTBetrayalGameMode::MaybeStartTeam, 1.0f, true);
	}
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

	InstigatorPRI->UpdateNemesis(HitPRI);
	InstigatorPRI->UpdateTeam(NULL);

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

	Super::ScoreKill_Implementation(Killer, Other, KilledPawn, DamageType);
}

void AUTBetrayalGameMode::CreateConfigWidgets(TSharedPtr<class SVerticalBox> MenuSpace, bool bCreateReadOnly, TArray< TSharedPtr<TAttributePropertyBase> >& ConfigProps)
{
	// TEMP: OVERRIDEN IN ORDER TO PREVENT CRASH
	//Super::CreateConfigWidgets(MenuSpace, bCreateReadOnly, ConfigProps);


	// TODO: REMOVE: TEMP: Copied from UTGameMode::CreateConfigWidgets
	//       this will prevent a crash as ForceRespawn is removed. Causing accessing null pointer
	// // *
	CreateGameURLOptions(ConfigProps);

	TSharedPtr< TAttributeProperty<int32> > TimeLimitAttr = StaticCastSharedPtr<TAttributeProperty<int32>>(FindGameURLOption(ConfigProps, TEXT("TimeLimit")));
	TSharedPtr< TAttributeProperty<int32> > GoalScoreAttr = StaticCastSharedPtr<TAttributeProperty<int32>>(FindGameURLOption(ConfigProps, TEXT("GoalScore")));
	TSharedPtr< TAttributeProperty<int32> > CombatantsAttr = StaticCastSharedPtr<TAttributeProperty<int32>>(FindGameURLOption(ConfigProps, TEXT("BotFill")));

	if (CombatantsAttr.IsValid())
	{
		MenuSpace->AddSlot()
		.AutoHeight()
		.VAlign(VAlign_Top)
		.Padding(0.0f, 0.0f, 0.0f, 5.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(0.0f, 5.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(350)
				[
					SNew(STextBlock)
					.TextStyle(SUWindowsStyle::Get(), "UT.Common.NormalText")
					.Text(NSLOCTEXT("UTGameMode", "NumCombatants", "Number of Combatants"))
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(20.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(300)
				[
					bCreateReadOnly ?
					StaticCastSharedRef<SWidget>(
					SNew(STextBlock)
					.TextStyle(SUWindowsStyle::Get(), "UT.Common.ButtonText.White")
					.Text(CombatantsAttr.ToSharedRef(), &TAttributeProperty<int32>::GetAsText)
					) :
					StaticCastSharedRef<SWidget>(
					SNew(SNumericEntryBox<int32>)
					.Value(CombatantsAttr.ToSharedRef(), &TAttributeProperty<int32>::GetOptional)
					.OnValueChanged(CombatantsAttr.ToSharedRef(), &TAttributeProperty<int32>::Set)
					.AllowSpin(true)
					.Delta(1)
					.MinValue(1)
					.MaxValue(32)
					.MinSliderValue(1)
					.MaxSliderValue(32)
					.EditableTextBoxStyle(SUWindowsStyle::Get(), "UT.Common.NumEditbox.White")

					)
				]
			]
		];
	}

	if (GoalScoreAttr.IsValid())
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
					.Text(NSLOCTEXT("UTGameMode", "GoalScore", "Goal Score"))
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
					.Text(GoalScoreAttr.ToSharedRef(), &TAttributeProperty<int32>::GetAsText)
					) :
					StaticCastSharedRef<SWidget>(
					SNew(SNumericEntryBox<int32>)
					.Value(GoalScoreAttr.ToSharedRef(), &TAttributeProperty<int32>::GetOptional)
					.OnValueChanged(GoalScoreAttr.ToSharedRef(), &TAttributeProperty<int32>::Set)
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

	if (TimeLimitAttr.IsValid())
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
					.Text(NSLOCTEXT("UTGameMode", "TimeLimit", "Time Limit"))
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
					.Text(TimeLimitAttr.ToSharedRef(), &TAttributeProperty<int32>::GetAsText)
					) :
					StaticCastSharedRef<SWidget>(
					SNew(SNumericEntryBox<int32>)
					.Value(TimeLimitAttr.ToSharedRef(), &TAttributeProperty<int32>::GetOptional)
					.OnValueChanged(TimeLimitAttr.ToSharedRef(), &TAttributeProperty<int32>::Set)
					.AllowSpin(true)
					.Delta(1)
					.MinValue(0)
					.MaxValue(999)
					.MinSliderValue(0)
					.MaxSliderValue(60)
					.EditableTextBoxStyle(SUWindowsStyle::Get(), "UT.Common.NumEditbox.White")
					)
				]
			]
		];
	}
	// * //

	// TODO: add menu widgets for changing additional game options
}

#if !UE_SERVER

void AUTBetrayalGameMode::BuildPlayerInfo(AUTPlayerState* PlayerState, TSharedPtr<SUTTabWidget> TabWidget, TArray<TSharedPtr<TAttributeStat> >& StatList)
{
	Super::BuildPlayerInfo(PlayerState, TabWidget, StatList);

	// TODO: re-implement if player mesh preview isn't that big
	//BuildBetrayalInfo(PlayerState, TabWidget, StatList);
}

void AUTBetrayalGameMode::BuildScoreInfo(AUTPlayerState* PlayerState, TSharedPtr<class SUTTabWidget> TabWidget, TArray<TSharedPtr<TAttributeStat> >& StatList)
{
	// COPIED from StatNames
	// //*
	static FName NAME_AttackerScore(TEXT("AttackerScore"));
	static FName NAME_DefenderScore(TEXT("DefenderScore"));
	static FName NAME_SupporterScore(TEXT("SupporterScore"));
	static FName NAME_TeamKills(TEXT("TeamKills"));

	static FName NAME_UDamageTime(TEXT("UDamageTime"));
	static FName NAME_BerserkTime(TEXT("BerserkTime"));
	static FName NAME_InvisibilityTime(TEXT("InvisibilityTime"));
	static FName NAME_UDamageCount(TEXT("UDamageCount"));
	static FName NAME_BerserkCount(TEXT("BerserkCount"));
	static FName NAME_InvisibilityCount(TEXT("InvisibilityCount"));
	static FName NAME_BootJumps(TEXT("BootJumps"));
	static FName NAME_ShieldBeltCount(TEXT("ShieldBeltCount"));
	static FName NAME_ArmorVestCount(TEXT("ArmorVestCount"));
	static FName NAME_ArmorPadsCount(TEXT("ArmorPadsCount"));
	static FName NAME_HelmetCount(TEXT("HelmetCount"));
	static FName NAME_KegCount(TEXT("KegCount"));

	static FName NAME_SkillRating(TEXT("SkillRating"));
	static FName NAME_TDMSkillRating(TEXT("TDMSkillRating"));
	static FName NAME_DMSkillRating(TEXT("DMSkillRating"));
	static FName NAME_CTFSkillRating(TEXT("CTFSkillRating"));

	static FName NAME_SkillRatingSamples(TEXT("SkillRatingSamples"));
	static FName NAME_TDMSkillRatingSamples(TEXT("TDMSkillRatingSamples"));
	static FName NAME_DMSkillRatingSamples(TEXT("DMSkillRatingSamples"));
	static FName NAME_CTFSkillRatingSamples(TEXT("CTFSkillRatingSamples"));

	static FName NAME_MatchesPlayed(TEXT("MatchesPlayed"));
	static FName NAME_MatchesQuit(TEXT("MatchesQuit"));
	static FName NAME_TimePlayed(TEXT("TimePlayed"));
	static FName NAME_Wins(TEXT("Wins"));
	static FName NAME_Losses(TEXT("Losses"));
	static FName NAME_Kills(TEXT("Kills"));
	static FName NAME_Deaths(TEXT("Deaths"));
	static FName NAME_Suicides(TEXT("Suicides"));

	static FName NAME_MultiKillLevel0(TEXT("MultiKillLevel0"));
	static FName NAME_MultiKillLevel1(TEXT("MultiKillLevel1"));
	static FName NAME_MultiKillLevel2(TEXT("MultiKillLevel2"));
	static FName NAME_MultiKillLevel3(TEXT("MultiKillLevel3"));

	static FName NAME_SpreeKillLevel0(TEXT("SpreeKillLevel0"));
	static FName NAME_SpreeKillLevel1(TEXT("SpreeKillLevel1"));
	static FName NAME_SpreeKillLevel2(TEXT("SpreeKillLevel2"));
	static FName NAME_SpreeKillLevel3(TEXT("SpreeKillLevel3"));
	static FName NAME_SpreeKillLevel4(TEXT("SpreeKillLevel4"));

	static FName NAME_ImpactHammerKills(TEXT("ImpactHammerKills"));
	static FName NAME_EnforcerKills(TEXT("EnforcerKills"));
	static FName NAME_BioRifleKills(TEXT("BioRifleKills"));
	static FName NAME_ShockBeamKills(TEXT("ShockBeamKills"));
	static FName NAME_ShockCoreKills(TEXT("ShockCoreKills"));
	static FName NAME_ShockComboKills(TEXT("ShockComboKills"));
	static FName NAME_LinkKills(TEXT("LinkKills"));
	static FName NAME_LinkBeamKills(TEXT("LinkBeamKills"));
	static FName NAME_MinigunKills(TEXT("MinigunKills"));
	static FName NAME_MinigunShardKills(TEXT("MinigunShardKills"));
	static FName NAME_FlakShardKills(TEXT("FlakShardKills"));
	static FName NAME_FlakShellKills(TEXT("FlakShellKills"));
	static FName NAME_RocketKills(TEXT("RocketKills"));
	static FName NAME_SniperKills(TEXT("SniperKills"));
	static FName NAME_SniperHeadshotKills(TEXT("SniperHeadshotKills"));
	static FName NAME_RedeemerKills(TEXT("RedeemerKills"));
	static FName NAME_InstagibKills(TEXT("InstagibKills"));
	static FName NAME_TelefragKills(TEXT("TelefragKills"));

	static FName NAME_ImpactHammerDeaths(TEXT("ImpactHammerDeaths"));
	static FName NAME_EnforcerDeaths(TEXT("EnforcerDeaths"));
	static FName NAME_BioRifleDeaths(TEXT("BioRifleDeaths"));
	static FName NAME_ShockBeamDeaths(TEXT("ShockBeamDeaths"));
	static FName NAME_ShockCoreDeaths(TEXT("ShockCoreDeaths"));
	static FName NAME_ShockComboDeaths(TEXT("ShockComboDeaths"));
	static FName NAME_LinkDeaths(TEXT("LinkDeaths"));
	static FName NAME_LinkBeamDeaths(TEXT("LinkBeamDeaths"));
	static FName NAME_MinigunDeaths(TEXT("MinigunDeaths"));
	static FName NAME_MinigunShardDeaths(TEXT("MinigunShardDeaths"));
	static FName NAME_FlakShardDeaths(TEXT("FlakShardDeaths"));
	static FName NAME_FlakShellDeaths(TEXT("FlakShellDeaths"));
	static FName NAME_RocketDeaths(TEXT("RocketDeaths"));
	static FName NAME_SniperDeaths(TEXT("SniperDeaths"));
	static FName NAME_SniperHeadshotDeaths(TEXT("SniperHeadshotDeaths"));
	static FName NAME_RedeemerDeaths(TEXT("RedeemerDeaths"));
	static FName NAME_InstagibDeaths(TEXT("InstagibDeaths"));
	static FName NAME_TelefragDeaths(TEXT("TelefragDeaths"));

	static FName NAME_PlayerXP(TEXT("PlayerXP"));

	static FName NAME_BestShockCombo(TEXT("BestShockCombo"));
	static FName NAME_AmazingCombos(TEXT("AmazingCombos"));
	static FName NAME_AirRox(TEXT("AirRox"));
	static FName NAME_FlakShreds(TEXT("FlakShreds"));
	static FName NAME_AirSnot(TEXT("AirSnot"));

	static FName NAME_RunDist(TEXT("RunDist"));
	static FName NAME_SprintDist(TEXT("SprintDist"));
	static FName NAME_InAirDist(TEXT("InAirDist"));
	static FName NAME_SwimDist(TEXT("SwimDist"));
	static FName NAME_TranslocDist(TEXT("TranslocDist"));
	static FName NAME_NumDodges(TEXT("NumDodges"));
	static FName NAME_NumWallDodges(TEXT("NumWallDodges"));
	static FName NAME_NumJumps(TEXT("NumJumps"));
	static FName NAME_NumLiftJumps(TEXT("NumLiftJumps"));
	static FName NAME_NumFloorSlides(TEXT("NumFloorSlides"));
	static FName NAME_NumWallRuns(TEXT("NumWallRuns"));
	static FName NAME_NumImpactJumps(TEXT("NumImpactJumps"));
	static FName NAME_NumRocketJumps(TEXT("NumRocketJumps"));
	static FName NAME_SlideDist(TEXT("SlideDist"));
	static FName NAME_WallRunDist(TEXT("WallRunDist"));

	static FName NAME_EnforcerShots(TEXT("EnforcerShots"));
	static FName NAME_BioRifleShots(TEXT("BioRifleShots"));
	static FName NAME_ShockRifleShots(TEXT("ShockRifleShots"));
	static FName NAME_LinkShots(TEXT("LinkShots"));
	static FName NAME_MinigunShots(TEXT("MinigunShots"));
	static FName NAME_FlakShots(TEXT("FlakShots"));
	static FName NAME_RocketShots(TEXT("RocketShots"));
	static FName NAME_SniperShots(TEXT("SniperShots"));
	static FName NAME_RedeemerShots(TEXT("RedeemerShots"));
	static FName NAME_InstagibShots(TEXT("InstagibShots"));

	static FName NAME_EnforcerHits(TEXT("EnforcerHits"));
	static FName NAME_BioRifleHits(TEXT("BioRifleHits"));
	static FName NAME_ShockRifleHits(TEXT("ShockRifleHits"));
	static FName NAME_LinkHits(TEXT("LinkHits"));
	static FName NAME_MinigunHits(TEXT("MinigunHits"));
	static FName NAME_FlakHits(TEXT("FlakHits"));
	static FName NAME_RocketHits(TEXT("RocketHits"));
	static FName NAME_SniperHits(TEXT("SniperHits"));
	static FName NAME_RedeemerHits(TEXT("RedeemerHits"));
	static FName NAME_InstagibHits(TEXT("InstagibHits"));
	// *//

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


		NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTBetrayalGameMode", "AvergageBetrayalPot", "Average Betrayal Pot"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float
		{
			return PS->BetrayalCount > 0.f ? PS->BetrayalPot / PS->BetrayalCount : 0.f;
		}, TwoDecimal)), StatList);

		NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTBetrayalGameMode", "AvergageVictimPot", "Average Victim Pot"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float
		{
			return PS->BetrayedCount > 0.f ? PS->BetrayedPot / PS->BetrayedCount : 0.f;
		}, TwoDecimal)), StatList);

		NewPlayerInfoLine(RightPane, NSLOCTEXT("AUTBetrayalGameMode", "HighestPot", "Highest Pot"), MakeShareable(new TAttributeStatBetrayal(BPRI, [](const AUTBetrayalPlayerState* PS) -> float { return PS->HighestPot; })), StatList);

		APlayerController* PC = Cast<APlayerController>(PlayerState->GetOwner());
		if (PC != NULL && PC->IsLocalPlayerController())
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
	Args.Add("RogueTimePenalty", FText::AsNumber(AUTBetrayalPlayerState::StaticClass()->GetDefaultObject<AUTBetrayalPlayerState>()->RogueTimePenalty));

	return FText::Format(NSLOCTEXT("UTBetrayalGameMode", "GameRules", "{Rules}  Rogue Value: {RogueValue}  Rogue time penalty: {RogueTimePenalty} s"), Args);
}

void AUTBetrayalGameMode::BuildServerResponseRules(FString& OutRules)
{
	// TODO: proper order
	OutRules += FString::Printf(TEXT("Rogue Value\t%i\t"), RogueValue);
	OutRules += FString::Printf(TEXT("Rogue time penalty\t%i\t"), AUTBetrayalPlayerState::StaticClass()->GetDefaultObject<AUTBetrayalPlayerState>()->RogueTimePenalty);

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

	// TODO: parameterize additional game options (like allowing Boots, Rogue value, Rogue penalty)
}
