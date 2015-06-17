#include "UTBetrayal.h"
#include "UTBetrayalGameMode.h"
#include "UTPlayerController.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalGameState.h"

#include "UTBetrayalCharacterPostRenderer.h"

AUTBetrayalPlayerState::AUTBetrayalPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RemainingRogueTime = -1000;
	RogueTimePenalty = 30;

	static ConstructorHelpers::FObjectFinder<USoundCue> RogueFadingSoundFinder(TEXT("SoundCue'/UTBetrayal/Sounds/UT3ServerSignOut_Cue.UT3ServerSignOut_Cue'"));
	RogueFadingSound = RogueFadingSoundFinder.Object;
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
	DOREPLIFETIME(AUTBetrayalPlayerState, HighestPot);

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
	HighestPot = 0;
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
	APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
	AUTBetrayalGameState* GS = GetWorld()->GetGameState<AUTBetrayalGameState>();

	if (PC != NULL && GS != NULL)
	{
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; ++It)
		{
			AUTCharacter* P = Cast<AUTCharacter>(*It);
			if (P != NULL && !P->bTearOff)
			{
				bool bOnSameTeam = PC->PlayerState == P->PlayerState || GS->OnSameTeam(PC, P);
				ApplyTeamColorFor(P, bOnSameTeam);
			}
		}
	}
}

void AUTBetrayalPlayerState::ApplyTeamColorFor(AUTCharacter* P, bool bIsTeam)
{
	const TArray<UMaterialInstanceDynamic*>& BodyMIs = P->GetBodyMIs();
	for (UMaterialInstanceDynamic* MI : BodyMIs)
	{
		if (MI != NULL)
		{
			MI->SetScalarParameterValue(TEXT("TeamSelect"), bIsTeam ? 1.0 : 0.0);

			// FIXME: TEMP HACK. HitFlashColor used for BrightSkin for team members
			MI->SetVectorParameterValue(TEXT("HitFlashColor"), bIsTeam ? FLinearColor(0.0f, 0.0f, 1.0f) : FLinearColor(0.f, 0.f, 0.f, 0.f));
		}
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
