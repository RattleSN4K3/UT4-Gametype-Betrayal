#include "UTBetrayal.h"
#include "UTBetrayalCharacterPostRenderer.h"
#include "UTBetrayalHUD.h"
#include "UTPlayerCameraManager.h"

AUTBetrayalCharacterPostRenderer::AUTBetrayalCharacterPostRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
	bReplicates = true;
	bAlwaysRelevant = true;
	bReplicateMovement = false;
	bNetLoadOnClient = true;
}

void AUTBetrayalCharacterPostRenderer::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AUTBetrayalCharacterPostRenderer, RenderPawn, COND_InitialOnly);
}

void AUTBetrayalCharacterPostRenderer::BeginPlay()
{
	if (Role == ROLE_Authority)
	{
		AUTCharacter* Pawn = Cast<AUTCharacter>(GetOwner());
		if (Pawn == NULL)
		{
			UE_LOG(Betrayal, Verbose, TEXT("CharacterPostRenderer: Destroy TeamColor helper %s"), *GetName());
			Destroy();
			return;
		}

		// store pawn
		RenderPawn = Pawn;
		if (GetWorld()->GetNetMode() != NM_DedicatedServer)
		{
			OnRep_RenderPawn();
		}
	}

	Super::BeginPlay();

	if (Role != ROLE_Authority)
	{
		// set timeout for replicated referenced Pawn... to clear/destroy zombie actors
		SetLifeSpan(10.0f);
	}
}

void AUTBetrayalCharacterPostRenderer::OnRep_RenderPawn()
{
	if (RenderPawn != NULL && !bRenderPawnInitialized)
	{
		// Actor referenced, set life spawn to infinite
		SetLifeSpan(0.0f);

		// Bind OnDied event to this pawn to garbage collect this zombie actor
		RenderPawn->OnDied.AddDynamic(this, &AUTBetrayalCharacterPostRenderer::OnPawnDied);

		if (GetWorld()->GetNetMode() != NM_DedicatedServer)
		{
			HookRender();
		}

		bRenderPawnInitialized = true;
	}
	else if (RenderPawn == NULL && bRenderPawnInitialized && GetWorld()->TimeSeconds - CreationTime > 10.0f)
	{
		// still existing and active replication? 
		// Set life spawn just in case to destroy actor automatically
		SetLifeSpan(5.0f);
	}
}

void AUTBetrayalCharacterPostRenderer::HookRender()
{
	TArray<AUTPlayerController*> PCs;

	// support for splitscreen. add to each PCs HUD
	bool bFound = false;
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AUTPlayerController* PC = Cast<AUTPlayerController>(*Iterator);
		if (PC == NULL) continue;

		if (PC->MyHUD == NULL)
		{
			FTimerHandle TempHandle;
			GetWorldTimerManager().SetTimer(TempHandle, this, &AUTBetrayalCharacterPostRenderer::HookRender, 0.1f, false);
			return;
		}

		PCs.Add(PC);
	}

	for (auto PC : PCs)
	{
		if (PC && PC->MyHUD)
		{
			PC->MyHUD->RemovePostRenderedActor(RenderPawn);
			PC->MyHUD->AddPostRenderedActor(this);
			bFound = true;
		}
	}

	// destroy if no PC is found
	if (!bFound)
	{
		UE_LOG(Betrayal, Verbose, TEXT("CharacterPostRenderer: No PC with a HUD found. Abort applying PostRender fix..."));
		Destroy();
		return;
	}
}

void AUTBetrayalCharacterPostRenderer::PostRenderFor(APlayerController* PC, UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	// TODO: FIXME: Find crash related to access violation (NULL POINTER)

	if (PC == NULL || RenderPawn == NULL || RenderPawn->TimeOfDeath > 0.0 || RenderPawn->IsPendingKillPending() /* RenderPawn->IsValidLowLevelFast()*/)
	{
		Destroy();
		return;
	}

	FVector WorldPosition = RenderPawn->GetMesh()->GetComponentLocation();
	FVector ScreenPosition = Canvas->Project(WorldPosition + FVector(0.f, 0.f, RenderPawn->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() * 2.25f));
	
	// make sure not clipped out
	if (FVector::DotProduct(CameraDir, (RenderPawn->GetActorLocation() - CameraPosition)) <= 0.0f)
	{
		return;
	}

	AUTBetrayalHUD* Hud = Cast<AUTBetrayalHUD>(PC->MyHUD);
	if (Hud != NULL && !RenderPawn->IsFeigningDeath())
	{
		float Dist = (CameraPosition - RenderPawn->GetActorLocation()).Size();
		float MaxDist = 2.0 * RenderPawn->TeamPlayerIndicatorMaxDistance;
		if (Dist <= MaxDist && IsPawnVisible(PC, CameraPosition, RenderPawn))
		{
			Hud->DrawPlayerBeacon(RenderPawn, Canvas, CameraPosition, CameraDir, ScreenPosition);
		}
	}
}

bool AUTBetrayalCharacterPostRenderer::IsPawnVisible(APlayerController* PC, FVector CameraPosition, ACharacter* P)
{
	if (AUTPlayerCameraManager* CamMgr = Cast<AUTPlayerCameraManager>(PC->PlayerCameraManager))
	{
		FHitResult Result(1.f);
		CamMgr->CheckCameraSweep(Result, P, CameraPosition, P->GetActorLocation() + FVector(0.f, 0.f, P->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight()));
		if (Result.bBlockingHit)
		{
			return false;
		}
	}

	return true;
}

void AUTBetrayalCharacterPostRenderer::OnPawnDied(AController* Killer, const UDamageType* DamageType)
{
	UE_LOG(Betrayal, Log, TEXT("CharacterPostRenderer: RefPawn died. Destroy..."));
	Destroy();
}
