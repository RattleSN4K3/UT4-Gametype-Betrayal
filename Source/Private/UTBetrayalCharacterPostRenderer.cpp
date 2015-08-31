#include "UTBetrayal.h"
#include "UTBetrayalCharacterPostRenderer.h"
#include "UTBetrayalHUD.h"
#include "UTPlayerCameraManager.h"

// TODO: Remove class if PostRender works fully reliable
// Class is not directly used/needed if the character class is a subclass of UTBetrayalCharacter

AUTBetrayalCharacterPostRenderer::AUTBetrayalCharacterPostRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAlwaysRelevant = true;
	InitialLifeSpan = 30.f;
}

void AUTBetrayalCharacterPostRenderer::Assign(AUTCharacter* Char)
{
	if (Char == NULL)
	{
		UE_LOG(Betrayal, Verbose, TEXT("CharacterPostRenderer: Destroy PostRender helper %s"), *GetName());
		Destroy();
		return;
	}

	RenderPawn = Char;
	InitializePawn();
}

void AUTBetrayalCharacterPostRenderer::InitializePawn()
{
	if (RenderPawn != NULL && !bPawnInitialized)
	{
		bPawnInitialized = true;

		// Actor referenced, set life span to infinite
		SetLifeSpan(0.f);

		// Bind OnDied event to this pawn to garbage collect this zombie actor
		RenderPawn->OnDied.AddDynamic(this, &AUTBetrayalCharacterPostRenderer::OnPawnDied);
	}
}

void AUTBetrayalCharacterPostRenderer::PostRenderFor(APlayerController* PC, UCanvas* Canvas, FVector CameraPosition, FVector CameraDir)
{
	// TODO: FIXME: Find crash related to access violation (NULL POINTER)

	if (PC == NULL || RenderPawn == NULL || RenderPawn->TimeOfDeath > 0.0 || RenderPawn->IsPendingKillPending() /* RenderPawn->IsValidLowLevelFast()*/)
	{
		RenderPawn = NULL;
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
	UE_LOG(Betrayal, Verbose, TEXT("CharacterPostRenderer: RenderPawn died. Destroy..."));
	Destroy();
}
