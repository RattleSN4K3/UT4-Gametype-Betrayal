#include "UTBetrayal.h"
#include "UTBetrayalHUD.h"
#include "UTBetrayalPlayerState.h"
#include "UTHUD_DM.h"

// TODO: Use FUTCanvasTextItem to draw beacon
// TODO: Add opacity to Beacons near crosshair

AUTBetrayalHUD::AUTBetrayalHUD(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RequiredHudWidgetClasses = AUTHUD_DM::StaticClass()->GetDefaultObject<AUTHUD_DM>()->RequiredHudWidgetClasses;

	RequiredHudWidgetClasses.Remove(TEXT("/Script/UnrealTournament.UTHUDWidget_WeaponCrosshair"));
	RequiredHudWidgetClasses.Remove(TEXT("/Game/RestrictedAssets/UI/HUDWidgets/bpHW_DMLeaderboard.bpHW_DMLeaderboard_C"));
	RequiredHudWidgetClasses.Remove(TEXT("/Game/RestrictedAssets/UI/HUDWidgets/bpHW_DMGameClock.bpHW_DMGameClock_C"));
	RequiredHudWidgetClasses.Remove(TEXT("/Script/UnrealTournament.UTScoreboard"));

	RequiredHudWidgetClasses.Remove(TEXT("/Game/RestrictedAssets/UI/HUDWidgets/bpHW_WeaponInfo.bpHW_WeaponInfo_C"));

	RequiredHudWidgetClasses.Add(TEXT("/Script/UTBetrayal.UTBetrayalHUDWidget_GameClock"));
	RequiredHudWidgetClasses.Add(TEXT("/Script/UTBetrayal.UTBetrayalHUDWidget_Leaderboard"));
	//RequiredHudWidgetClasses.Add(TEXT("/Script/UTBetrayal.UTBetrayalHUDWidget_Pot"));
	RequiredHudWidgetClasses.Add(TEXT("/Script/UTBetrayal.UTBetrayalHUDWidget_TeamInfo"));
	RequiredHudWidgetClasses.Add(TEXT("/Script/UTBetrayal.UTBetrayalHUDWidget_WeaponCrosshair"));
	RequiredHudWidgetClasses.Add(TEXT("/Script/UTBetrayal.UTBetrayalScoreboard"));

	RequiredHudWidgetClasses.Add(TEXT("/Game/RestrictedAssets/UI/HUDWidgets/bpHW_FloatingScore.bpHW_FloatingScore_C"));

	TextDefaultColor = FLinearColor(FColor(255, 255, 128, 255));
	BackgroundDefaultColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);
	BackgroundTeamColor = FLinearColor(0.5f, 0.8f, 10.f, 0.8f);

	static ConstructorHelpers::FObjectFinder<UFont> BeaconFontObj(TEXT("Font'/Engine/EngineFonts/Roboto.Roboto'"));
	BeaconFont = BeaconFontObj.Object != NULL ? BeaconFontObj.Object : MediumFont;
}

void AUTBetrayalHUD::DrawPlayerBeacon(AUTCharacter* P, UCanvas* Canvas, FVector CameraPosition, FVector CameraDir, FVector ScreenLoc)
{
	if (PlayerOwner == NULL || P == NULL || Canvas == NULL || PlayerOwner == P->Controller)
		return;

	AUTGameState* GS = GetWorld()->GetGameState<AUTGameState>();
	AUTBetrayalPlayerState* PRI = Cast<AUTBetrayalPlayerState>(P->PlayerState);
	AUTBetrayalPlayerState* ViewerPRI = Cast<AUTBetrayalPlayerState>(PlayerOwner->PlayerState);
	if (GS == NULL || PRI == NULL || ViewerPRI == NULL)
		return;

	FFontRenderInfo TextRenderInfo = FFontRenderInfo();
	TextRenderInfo.bClipText = true;

	float NameFontScale = 0.8;
	float NumFontScale = 3.0;
	UFont* NameFont = BeaconFont; // TinyFont;
	UFont* NumFont = BeaconFont; // GetFontFromSizeIndex(3); //MediumFont;

	float TextXL, TextYL;
	bool bSameTeam = GS->OnSameTeam(P, PlayerOwner);
	FLinearColor TextColor = TextDefaultColor;
	FLinearColor BeaconColor = bSameTeam ? BackgroundTeamColor : BackgroundDefaultColor;
	FString ScreenName = PRI->PlayerName; // TODO: use player alias instead of PlayerName
	Canvas->TextSize(NameFont, ScreenName, TextXL, TextYL);
	TextXL *= NameFontScale;
	TextYL *= NameFontScale;

	float NumXL, NumYL;
	float Dist = (CameraPosition - P->GetActorLocation()).Size();
	int32 Bonus = PRI->ScoreValueFor(ViewerPRI);
	FString NumString = FString::Printf(TEXT("+%i"), Bonus);

	Canvas->TextSize(NumFont, NumString, NumXL, NumYL);
	float FontScale = FMath::Clamp<float>(800.0f / (Dist + 1.0f), 0.65f, 1.0f);
	FontScale *= NumFontScale;
	NumXL *= FontScale;
	NumYL *= FontScale;

	int AudioHeight = 0;
	int XL = FMath::Max(TextXL, NumXL);
	int YL = TextYL + NumYL;

	// TODO: Add support for Char preview height for speaking player
	//if (CharPRI == PRI)
	//{
	//	AudioHeight = 34 * Canvas.ClipX / 1280;
	//	YL += AudioHeight;
	//}
	
	// Draw Beacon Background
	float BeaconPaddingX = 0.1f * XL;
	float BeaconPaddingY = BeaconPaddingX;
	float BeaconW = XL + 2.0f * BeaconPaddingX;
	float BeaconH = YL + 2.0f * BeaconPaddingY;
	float BeaconPosX = ScreenLoc.X - 0.5f * BeaconW;
	float BeaconPosY = ScreenLoc.Y - BeaconH;
	Canvas->SetLinearDrawColor(BeaconColor);
	Canvas->DrawTile(Canvas->DefaultTexture, BeaconPosX, BeaconPosY, BeaconW, BeaconH, 0, 0, 1, 1);

	// TODO: Add support for Char preview height for speaking player
	//if (CharPRI == PRI)
	//{
	//	AudioWidth = 57 * Canvas->ClipX / 1280;
	//	PulseAudioWidth = AudioWidth * (0.75 + 0.25*sin(6.0*WorldInfo.TimeSeconds));
	//	Canvas.DrawColor = TextColor;
	//	Canvas.SetPos(ScreenLoc.X - 0.5*PulseAudioWidth, ScreenLoc.Y - 1.5*AudioHeight - 1.5*TextYL*FontScale);
	//	Canvas.DrawTile(UT3GHudTexture, PulseAudioWidth, AudioHeight, 173, 132, 57, 34);
	//}

	//FUTCanvasTextItem TextItem(
	//	FVector2D(FMath::TruncToFloat(ScreenLoc.X - 0.5*TextXL), FMath::TruncToFloat(ScreenLoc.Y - 2.5*TextYL*FontScale)),
	//	FText::FromString(ScreenName), 
	//	NameFont, TextColor, NULL
	//);
	//TextItem.Scale = FVector2D(FontScale, FontScale);
	//TextItem.BlendMode = SE_BLEND_Translucent;
	//TextItem.FontRenderInfo = Canvas->CreateFontRenderInfo(true, false);
	//Canvas->DrawItem(TextItem);

	Canvas->SetLinearDrawColor(TextColor);
	Canvas->DrawText(NameFont, ScreenName, ScreenLoc.X - 0.5f*TextXL, ScreenLoc.Y - BeaconPaddingY - TextYL, NameFontScale, NameFontScale, TextRenderInfo);

	FLinearColor BonusTextColor = TextColor;
	if (!bSameTeam)
	{
		if (PRI->bIsRogue && (ViewerPRI->Betrayer == PRI))
		{
			//This pawn is a rogue and betrayed the PC looking at him
			BonusTextColor = RedColor;
		}
		else if (ViewerPRI->bIsRogue && (PRI->Betrayer == ViewerPRI))
		{
			//This pawn is out to get the rogue looking at him
			BonusTextColor = GreenColor;
		}
	}

	// draw value of this player
	Canvas->SetLinearDrawColor(BonusTextColor);
	Canvas->DrawText(NumFont, NumString, ScreenLoc.X - 0.5*NumXL, BeaconPosY + BeaconPaddingY, FontScale, FontScale, TextRenderInfo); // TODO: add AudioHeight

	//Canvas->SetLinearDrawColor(TextColor);
	//Canvas->DrawText(NumFont, NumString, ScreenLoc.X - 0.5*NumXL, ScreenLoc.Y - 2.0*TextYL*FontScale - NumYL - AudioHeight, FontScale, FontScale);

	//TextItem.SetColor(BonusTextColor);
	//FFormatNamedArguments Args;
	//Args.Add("Bonus", FText::AsNumber(Bonus));
	//TextItem.Text = FText::Format(NSLOCTEXT("UTBetrayal", "BeaconBonusDisplay", "+{Bonus}"), Args);

	//TextItem.Position = FVector2D(FMath::TruncToFloat(ScreenLoc.X - 0.5*NumXL), FMath::TruncToFloat(ScreenLoc.Y - 2.0*TextYL*FontScale - NumYL - AudioHeight));
	//Canvas->DrawItem(TextItem);

}
