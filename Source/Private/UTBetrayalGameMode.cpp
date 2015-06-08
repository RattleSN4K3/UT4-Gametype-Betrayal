#include "UTBetrayal.h"
#include "UTBetrayalCharacter.h"
#include "UTBetrayalGameState.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalHUD.h"
#include "UTBetrayalMessage.h"
#include "UTBetrayalBot.h"
#include "UTBetrayalCharacterPostRenderer.h"

#include "UTBot.h"
#include "UTFirstBloodMessage.h"
#include "UTMutator_WeaponArena.h"
#include "UTMutator_WeaponReplacement.h"

#include "UTBetrayalGameMode.h"

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

	// Workaround for bot decision to betray team members
	// TODO: allow to have PickNewEnemy in Squad/BotDecisionComponent
	//BotClass = AUTBetrayalBot::StaticClass();

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
	}

	Super::Logout(Exiting);
}

void AUTBetrayalGameMode::ScoreKill(AController* Killer, AController* Other, APawn* KilledPawn, TSubclassOf<UDamageType> DamageType)
{
	// TODO: Improve ScoreKill, call parent method and apply Betrayal logic separately

	if ((Killer == Other) || (Killer == NULL))
	{
		// If it's a suicide, subtract a kill from the player...

		if (Other != NULL && Other->PlayerState != NULL && Cast<AUTPlayerState>(Other->PlayerState) != NULL)
		{
			Cast<AUTPlayerState>(Other->PlayerState)->AdjustScore(-1);
			Cast<AUTPlayerState>(Other->PlayerState)->IncrementKills(DamageType, false);
		}
	}
	else
	{
		AUTBetrayalPlayerState* KillerPRI = Cast<AUTBetrayalPlayerState>(Killer->PlayerState);
		AUTBetrayalPlayerState* OtherPRI = Cast<AUTBetrayalPlayerState>(Other->PlayerState);
		if (KillerPRI != NULL && OtherPRI != NULL)
		{
			KillerPRI->AdjustScore(OtherPRI->ScoreValueFor(KillerPRI));
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

			KillerPRI->IncrementKills(DamageType, true); 
			if (KillerPRI->CurrentTeam != NULL)
			{
				KillerPRI->CurrentTeam->TeamPot++;

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

								UE_LOG(Betrayal, Verbose, TEXT("%s betrayal value %d  vs. %d"), *KillerPRI->PlayerName, BetrayalValue, BetrayalRandomness);
								if ((BetrayalValue > BetrayalRandomness) && (FMath::FRand() < 0.2))
								{
									B->bBetrayTeam = true;
								}
							}
						}
					}
				}
			}

			FindAndMarkHighScorer();
			CheckScore(KillerPRI);
		}

		if (!bFirstBloodOccurred)
		{
			BroadcastLocalized(this, UUTFirstBloodMessage::StaticClass(), 0, KillerPRI, NULL, NULL);
			bFirstBloodOccurred = true;
		}
	}

	if (BaseMutator != NULL)
	{
		BaseMutator->ScoreKill(Killer, Other, DamageType);
	}
}

// Workaround for PostRender routed to HUD
// TODO: FIXME: Route PostRender to UTHUD class

void AUTBetrayalGameMode::SetPlayerDefaults(APawn* PlayerPawn)
{
	// FIXME: workaround for JumpBoots calling SetPlayerDefaults
	// TODO: Pull Request/Forum thread update to get this fixed
	if (!AlreadySpawnedPlayers.Contains(PlayerPawn))
	{
		AlreadySpawnedPlayers.Add(PlayerPawn);
		AlreadySpawnedPlayers.Remove(NULL);

		FActorSpawnParameters Params;
		Params.bNoCollisionFail = true;
		Params.Owner = PlayerPawn;
		GetWorld()->SpawnActor<AUTBetrayalCharacterPostRenderer>(AUTBetrayalCharacterPostRenderer::StaticClass(), Params);
	}

	Super::SetPlayerDefaults(PlayerPawn);
}

// END Workaround for PostRender routed to HUD


// Workaround for spawning custom bot
// TODO: FIXME: Set bot class as class field (Pull Request?)

// COPIED FROM UTGameMode
AUTBot* AUTBetrayalGameMode::AddBot(uint8 TeamNum)
{
	AUTBot* NewBot = GetWorld()->SpawnActor<AUTBot>(AUTBetrayalBot::StaticClass());
	if (NewBot != NULL)
	{
		if (BotConfig == NULL)
		{
			BotConfig = NewObject<UUTBotConfig>(this);
		}
		// pick bot character
		if (BotConfig->BotCharacters.Num() == 0)
		{
			UE_LOG(Betrayal, Warning, TEXT("AddBot(): No BotCharacters defined"));
			static int32 NameCount = 0;
			NewBot->PlayerState->SetPlayerName(FString(TEXT("TestBot")) + ((NameCount > 0) ? FString::Printf(TEXT("_%i"), NameCount) : TEXT("")));
			NameCount++;
		}
		else
		{
			int32 NumEligible = 0;
			FBotCharacter* BestChar = NULL;
			uint8 BestSelectCount = MAX_uint8;
			for (FBotCharacter& Character : BotConfig->BotCharacters)
			{
				if (Character.SelectCount < BestSelectCount)
				{
					NumEligible = 1;
					BestChar = &Character;
					BestSelectCount = Character.SelectCount;
				}
				else if (Character.SelectCount == BestSelectCount)
				{
					NumEligible++;
					if (FMath::FRand() < 1.0f / float(NumEligible))
					{
						BestChar = &Character;
					}
				}
			}
			BestChar->SelectCount++;
			NewBot->Personality = *BestChar;
			NewBot->PlayerState->SetPlayerName(BestChar->PlayerName);
		}
		AUTPlayerState* PS = Cast<AUTPlayerState>(NewBot->PlayerState);
		if (PS != NULL)
		{
			PS->bReadyToPlay = true;
		}

		NewBot->InitializeSkill(GameDifficulty);
		NumBots++;
		ChangeTeam(NewBot, TeamNum);
		GenericPlayerInitialization(NewBot);
	}
	return NewBot;
}

// COPIED FROM UTGameMode
AUTBot* AUTBetrayalGameMode::AddNamedBot(const FString& BotName, uint8 TeamNum)
{
	if (BotConfig == NULL)
	{
		BotConfig = NewObject<UUTBotConfig>(this);
	}
	FBotCharacter* TheChar = NULL;
	for (FBotCharacter& Character : BotConfig->BotCharacters)
	{
		if (Character.PlayerName == BotName)
		{
			TheChar = &Character;
			break;
		}
	}

	if (TheChar == NULL)
	{
		UE_LOG(Betrayal, Error, TEXT("Character data for bot '%s' not found"), *BotName);
		return NULL;
	}
	else
	{
		AUTBot* NewBot = GetWorld()->SpawnActor<AUTBot>(AUTBetrayalBot::StaticClass());
		if (NewBot != NULL)
		{
			TheChar->SelectCount++;
			NewBot->Personality = *TheChar;
			NewBot->PlayerState->SetPlayerName(TheChar->PlayerName);

			AUTPlayerState* PS = Cast<AUTPlayerState>(NewBot->PlayerState);
			if (PS != NULL)
			{
				PS->bReadyToPlay = true;
			}

			NewBot->InitializeSkill(GameDifficulty);
			NumBots++;
			ChangeTeam(NewBot, TeamNum);
			GenericPlayerInitialization(NewBot);
		}

		return NewBot;
	}
}

// END Workaround for spawning custom bot