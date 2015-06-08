#include "UTBetrayal.h"
#include "UTBetrayalScoreboard.h"
#include "UTBetrayalPlayerState.h"
#include "UTGameState.h"

UUTBetrayalScoreboard::UUTBetrayalScoreboard(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AllyColor = FColor(64, 128, 255);

	DaggerWidth = 16;				//width of dagger icon
	DaggerHeight = 28;				//height of dagger icon
	DaggerSpacing = 12.0f;			//spacing between individual daggers
	SilverDaggerOffset = 10.0f;		//spacing between silver and gold daggers
	DaggerXPadding = 6.0f;			//spacing between name and dagger icons
	

	DaggerTexCoords = FTextureUVs(262.0f, 53.0f, 16.0f, 28.0f);

	GoldLinearColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	SilverLinearColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);

	static ConstructorHelpers::FObjectFinder<UTexture2D> UT3GHudTextureObj(TEXT("Texture2D'/UTBetrayal/Textures/HUDIcons.HUDIcons'"));
	UT3GHudTexture = UT3GHudTextureObj.Object;
}

void UUTBetrayalScoreboard::DrawScoreHeaders(float RenderDelta, float& YOffset)
{
	// workdaround for larger cell width // FIXMESTEVE: add cell width property
	float OriginalCenterBuffer = CenterBuffer;
	CenterBuffer = -(0.25f * Size.X);

	Super::DrawScoreHeaders(RenderDelta, YOffset);

	// revert center buffer
	CenterBuffer = OriginalCenterBuffer;
}

void UUTBetrayalScoreboard::DrawPlayer(int32 Index, AUTPlayerState* PlayerState, float RenderDelta, float XOffset, float YOffset)
{
	// Workaround for larger cell width // FIXMESTEVE: add cell width property
	float OriginalCenterBuffer = CenterBuffer;
	XOffset = (Size.X * 0.125f);
	CenterBuffer = -(0.25f * Size.X);

	Super::DrawPlayer(Index, PlayerState, RenderDelta, XOffset, YOffset);

	// skip for wrong gametype
	AUTBetrayalPlayerState* BPRI = Cast<AUTBetrayalPlayerState>(PlayerState);
	if (BPRI == NULL) return;

	float Width = (Size.X * 0.5f) - CenterBuffer;
	float XL, YL;

	// revert center buffer
	CenterBuffer = OriginalCenterBuffer;

	// re-create player name
	FText PlayerName = FText::FromString(GetClampedName(PlayerState, UTHUDOwner->MediumFont, 1.f, 0.475f*Width));

	// retrieve the size of the previously drawn player name
	Canvas->TextSize(UTHUDOwner->MediumFont, PlayerName.ToString(), XL, YL);

	// Draw the daggers
	float PosX = XOffset + (Width * ColumnHeaderPlayerX);
	PosX += DaggerXPadding + XL; // add playername/dagger offset
	if (PlayerState->bIsFriend) PosX += 30 + 5; // add friend icon offset
	DrawDaggers(BPRI, RenderDelta, PosX, YOffset);

	// Workaround for drawing team players in different color // FIXMESTEVE: add GetPlayerColor to get color by player
	if (AUTGameState* GS = GetWorld()->GetGameState<AUTGameState>())
	{
		if (UTHUDOwner != NULL && GS->OnSameTeam(BPRI, UTHUDOwner->PlayerOwner))
		{
			DrawText(PlayerName, XOffset + (Width * ColumnHeaderPlayerX), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, AllyColor, ETextHorzPos::Left, ETextVertPos::Center);
		}
	}
}

void UUTBetrayalScoreboard::DrawDaggers(AUTBetrayalPlayerState* PRI, float RenderDelta, float XOffset, float YOffset)
{
	//Just sanity clamp
	int32 TempCount = FMath::Clamp<int32>(PRI->BetrayalCount, 0, 100);
	int32 NumGoldDaggers = TempCount / 5;
	int32 NumSilverDaggers = TempCount % 5;

	float BarOpacity = 0.75f;
	float DaggerAspect = DaggerTexCoords.VL == 0.0 ? 0.0 : DaggerTexCoords.UL / DaggerTexCoords.VL;

	//Start drawing the daggers
	for (int32 i = 0; i<NumGoldDaggers; i++)
	{
		DrawTexture(UT3GHudTexture, XOffset, YOffset, 32 * DaggerAspect, 32, DaggerTexCoords.U, DaggerTexCoords.V, DaggerTexCoords.UL, DaggerTexCoords.VL, BarOpacity, GoldLinearColor);

		//Don't bump for the last gold dagger drawn
		if (i<NumGoldDaggers - 1)
		{
			XOffset += (DaggerSpacing * RenderScale);
		}
	}

	//Add spacing between gold/silver daggers
	if (NumGoldDaggers > 0)
	{
		XOffset += (SilverDaggerOffset * RenderScale);
	}

	for (int32 i = 0; i<NumSilverDaggers; i++)
	{
		DrawTexture(UT3GHudTexture, XOffset, YOffset, 32 * DaggerAspect, 32, DaggerTexCoords.U, DaggerTexCoords.V, DaggerTexCoords.UL, DaggerTexCoords.VL, BarOpacity, SilverLinearColor);

		XOffset += (DaggerSpacing * RenderScale);
	}
}
