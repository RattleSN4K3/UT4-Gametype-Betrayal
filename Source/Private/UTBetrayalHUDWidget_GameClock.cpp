#include "UTBetrayal.h"
#include "UTBetrayalHUDWidget_GameClock.h"

UUTBetrayalHUDWidget_GameClock::UUTBetrayalHUDWidget_GameClock(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// General Layout

	Position = FVector2D(0.0f, 0.0f);
	Size = FVector2D(288.0f, 83.0f);
	ScreenPosition = FVector2D(0.0f, 0.0f);
	Origin = FVector2D(0.0f, 0.0f);

	DesignedResolution = 1080.f;

	AltClockScale = 0.5f;


	// Elements Look & Layout

	static ConstructorHelpers::FObjectFinder<UTexture2D> BackgroundSlateObj(TEXT("Texture2D'/Game/RestrictedAssets/UI/HUDAtlas01.HUDAtlas01'"));
	if (BackgroundSlateObj.Object != NULL)
	{
		BackgroundSlate.Atlas = BackgroundSlateObj.Object;
		BackgroundSlate.UVs = FTextureUVs(256.0f, 313.0f, 144.0f, 83.0f);
		BackgroundSlate.bIsSlateElement = true;
		//BackgroundSlate.Position = FVector2D(0.0f, 0.0f);
		BackgroundSlate.Size = FVector2D(230.0f, 0.0f);
		BackgroundSlate.RenderColor = FLinearColor::Black;

		BackgroundBorder.Atlas = BackgroundSlateObj.Object;
		BackgroundBorder.UVs = FTextureUVs(230.0f, 313.0f, 430.0f, 83.0f);
		BackgroundBorder.bIsBorderElement = true;
		BackgroundBorder.RenderColor = FLinearColor::Black;
	}

	static ConstructorHelpers::FObjectFinder<UFont> ClockTextFontObj(TEXT("Font'/Game/RestrictedAssets/UI/Fonts/fntScoreboard_Clock.fntScoreboard_Clock'"));
	if (ClockTextFontObj.Object != NULL)
	{
		ClockText.Font = ClockTextFontObj.Object;
		ClockText.Text = FText::FromString(TEXT("000"));
		ClockText.TextScale = 1.6f;
		ClockText.HorzPosition = ETextHorzPos::Right;
		ClockText.VertPosition = ETextVertPos::Top;
		ClockText.Position = FVector2D(230.0f, 0.0f);
		ClockText.RenderColor = FLinearColor::White;
	}
}

void UUTBetrayalHUDWidget_GameClock::InitializeWidget(AUTHUD* Hud)
{
	Super::InitializeWidget(Hud);

	ClockText.GetTextDelegate.BindUObject(this, &UUTBetrayalHUDWidget_GameClock::GetClockText_Implementation);
}

FText UUTBetrayalHUDWidget_GameClock::GetClockText_Implementation()
{
	float RemainingTime = UTGameState ? UTGameState->GetClockTime() : 0.f;
	FText ClockString = UTHUDOwner->ConvertTime(FText::GetEmpty(), FText::GetEmpty(), RemainingTime, false);
	ClockText.TextScale = (RemainingTime >= 3600) ? AltClockScale : GetClass()->GetDefaultObject<UUTBetrayalHUDWidget_GameClock>()->ClockText.TextScale;
	return ClockString;
}
