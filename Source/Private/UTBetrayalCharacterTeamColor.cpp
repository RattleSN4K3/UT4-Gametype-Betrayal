#include "UTBetrayal.h"
#include "UTBetrayalCharacterTeamColor.h"
#include "UTBetrayalGameState.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalTeamInfoStub.h"

#include "UTCharacterContent.h"

// TODO: Merge with Betrayal PlayerState

AUTBetrayalCharacterTeamColor::AUTBetrayalCharacterTeamColor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAlwaysRelevant = true;
	InitialLifeSpan = 30.f;
}

void AUTBetrayalCharacterTeamColor::Assign(AUTCharacter* Char)
{
	if (Char == NULL)
	{
		UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor: No character. Destroy TeamColor helper %s"), *GetName());
		Destroy();
		return;
	}

	RefPawn = Char;
	InitializePawn();
}

void AUTBetrayalCharacterTeamColor::InitializePawn()
{
	if (RefPawn != NULL && !bPawnInitialized)
	{
		bPawnInitialized = true;

		// Actor referenced, set life spawn to infinite
		SetLifeSpan(0.f);

		// Bind OnDied event to this pawn to garbage collect this zombie actor
		RefPawn->OnDied.AddDynamic(this, &AUTBetrayalCharacterTeamColor::OnPawnDied);

		// initialize team color material
		HookPawn();
	}
}

void AUTBetrayalCharacterTeamColor::HookPawn()
{
	// FIXME: TEMP HACK. Exchange default materials with Team materials properly

	if (RefPawn == NULL)
	{
		UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor::HookPawn - No Pawn. Abort..."));
		Destroy();
		return;
	}

	AUTPlayerState* PS = Cast<AUTPlayerState>(RefPawn->PlayerState);
	if (PS == NULL)
	{
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AUTBetrayalCharacterTeamColor::HookPawn, 0.1f, false);

		PlayerStateErrorCount++;
		if (PlayerStateErrorCount > 100)
		{
			UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor::HookPawn - Too many tries to find PlayerState for %s. Destroy helper..."), *RefPawn->GetName());
			Destroy();
		}

		UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor::HookPawn - Retry to find PlayerState for %s..."), *RefPawn->GetName());
		return;
	}

	// override materials with Team materials to apply team color
	AUTTeamInfo* NewTeam = GetWorld()->SpawnActor<AUTTeamInfo>(AUTBetrayalTeamInfoStub::StaticClass());
	PS->Team = NewTeam;
	RefPawn->ApplyCharacterData(PS->GetSelectedCharacter());
	PS->Team = NULL;

	NewTeam->Destroy();

	// Apply Rim color so Blue team glows from distance
	const TArray<UMaterialInstanceDynamic*>& BodyMIs = RefPawn->GetBodyMIs();
	for (UMaterialInstanceDynamic* MI : BodyMIs)
	{
		if (MI != NULL)
		{
			MI->SetVectorParameterValue(TEXT("RedTeamRim"), FLinearColor(0.0f, 0.0f, 0.0f));
			MI->SetVectorParameterValue(TEXT("BlueTeamRim"), FLinearColor(0.0f, 0.0f, 40.0f));
		}
	}

	PlayerStateErrorCount = 0;
	UpdateTeamColor();
}

void AUTBetrayalCharacterTeamColor::UpdateTeamColor()
{
	AUTBetrayalPlayerState* PS = NULL;
	if (RefPawn != NULL)
	{
		PS = Cast<AUTBetrayalPlayerState>(RefPawn->PlayerState);
	}

	if (PS == NULL)
	{
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AUTBetrayalCharacterTeamColor::UpdateTeamColor, 0.1f, false);

		PlayerStateErrorCount++;
		if (PlayerStateErrorCount > 100)
		{
			UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor::UpdateTeamColor - Too many tries to find PlayerState for %s. Destroy helper..."), *RefPawn->GetName());
			Destroy();
		}

		UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor::UpdateTeamColor - Retry to find PlayerState for %s..."), *RefPawn->GetName());
		return;
	}

	bool bOnSameTeam = false;
	if (PS->CurrentTeam != NULL)
	{
		APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (PS->GetInstigatorController() == LocalPC || LocalPC->PlayerState == PS)
		{
			bOnSameTeam = true;
		}
		else
		{
			AUTGameState* GS = GetWorld()->GetGameState<AUTGameState>();
			bOnSameTeam = GS != NULL && GS->OnSameTeam(LocalPC, PS);
		}
	}
	
	ApplyTeamColorFor(RefPawn, bOnSameTeam);
}

void AUTBetrayalCharacterTeamColor::ApplyTeamColorFor(AUTCharacter* P, bool bIsTeam)
{
	const TArray<UMaterialInstanceDynamic*>& BodyMIs = P->GetBodyMIs();
	for (UMaterialInstanceDynamic* MI : BodyMIs)
	{
		if (MI != NULL)
		{
			MI->SetScalarParameterValue(TEXT("TeamSelect"), bIsTeam ? 1.0 : 0.0);
			MI->SetScalarParameterValue(TEXT("FullBodyFlashPct"), bIsTeam ? 0.4 : 1.0);

			// FIXME: TEMP HACK. HitFlashColor used for BrightSkin for team members
			MI->SetVectorParameterValue(TEXT("HitFlashColor"), bIsTeam ? FLinearColor(0.0f, 0.0f, 1.0f) : FLinearColor(0.f, 0.f, 0.f, 0.f));
		}
	}
}

void AUTBetrayalCharacterTeamColor::OnPawnDied(AController* Killer, const UDamageType* DamageType)
{
	UE_LOG(Betrayal, Verbose, TEXT("CharacterTeamColor: RefPawn died. Destroy..."));
	Destroy();
}
