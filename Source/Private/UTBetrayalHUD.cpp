#include "UTBetrayal.h"
#include "UTBetrayalHUD.h"
#include "UTBetrayalPlayerState.h"
#include "UTHUD_DM.h"

// TODO: Use FUTCanvasTextItem to draw beacon
// TODO: Split Pot from TeamInfo once widgets are fully customizable

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

	static ConstructorHelpers::FObjectFinder<UTexture2D> UT3GHudTextureObj(TEXT("Texture2D'/UTBetrayal/Textures/HUDIcons.HUDIcons'"));
	UT3GHudTexture = UT3GHudTextureObj.Object;

	BeaconTextureUV = FTextureUVs(137.0, 91.0, 101.0, 34.0);

	BeaconBonusString = NSLOCTEXT("UTBetrayal", "BeaconBonusDisplay", "+{Bonus}");
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

	FFontRenderInfo TextRenderInfo = Canvas->CreateFontRenderInfo(true, false);

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
	FFormatNamedArguments Args;
	Args.Add("Bonus", FText::AsNumber(Bonus));
	FString NumString = FText::Format(BeaconBonusString, Args).ToString();

	Canvas->TextSize(NumFont, NumString, NumXL, NumYL);
	float FontScale = FMath::Clamp<float>(800.0f / (Dist + 1.0f), 0.65f, 1.0f);
	FontScale *= NumFontScale;
	NumXL *= FontScale;
	NumYL *= FontScale;

	int AudioHeight = 0;
	int XL = FMath::Max(TextXL, NumXL);
	int YL = TextYL + NumYL;

	// fade out beacons near crosshair and on distance
	float AlphaDist = FMath::Clamp<float>((Dist - 1000.0f) / 2000.0f, 0.33f, 1.0f);
	FVector CenterScreen(Canvas->SizeX*0.5, Canvas->SizeY*0.5, 0.0);
	float AlphaFov = FMath::Clamp<float>((ScreenLoc-CenterScreen).Size2D() / (0.25*FMath::Min<int32>(Canvas->SizeX, Canvas->SizeY)), 0.33f, 1.0f);
	float AlphaValue = AlphaDist * AlphaFov;
	TextColor.A *= AlphaValue;
	BeaconColor.A *= AlphaValue;

	// TODO: Add support for Char preview height for speaking player
	//if (CharPRI == PRI)
	//{
	//	AudioHeight = 34 * Canvas.ClipX / 1280;
	//	YL += AudioHeight;
	//}
	
	// Draw Beacon Background
	float BeaconPaddingX = 8.0f;
	float BeaconPaddingY = BeaconPaddingX;
	float BeaconW = XL + 4.0f * BeaconPaddingX;
	float BeaconH = YL + 2.0f * BeaconPaddingY;
	float BeaconPosX = ScreenLoc.X - 0.5f * BeaconW;
	float BeaconPosY = ScreenLoc.Y - BeaconH;
	Canvas->SetLinearDrawColor(FLinearColor(BeaconColor.R * 0.25, BeaconColor.R * 0.25, BeaconColor.R * 0.25, BeaconColor.A));
	Canvas->DrawTile(UT3GHudTexture, BeaconPosX, BeaconPosY - 0.4*BeaconH, BeaconW, 1.8*BeaconH, BeaconTextureUV.U, BeaconTextureUV.V, BeaconTextureUV.UL, BeaconTextureUV.VL);

	// TODO: Add support for Char preview height for speaking player
	//if (CharPRI == PRI)
	//{
	//	AudioWidth = 57 * Canvas->ClipX / 1280;
	//	PulseAudioWidth = AudioWidth * (0.75 + 0.25*sin(6.0*WorldInfo.TimeSeconds));
	//	Canvas.DrawColor = TextColor;
	//	Canvas.SetPos(ScreenLoc.X - 0.5*PulseAudioWidth, ScreenLoc.Y - 1.5*AudioHeight - 1.5*TextYL*FontScale);
	//	Canvas.DrawTile(UT3GHudTexture, PulseAudioWidth, AudioHeight, 173, 132, 57, 34);
	//}

	Canvas->SetLinearDrawColor(TextColor);
	Canvas->DrawText(NameFont, ScreenName, ScreenLoc.X - 0.5f*TextXL, ScreenLoc.Y - BeaconPaddingY - TextYL, NameFontScale, NameFontScale, TextRenderInfo);

	FLinearColor BonusTextColor = TextColor;
	if (!bSameTeam)
	{
		if (PRI->bIsRogue && (ViewerPRI->Betrayer == PRI))
		{
			//This pawn is a rogue and betrayed the PC looking at him
			BonusTextColor = RedColor;
			BonusTextColor.A += 0.5;
		}
		else if (ViewerPRI->bIsRogue && (PRI->Betrayer == ViewerPRI))
		{
			//This pawn is out to get the rogue looking at him
			BonusTextColor = GreenColor;
			BonusTextColor.A += 0.5;
		}
	}

	// draw value of this player
	Canvas->SetLinearDrawColor(BonusTextColor);
	Canvas->DrawText(NumFont, NumString, ScreenLoc.X - 0.5*NumXL, BeaconPosY + BeaconPaddingY, FontScale, FontScale, TextRenderInfo); // TODO: add AudioHeight
}
