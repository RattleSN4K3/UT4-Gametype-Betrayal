#include "UTBetrayal.h"
#include "UTBetrayalGameMode.h"
#include "UTPlayerController.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalGameState.h"

#include "UTBetrayalCharacterPostRenderer.h"

AUTBetrayalPlayerState::AUTBetrayalPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<USoundCue> RogueFadingSound;

		FConstructorStatics()
			: RogueFadingSound(TEXT("SoundCue'/UTBetrayal/Sounds/UT3ServerSignOut_Cue.UT3ServerSignOut_Cue'"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;

	RemainingRogueTime = -1000;
	RogueTimePenalty = 30;

	BetrayalLowestPot = -1;
	BetrayedLowestPot = -1;

	RogueFadingSound = ConstructorStatics.RogueFadingSound.Object;

#if WITH_EDITOR || UE_BUILD_DEBUG || BETRAYAL_DEBUG
	bDebug = true;
#endif
}

void AUTBetrayalPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AUTBetrayalPlayerState, CurrentTeam);
	DOREPLIFETIME(AUTBetrayalPlayerState, Betrayer);
	DOREPLIFETIME(AUTBetrayalPlayerState, BetrayalCount);
	DOREPLIFETIME(AUTBetrayalPlayerState, bIsRogue);
	DOREPLIFETIME(AUTBetrayalPlayerState, RemainingRogueTime);

	// stats replication
	DOREPLIFETIME(AUTBetrayalPlayerState, BetrayalPot);
	DOREPLIFETIME(AUTBetrayalPlayerState, BetrayedCount);
	DOREPLIFETIME(AUTBetrayalPlayerState, BetrayedPot);
	DOREPLIFETIME(AUTBetrayalPlayerState, RetributionCount);
	DOREPLIFETIME(AUTBetrayalPlayerState, PaybackCount);
	DOREPLIFETIME(AUTBetrayalPlayerState, BetrayalLowestPot);
	DOREPLIFETIME(AUTBetrayalPlayerState, BetrayalHighestPot);
	DOREPLIFETIME(AUTBetrayalPlayerState, BetrayedLowestPot);
	DOREPLIFETIME(AUTBetrayalPlayerState, BetrayedHighestPot);

	// Only to owner as this info shouldn't be revealed to others
	// TODO: Replicate at end of the match to everyone?
	DOREPLIFETIME_CONDITION(AUTBetrayalPlayerState, CurrentNemesis, COND_OwnerOnly);
}

void AUTBetrayalPlayerState::Reset()
{
	Super::Reset();

	RemainingRogueTime = 0;
	bIsRogue = false;
	CurrentTeam = NULL;
	Betrayer = NULL;
	BetrayalCount = 0;

	BetrayedTeam = NULL;

	BetrayalPot = 0;
	BetrayedCount = 0;
	BetrayedPot = 0;
	RetributionCount = 0;
	PaybackCount = 0;
	BetrayalLowestPot = -1;
	BetrayalHighestPot = 0;
	BetrayedLowestPot = -1;
	BetrayedHighestPot = 0;
}

void AUTBetrayalPlayerState::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR || UE_BUILD_DEBUG || BETRAYAL_DEBUG
	if (bDebug)
	{
		// set the betrayal count to random values for HUD test
		if (FMath::FRand() > 0.8)
			BetrayalCount = FMath::RandRange(100, 99999);
		else
			BetrayalCount = FMath::RandRange(4, 99);
	}
#endif
}

void AUTBetrayalPlayerState::SetRogueTimer()
{
	RemainingRogueTime = RogueTimePenalty;
	ForceNetUpdate();
	bIsRogue = true;
	GetWorldTimerManager().SetTimer(TimerHandle_RogueTimer, this, &AUTBetrayalPlayerState::RogueTimer, 1.0f, true);
}

void AUTBetrayalPlayerState::RogueTimer()
{
	AUTPlayerController* PC = Cast<AUTPlayerController>(GetOwner());

	RemainingRogueTime--;
	if (RemainingRogueTime < 0)
	{
		RogueExpired();
		if (PC != NULL)
		{
			AUTBetrayalGameMode* Game = GetWorld()->GetAuthGameMode<AUTBetrayalGameMode>();
			if (Game != NULL)
			{
				PC->ClientReceiveLocalizedMessage(Game->AnnouncerMessageClass, 5);
			}
		}
	}
	else if (RemainingRogueTime < 3 && PC != NULL)
	{
		PC->ClientPlaySound(RogueFadingSound);
	}
}

void AUTBetrayalPlayerState::RogueExpired()
{
	RemainingRogueTime = -100.0;
	bIsRogue = false;
	ForceNetUpdate();
	GetWorldTimerManager().ClearTimer(TimerHandle_RogueTimer);

	AGameState* GameState = GetWorld()->GetGameState();
	if (GameState != NULL)
	{
		for (APlayerState* PS : GameState->PlayerArray)
		{
			AUTBetrayalPlayerState* PRI = Cast<AUTBetrayalPlayerState>(PS);
			if ((PRI != NULL) && (PRI->Betrayer == this))
			{
				PRI->Betrayer = NULL;
			}
		}
	}
}

int32 AUTBetrayalPlayerState::ScoreValueFor(AUTBetrayalPlayerState* OtherPRI)
{
	int32 ScoreValue = 1 + FMath::Clamp<int32>((Score - OtherPRI->Score) / 4, 0, 9);
	if (bIsRogue && (OtherPRI->Betrayer == this))
	{
		ScoreValue += AUTBetrayalGameMode::StaticClass()->GetDefaultObject<AUTBetrayalGameMode>()->RogueValue;
	}
	return ScoreValue;
}

float AUTBetrayalPlayerState::GetTrustWorthiness()
{
	// TODO: Add support for FamilyInfo
	/*if (!bHasSetTrust && CharacterData.FamilyID != "" && CharacterData.FamilyID != "NONE")
	{
		// We have decent family, look in info class
		TSubclassOf<UUTFamilyInfo> FamilyInfoClass = UUTFamilyInfo::FindFamilyInfo(CharacterData.FamilyID);
		if (FamilyInfoClass != NULL)
		{
			bHasSetTrust = true;
			TrustWorthiness = FamilyInfoClass.GetDefaultObject<UUTFamilyInfo>()->Trustworthiness;
		}
	}
	*/

	return TrustWorthiness;
}

void AUTBetrayalPlayerState::UpdateTeam(AUTBetrayalTeam* Team)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		// for listen server support, call notify team change locally
		NotifyTeamChanged();
	}
}

void AUTBetrayalPlayerState::UpdateNemesis(AUTBetrayalPlayerState* PRI)
{
	if (Role == ROLE_Authority && PRI != NULL)
	{
		// update/increase kill count for current player
		auto CurrentCount = NemesisData.FindRef(PRI->PlayerId);
		NemesisData.Add(PRI->PlayerId, CurrentCount + 1);
		NemesisNames.Add(PRI->PlayerId, PRI->PlayerName);

		// find player with highest kill count
		int32 BestId = -1;
		auto BestCount = 0;
		for (auto& Elem : NemesisData)
		{
			if (Elem.Value > BestCount)
			{
				BestId = Elem.Key;
				BestCount = Elem.Value;
			}
		}

		// update current replicated nemesis value
		if (BestId != -1)
		{
			FString PlayerName = NemesisNames.FindRef(BestId);
			CurrentNemesis = PlayerName;
		}
	}
}

void AUTBetrayalPlayerState::NotifyTeamChanged_Implementation()
{
	Super::NotifyTeamChanged_Implementation();

	// notify all other pawns to change the team overlay
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
	{
		AUTBetrayalCharacter* P = Cast<AUTBetrayalCharacter>(*It);
		if (P != NULL && P->PlayerState != this && !P->bTearOff)
		{
			P->NotifyTeamChanged();
		}
	}
}
