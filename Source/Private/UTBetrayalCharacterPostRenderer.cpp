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

void AUTBetrayalCharacterPostRenderer::BeginPlay()
{
	RenderPawn = Cast<AUTCharacter>(GetOwner());
	if (RenderPawn == NULL)
	{
		Destroy();
		return;
	}

	// support for splitscreen. add to each PCs HUD
	bool bFound = false;
	for (FConstControllerIterator Iterator = GetWorld()->GetControllerIterator(); Iterator; ++Iterator)
	{
		AUTPlayerController* PC = Cast<AUTPlayerController>(*Iterator);
		if (PC != NULL && PC->MyHUD != NULL)
		{
			PC->MyHUD->RemovePostRenderedActor(RenderPawn);
			PC->MyHUD->AddPostRenderedActor(this);
			bFound = true;
		}
	}

	// destroy if no PC is found
	if (!bFound)
	{
		Destroy();
		return;
	}

	Super::BeginPlay();
}

void AUTBetrayalCharacterPostRenderer::PostRenderFor(APlayerController* PC, UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	if (PC == NULL || RenderPawn == NULL || RenderPawn->IsPendingKillPending() /* RenderPawn->IsValidLowLevelFast()*/)
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
		if (Dist <= RenderPawn->TeamPlayerIndicatorMaxDistance && IsPawnVisible(PC, CameraPosition, RenderPawn))
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