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
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bAlwaysRelevant = true;
	bReplicateMovement = false;
	bNetLoadOnClient = true;
}

void AUTBetrayalCharacterTeamColor::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AUTBetrayalCharacterTeamColor, RefPawn, COND_InitialOnly);
}

void AUTBetrayalCharacterTeamColor::BeginPlay()
{
	if (Role == ROLE_Authority)
	{
		AUTCharacter* Pawn = Cast<AUTCharacter>(GetOwner());
		if (Pawn == NULL)
		{
			UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor: Destroy TeamColor helper %s"), *GetName());
			Destroy();
			return;
		}

		RefPawn = Pawn;
		if (GetWorld()->GetNetMode() != NM_DedicatedServer)
		{
			OnRep_RefPawn();
		}
	}

	Super::BeginPlay();

	if (Role != ROLE_Authority)
	{
		// set timeout for replicated referenced Pawn... to clear/destroy zombie actors
		SetLifeSpan(10.0f);
	}
}

void AUTBetrayalCharacterTeamColor::OnRep_RefPawn()
{
	if (RefPawn != NULL)
	{
		// Actor referenced, set life spawn to infinite
		SetLifeSpan(0.0f);

		// Bind OnDied event to this pawn to garbage collect this zombie actor
		RefPawn->OnDied.AddDynamic(this, &AUTBetrayalCharacterTeamColor::OnRefPawnDied);

		// initialize team color material
		InitializePawn(RefPawn);

		if (GetWorld()->GetNetMode() != NM_DedicatedServer)
		{
			UpdateTeamColor();
		}
		
		bRefPawnInitialized = true;
	}
	else if (bRefPawnInitialized && GetWorld()->TimeSeconds - CreationTime > 10.0f)
	{
		// still existing and active replication? 
		// Set life spawn just in case to destroy actor automatically
		SetLifeSpan(5.0f);
	}
}

void AUTBetrayalCharacterTeamColor::UpdateTeamColor()
{
	AUTBetrayalPlayerState* PS = RefPawn ? Cast<AUTBetrayalPlayerState>(RefPawn->PlayerState) : NULL;
	if (PS == NULL)
	{
		FTimerHandle TempHandle;
		GetWorldTimerManager().SetTimer(TempHandle, this, &AUTBetrayalCharacterTeamColor::UpdateTeamColor, 0.1f, false);

		PlayerStateErrorCount++;
		if (PlayerStateErrorCount > 100)
		{
			UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor: Too many tries to find PlayerState for %s. Destroy helper..."), *RefPawn->GetName());
			Destroy();
		}

		UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor: Retry to find PlayerState for %s..."), *RefPawn->GetName());
		return;
	}

	bool bOnSameTeam = false;
	if (PS->CurrentTeam != NULL)
	{
		// TODO: Add support for Splitscreen team color overlay
		APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
		AUTBetrayalGameState* GS = GetWorld()->GetGameState<AUTBetrayalGameState>();
		if (PC != NULL && GS != NULL)
		{
			bOnSameTeam = PC->PlayerState == PS || GS->OnSameTeam(PC, PS);
		}
	}
	
	PS->ApplyTeamColorFor(RefPawn, bOnSameTeam);
}

void AUTBetrayalCharacterTeamColor::InitializePawn(AUTCharacter* Pawn)
{
	// FIXME: TEMP HACK. Exchange default materials with Team materials properly

	if (Pawn == NULL)
	{
		UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor: No Pawn. Abort..."));
		return;
	}

	AUTPlayerState* PS = Cast<AUTPlayerState>(Pawn->PlayerState);
	if (PS == NULL)
	{
		UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor: No PlayerState. Abort..."));
		return;
	}

	// override materials with Team materials to apply team color
	AUTTeamInfo* NewTeam = GetWorld()->SpawnActor<AUTTeamInfo>(AUTBetrayalTeamInfoStub::StaticClass());
	PS->Team = NewTeam;
	Pawn->ApplyCharacterData(PS->GetSelectedCharacter());
	PS->Team = NULL;

	NewTeam->Destroy();
}

void AUTBetrayalCharacterTeamColor::OnRefPawnDied(AController* Killer, const UDamageType* DamageType)
{
	UE_LOG(Betrayal, Log, TEXT("CharacterTeamColor: RefPawn died. Destroy..."));
	Destroy();
}
