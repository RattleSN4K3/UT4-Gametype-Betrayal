#include "UTBetrayal.h"
#include "UTBetrayalHUDWidget_WeaponCrosshair.h"

UUTBetrayalHUDWidget_WeaponCrosshair::UUTBetrayalHUDWidget_WeaponCrosshair(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUTBetrayalHUDWidget_WeaponCrosshair::Draw_Implementation(float DeltaTime)
{
	if (UTHUDOwner != NULL && UTHUDOwner->UTPlayerOwner != NULL)
	{
		AUTCharacter* UTCharacter = Cast<AUTCharacter>(UTHUDOwner->UTPlayerOwner->GetViewTarget());
		if (UTCharacter)
		{
			// only draw simplistic crosshair (also prevent drawing friendly fire indicator)

			UTexture2D* CrosshairTexture = UTHUDOwner->DefaultCrosshairTex;
			if (CrosshairTexture != NULL)
			{
				float W = CrosshairTexture->GetSurfaceWidth();
				float H = CrosshairTexture->GetSurfaceHeight();
				float CrosshairScale = UTHUDOwner->GetCrosshairScale();

				DrawTexture(CrosshairTexture, 0, 0, W * CrosshairScale, H * CrosshairScale, 0.0, 0.0, 16, 16, 1.0, UTHUDOwner->GetCrosshairColor(FLinearColor::White), FVector2D(0.5f, 0.5f));
			}
		}
	}
}
