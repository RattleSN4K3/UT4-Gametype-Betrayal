#include "UTBetrayal.h"
#include "UTBetrayalScoreboard.h"
#include "UTBetrayalPlayerState.h"
#include "UTGameState.h"

UUTBetrayalScoreboard::UUTBetrayalScoreboard(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UTexture2D> HudTexture;

		FConstructorStatics()
			: HudTexture(TEXT("Texture2D'/UTBetrayal/Textures/HUDIcons.HUDIcons'"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics; 
	
	ColumnMedalX = 0.5;
	ColumnHeaderScoreX = 0.6;
	ColumnHeaderKillsX = 0.72;

	AllyColor = FColor(64, 128, 255);

	DaggerWidth = 16;				//width of dagger icon
	DaggerHeight = 28;				//height of dagger icon
	DaggerSpacing = 12.0f;			//spacing between individual daggers
	SilverDaggerOffset = 10.0f;		//spacing between silver and gold daggers
	DaggerXPadding = 6.0f;			//spacing between name and dagger icons

	DaggerTexCoords = FTextureUVs(262.0f, 53.0f, 16.0f, 28.0f);

	GoldLinearColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	SilverLinearColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);

	UT3GHudTexture = ConstructorStatics.HudTexture.Object;
}

void UUTBetrayalScoreboard::DrawScoreHeaders(float RenderDelta, float& YOffset)
{
	// workdaround for larger cell width // FIXMESTEVE: add cell width property
	float OriginalCenterBuffer = CenterBuffer;
	CenterBuffer = -(0.25f * Size.X);

	Super::DrawScoreHeaders(RenderDelta, YOffset);

	// revert center buffer
	CenterBuffer = OriginalCenterBuffer;

	// TODO: Use GetScoreHeaders (Pull Request?!)
	// draw additional kills column
	float Width = 0.75 * Size.X;
	float Height = 23;
	int32 ColumnCnt = ((UTGameState && UTGameState->bTeamGame) || ActualPlayerCount > 16) ? 2 : 1;
	float XOffset = ColumnCnt > 1 ? 0 : (Size.X * 0.5) - (Width * 0.5);

	FText CH_Kills = NSLOCTEXT("UTBetrayalScoreboard", "ColumnHeader_PlayerKills", "KILLS");

	float TempYOffset = YOffset - Height - 4;
	for (int32 i = 0; i < ColumnCnt; i++)
	{
		if (UTGameState && UTGameState->HasMatchStarted())
		{
			DrawText(CH_Kills, XOffset + (Width * ColumnHeaderKillsX), TempYOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);
		}
	}
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

	// draw additional kills stat
	if (UTGameState && UTGameState->HasMatchStarted())
	{
		FText PlayerKills = FText::AsNumber(PlayerState->Kills);
		DrawText(PlayerKills, XOffset + (Width * ColumnHeaderKillsX), YOffset + ColumnY, UTHUDOwner->SmallFont, 1.0f, 1.0f, Canvas->DrawColor, ETextHorzPos::Center, ETextVertPos::Center);
	}

	// retrieve the size of the previously drawn player name
	Canvas->TextSize(UTHUDOwner->MediumFont, LastPlayerName, XL, YL);

	// Draw the daggers
	float PosX = XOffset + (Width * ColumnHeaderPlayerX);
	PosX += DaggerXPadding + XL; // add playername/dagger offset
	if (PlayerState->bIsFriend) PosX += 30 + 5; // add friend icon offset
	DrawDaggers(BPRI, RenderDelta, PosX, YOffset);
}

void UUTBetrayalScoreboard::DrawDaggers(AUTBetrayalPlayerState* PRI, float RenderDelta, float XOffset, float YOffset)
{
	float BarOpacity = 0.75f;
	float DaggerAspect = DaggerTexCoords.VL == 0.0 ? 0.0 : DaggerTexCoords.UL / DaggerTexCoords.VL;

	if (PRI->BetrayalCount >= 100)
	{
		// draw simple format "x NUM"
		DrawTexture(UT3GHudTexture, XOffset, YOffset, 32 * DaggerAspect, 32, DaggerTexCoords.U, DaggerTexCoords.V, DaggerTexCoords.UL, DaggerTexCoords.VL, BarOpacity, FLinearColor::White);
		XOffset += 32 * DaggerAspect + (DaggerSpacing * RenderScale);

		FText DaggerString = FText::Format(NSLOCTEXT("UTBetrayalScoreboard", "DaggersShortString", "x {0}"), FText::AsNumber(PRI->BetrayalCount));
		DrawText(DaggerString, XOffset, YOffset + ColumnY, UTHUDOwner->SmallFont, 1.0f, 1.0f, FLinearColor::Gray, ETextHorzPos::Left, ETextVertPos::Center);
	}
	else
	{
		int32 NumGoldDaggers = PRI->BetrayalCount / 5;
		int32 NumSilverDaggers = PRI->BetrayalCount % 5;

		//Start drawing the daggers
		for (int32 i = 0; i < NumGoldDaggers; i++)
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

		for (int32 i = 0; i < NumSilverDaggers; i++)
		{
			DrawTexture(UT3GHudTexture, XOffset, YOffset, 32 * DaggerAspect, 32, DaggerTexCoords.U, DaggerTexCoords.V, DaggerTexCoords.UL, DaggerTexCoords.VL, BarOpacity, SilverLinearColor);

			XOffset += (DaggerSpacing * RenderScale);
		}
	}
}

FLinearColor UUTBetrayalScoreboard::GetPlayerColorFor(AUTPlayerState* InPS) const
{
	if (AUTGameState* GS = GetWorld()->GetGameState<AUTGameState>())
	{
		if (UTHUDOwner != NULL && GS->OnSameTeam(InPS, UTHUDOwner->PlayerOwner))
		{
			return FLinearColor(AllyColor);
		}
	}

	return Super::GetPlayerColorFor(InPS);
}

FString UUTBetrayalScoreboard::GetClampedName(AUTPlayerState* PS, UFont* NameFont, float NameScale, float MaxWidth)
{
	// Workaround for GetPlayerNameFor not being used. Original code uses GetClampedName in UUTScoreboard::DrawPlayer. Store the last created name
	LastPlayerName = Super::GetClampedName(PS, NameFont, NameScale, MaxWidth);
	return LastPlayerName;
}
