#include "UTBetrayal.h"
#include "UTBetrayalHUDWidget_TeamInfo.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalTeam.h"

UUTBetrayalHUDWidget_TeamInfo::UUTBetrayalHUDWidget_TeamInfo(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	// Structure to hold one-time initialization
	struct FConstructorStatics
	{
		ConstructorHelpers::FObjectFinder<UTexture2D> HudTexture;
		ConstructorHelpers::FObjectFinder<UFont> NameFont;

		FConstructorStatics()
			: HudTexture(TEXT("Texture2D'/UTBetrayal/Textures/HUDIcons.HUDIcons'"))
			, NameFont(TEXT("Font'/Engine/EngineFonts/Roboto.Roboto'"))
		{
		}
	};
	static FConstructorStatics ConstructorStatics;


	Position = FVector2D(-5.0f, 5.0f);
	Size = FVector2D(180.0f, 43.0f);
	ScreenPosition = FVector2D(0.5f, 0.0f);
	Origin = FVector2D(1.0f, 0.0f);


	CoinCountPosition = FVector2D(0.5f, 0.0f);

	NameFontSize = 1;				//font size for teammates
	DaggerWidth = 16;				//width of dagger icon
	DaggerHeight = 28;				//height of dagger icon
	DaggerSpacing = 7.0f;			//spacing between individual daggers
	SilverDaggerOffset = 10.0f;		//spacing between silver and gold daggers
	YFudgeValue = 10;				//spacing between other hud
	NameYPadding = 6.0f;		    //spcaing between top of background and teammate name text
	DaggerYPadding = 3.6f;			//spacing between top of background and dagger icons
	PotValPadding = 7;				//distance between teamnames and potvalue text
	TeammateSpacing = -3.0f;		//distance between individual teammate huds

	DaggersPlateMinWidth = 0.04; // about 76px for 1920 width
	DaggersPlateMaxWidth = 0.07; // about 134px for 1920 width

	DaggerTexCoords = FTextureUVs(262.0f, 53.0f, 16.0f, 28.0f);


	// Temp
	NameplateWidth = 8.0f;			//width of the left/right endcaps
	NameplateBubbleWidth = 15.0f;	//width of the middle divot

	NameplateLeft = FTextureUVs(224.0f, 11.0f, 14.0f, 35.0f);
	NameplateCenter = FTextureUVs(238.0f, 11.0f, 5.0f, 35.0f);
	NameplateBubble = FTextureUVs(243.0f, 11.0f, 26.0f, 35.0f);
	NameplateRight = FTextureUVs(275.0f, 11.0f, 14.0f, 35.0f);

	GoldLinearColor = FLinearColor(1.0f, 1.0f, 0.0f, 1.0f);
	SilverLinearColor = FLinearColor(0.75f, 0.75f, 0.75f, 1.0f);
	BlackBackgroundColor = FLinearColor(0.7f, 0.7f, 0.7f, 0.7f);

	UT3GHudTexture = ConstructorStatics.HudTexture.Object;
	if (ConstructorStatics.NameFont.Object != NULL)
	{
		bOverrideNameFont = true;
		NameFont = ConstructorStatics.NameFont.Object;
	}

	PotString = NSLOCTEXT("UTBetrayalHud", "PotString", "Pot {Pot}");
	RogueString = NSLOCTEXT("UTBetrayalHud", "RogueString", "Rogue {Time}");
	FreelanceString = NSLOCTEXT("UTBetrayalHud", "FreelanceString", "Freelance");
	DaggersPlateString = NSLOCTEXT("UTBetrayalHud", "DaggersPlateString", "x {0}");
}

FVector2D UUTBetrayalHUDWidget_TeamInfo::ResolveHUDPosition(FVector2D Position, float Width, float Height)
{
	// TODO: Pull request

	FVector2D FinalPos;
	FinalPos.X = (Position.X < 0) ? Canvas->ClipX - (Position.X * RenderScale) - (Width * RenderScale) : Position.X * RenderScale;
	FinalPos.Y = (Position.Y < 0) ? Canvas->ClipY - (Position.Y * RenderScale) - (Height * RenderScale) : Position.Y * RenderScale;
	return FinalPos;
}

void UUTBetrayalHUDWidget_TeamInfo::Draw_Implementation(float DeltaTime)
{
	if (UTHUDOwner != NULL)
	{
		// TODO: parameterize Coin-Count width/height
		FVector2D Pos = ResolveHUDPosition(CoinCountPosition, 115.0f, 44.0f);
		//DrawTeamInfo(DeltaTime, Pos);

		DrawTeamInfo(DeltaTime, FVector2D());
	}

#if UE_BUILD_DEBUG

	DrawFontDebugOffline();
	//DrawFontDebugRuntime();

#endif
}

void UUTBetrayalHUDWidget_TeamInfo::DrawTeamInfo(float DeltaTime, FVector2D Pos)
{
	//Early out for wrong gametype
	AUTBetrayalPlayerState* BPRI = UTPlayerOwner != NULL ? Cast<AUTBetrayalPlayerState>(UTPlayerOwner->PlayerState) : NULL;
	if (BPRI == NULL)
	{
		return;
	}

	float XL = 0.0f, YL = 0.0f;
	float MaxTeammmateNameStrWidth = 0.0f;

	FTeammateHudInfo aTeammate;
	TArray<FTeammateHudInfo> HudTeammates;

	UFont* DrawFont = (bOverrideNameFont && NameFont != NULL) ? NameFont : UTHUDOwner->GetFontFromSizeIndex(NameFontSize);

	//Start at top center screen
	float DaggerStartPos = CanvasCenter.X; // TODO: Safe region? // FIXME: UC: (FullWidth * SafeRegionPct) * 0.5;
	Pos.X = DaggerStartPos;
	Pos.Y += YFudgeValue;

	//Determine number of daggers drawn	for your teammates
	if (BPRI->CurrentTeam != NULL)
	{
		for (int32 i = 0; i<ARRAY_COUNT(BPRI->CurrentTeam->Teammates); i++)
		{
			if (BPRI->CurrentTeam->Teammates[i] != NULL && BPRI->CurrentTeam->Teammates[i] != BPRI)
			{
				aTeammate.TeammateName = BPRI->CurrentTeam->Teammates[i]->PlayerName;
				Canvas->StrLen(DrawFont,  FText::FromString(aTeammate.TeammateName).ToString(), aTeammate.TeammateNameStrWidth, YL);

				if (aTeammate.TeammateNameStrWidth > MaxTeammmateNameStrWidth)
				{
					MaxTeammmateNameStrWidth = aTeammate.TeammateNameStrWidth;
				}

				aTeammate.NumRawDaggers = BPRI->CurrentTeam->Teammates[i]->BetrayalCount;
				aTeammate.NumGoldDaggers = aTeammate.NumRawDaggers / 5;
				aTeammate.NumSilverDaggers = aTeammate.NumRawDaggers % 5;

				HudTeammates.Add(aTeammate);
			}
		}
	}

	//Add the betrayer to the list (in red)
	if (BPRI->Betrayer != NULL && BPRI->Betrayer->bIsRogue && (BPRI->Betrayer->RemainingRogueTime > 0))
	{
		aTeammate.TeammateName = BPRI->Betrayer->PlayerName;
		Canvas->StrLen(DrawFont, FText::FromString(aTeammate.TeammateName).ToString(), aTeammate.TeammateNameStrWidth, YL);

		if (aTeammate.TeammateNameStrWidth > MaxTeammmateNameStrWidth)
		{
			MaxTeammmateNameStrWidth = aTeammate.TeammateNameStrWidth;
		}

		aTeammate.NumRawDaggers = BPRI->Betrayer->BetrayalCount;
		aTeammate.NumGoldDaggers = aTeammate.NumRawDaggers / 5;
		aTeammate.NumSilverDaggers = aTeammate.NumRawDaggers % 5;

		HudTeammates.Add(aTeammate);
	}

	Canvas->bCenterX = false;
	Canvas->bCenterY = false;
	Canvas->SetDrawColor(FColor::White);

	//Draw the names of players on your team
	for (int32 i = 0; i<HudTeammates.Num(); i++)
	{
		bool bDrawRaw = false;
		FString PlayerName = HudTeammates[i].TeammateName;
		float PlayerNameWidth = HudTeammates[i].TeammateNameStrWidth;
		int32 NumGoldDaggers = HudTeammates[i].NumGoldDaggers;
		int32 NumSilverDaggers = HudTeammates[i].NumSilverDaggers;
		int32 NumRawDaggers = HudTeammates[i].NumRawDaggers;
		
		//Calculate the width the daggers take up on screen
		float NumDaggersWidth = 0;
		if (NumGoldDaggers > 0)
		{
			//Make room for one gold icon
			NumDaggersWidth += DaggerWidth;

			//Plus the spacing added for each additional
			if (NumGoldDaggers > 1)
			{
				NumDaggersWidth += ((float)(NumGoldDaggers - 1) * DaggerSpacing);
			}
		}

		if (NumSilverDaggers > 0)
		{
			//Add the offset between gold/silver if there are gold and silver
			if (NumGoldDaggers > 0)
			{
				NumDaggersWidth += SilverDaggerOffset;
			}
			else
			{
				//Make room for one silver icon
				NumDaggersWidth += (float)DaggerWidth;
			}

			//Plus the spacing added for each additional
			if (NumSilverDaggers > 1)
			{
				NumDaggersWidth += ((float)(NumSilverDaggers - 1) * DaggerSpacing);
			}
		}

		// override drawing daggers, draw raw number instead
		if (NumDaggersWidth >= DaggersPlateMaxWidth * Canvas->SizeX)
		{
			FText DaggerString = FText::Format(DaggersPlateString, FText::AsNumber(NumRawDaggers));
			float TempXL, TempYL;
			Canvas->TextSize(DrawFont, DaggerString.ToString(), TempXL, TempYL);

			NumDaggersWidth = TempXL + DaggerSpacing + DaggerWidth;
			bDrawRaw = true;
		}
		else
		{
			NumDaggersWidth = FMath::Max<float>(DaggersPlateMinWidth * Canvas->SizeX, NumDaggersWidth);
		}

		//Center of screen
		Pos.X = DaggerStartPos;

		//Draw some sort of bounds around the betrayal details
		DrawTeammateBackground(Pos, MaxTeammmateNameStrWidth, NumDaggersWidth);

		//The last guy in the list is the betrayer (if any)
		if (i == HudTeammates.Num() - 1 && BPRI->Betrayer != NULL && BPRI->Betrayer->bIsRogue)
		{
			Canvas->SetDrawColor(255, 64, 0, 255);

			//Draw the player name to the left of center
			Canvas_DrawText(DrawFont, FText::FromString(PlayerName),
				DaggerStartPos - PlayerNameWidth - (0.5 * NameplateBubbleWidth) * RenderScale,
				Pos.Y + (NameYPadding * RenderScale));

			//Draw remaining time instead of daggers
			Pos.X = DaggerStartPos + (0.5 * NameplateBubbleWidth * RenderScale);
			FText NumberText = FText::AsNumber(BPRI->Betrayer->RemainingRogueTime);
			Canvas_DrawText(DrawFont, NumberText, Pos.X, Pos.Y + (NameYPadding * RenderScale));
		}
		else
		{
			//Draw the player name to the left of center
			Canvas_DrawText(DrawFont, FText::FromString(PlayerName),
				DaggerStartPos - PlayerNameWidth - (0.5 * NameplateBubbleWidth) * RenderScale,
				Pos.Y + (NameYPadding * RenderScale));

			if (bDrawRaw)
			{
				//Draw simple format "x NUM"
				Canvas->DrawColor = FLinearColor::White;
				Canvas->DrawTile(UT3GHudTexture, Pos.X, Pos.Y + (DaggerYPadding * RenderScale),
					DaggerWidth * RenderScale, DaggerHeight * RenderScale,
					DaggerTexCoords.U, DaggerTexCoords.V, DaggerTexCoords.UL, DaggerTexCoords.VL);

				FText DaggerString = FText::Format(DaggersPlateString, FText::AsNumber(NumRawDaggers));
				Canvas->DrawColor = FLinearColor::Gray;
				Canvas_DrawText(DrawFont, DaggerString, Pos.X + (DaggerWidth + DaggerSpacing) * RenderScale, Pos.Y + (NameYPadding * RenderScale));
			}
			else
			{
				//Start drawing the daggers
				Pos.X = DaggerStartPos + (0.5 * NameplateBubbleWidth * RenderScale);
				for (int32 j = 0; j < NumGoldDaggers; j++)
				{
					Canvas->DrawColor = GoldLinearColor;
					Canvas->DrawTile(UT3GHudTexture, Pos.X, Pos.Y + (DaggerYPadding * RenderScale),
						DaggerWidth * RenderScale, DaggerHeight * RenderScale,
						DaggerTexCoords.U, DaggerTexCoords.V, DaggerTexCoords.UL, DaggerTexCoords.VL);

					//Don't bump for the last gold dagger drawn
					if (j<NumGoldDaggers - 1)
					{
						Pos.X += (DaggerSpacing * RenderScale);
					}
				}

				//Add spacing between gold/silver daggers
				if (NumGoldDaggers > 0)
				{
					Pos.X += (SilverDaggerOffset * RenderScale);
				}

				for (int32 j = 0; j < NumSilverDaggers; j++)
				{
					Canvas->DrawColor = SilverLinearColor;
					Canvas->DrawTile(UT3GHudTexture, Pos.X, Pos.Y + (DaggerYPadding * RenderScale),
						DaggerWidth * RenderScale, DaggerHeight * RenderScale, DaggerTexCoords.U, DaggerTexCoords.V, DaggerTexCoords.UL, DaggerTexCoords.VL);

					Pos.X += (DaggerSpacing * RenderScale);
				}
			}
		}

		//Go down some
		Pos.Y += ((NameplateCenter.VL + TeammateSpacing) * RenderScale);
	}

	//Draw the POT string
	FText PotText = FText::GetEmpty();
	FColor PotColor = FColor::White;
	FFormatNamedArguments Args;
	if (BPRI->CurrentTeam != NULL)
	{
		PotColor.R = 64;
		PotColor.G = 128;

		Args.Add(TEXT("Pot"), FText::AsNumber(BPRI->CurrentTeam->TeamPot));
		PotText = FText::Format(PotString, Args);
	}
	else if (BPRI->bIsRogue)
	{
		PotColor.B = 0;
		PotColor.G = 64;

		Args.Add(TEXT("Time"), FText::AsNumber(BPRI->RemainingRogueTime));
		PotText = FText::Format(RogueString, Args);
	}
	else
	{
		PotColor.B = 0;
		PotText = FreelanceString;
	}

	//Draw the pot value / freelance or rogue text
	//DrawFont = UTHUDOwner->GetFontFromSizeIndex(NameFontSize); // already set. // TODO: remove
	Canvas->StrLen(DrawFont, PotText.ToString(), XL, YL);

	Pos.X = DaggerStartPos;
	DrawNameplateBackground(Pos, XL, BlackBackgroundColor);

	//Center the string
	Canvas->SetDrawColor(PotColor);
	Canvas_DrawText(DrawFont, PotText, DaggerStartPos - (0.5 * XL), Pos.Y + (PotValPadding * RenderScale));
	Canvas->SetDrawColor(FColor::White);
}

void UUTBetrayalHUDWidget_TeamInfo::DrawNameplateBackground(FVector2D Pos, float WordWidth, FLinearColor NameplateColor, float WordHeight)
{
	float NameplateHeight = 0;
	if (WordHeight > 0)
	{
		NameplateHeight = WordHeight;
	}
	else
	{
		NameplateHeight = NameplateCenter.VL * RenderScale;
	}

	float EndCapWidth = NameplateWidth * RenderScale;

	//Start to the lft half the length of the text
	float DrawPosX = Pos.X - (0.5 * WordWidth) - EndCapWidth;

	Canvas->DrawColor = NameplateColor;
	Canvas->DrawTile(UT3GHudTexture, DrawPosX, Pos.Y, EndCapWidth, NameplateHeight, NameplateLeft.U, NameplateLeft.V, NameplateLeft.UL, NameplateLeft.VL);
	DrawPosX += EndCapWidth;
	Canvas->DrawTile(UT3GHudTexture, DrawPosX, Pos.Y, WordWidth, NameplateHeight, NameplateCenter.U, NameplateCenter.V, NameplateCenter.UL, NameplateCenter.VL);
	DrawPosX += WordWidth;
	Canvas->DrawTile(UT3GHudTexture, DrawPosX, Pos.Y, EndCapWidth, NameplateHeight, NameplateRight.U, NameplateRight.V, NameplateRight.UL, NameplateRight.VL);
}

void UUTBetrayalHUDWidget_TeamInfo::DrawTeammateBackground(FVector2D Pos, float TeammateNameWidth, float NumDaggersWidth)
{
	float NameplateHeight = NameplateCenter.VL * RenderScale;

	//Start to the left with the player name
	float DrawPosX = Pos.X - TeammateNameWidth - ((NameplateWidth + 0.5 * NameplateBubbleWidth) * RenderScale);

	Canvas->DrawColor = BlackBackgroundColor;

	Canvas->DrawTile(UT3GHudTexture, DrawPosX, Pos.Y, NameplateWidth * RenderScale, NameplateHeight, NameplateLeft.U, NameplateLeft.V, NameplateLeft.UL, NameplateLeft.VL);
	DrawPosX += NameplateWidth * RenderScale;
	Canvas->DrawTile(UT3GHudTexture, DrawPosX, Pos.Y, TeammateNameWidth, NameplateHeight, NameplateCenter.U, NameplateCenter.V, NameplateCenter.UL, NameplateCenter.VL);
	DrawPosX += TeammateNameWidth;

	Canvas->DrawTile(UT3GHudTexture, DrawPosX, Pos.Y, NameplateBubbleWidth * RenderScale, NameplateHeight, NameplateBubble.U, NameplateBubble.V, NameplateBubble.UL, NameplateBubble.VL);
	DrawPosX += NameplateBubbleWidth * RenderScale;
	Canvas->DrawTile(UT3GHudTexture, DrawPosX, Pos.Y, NumDaggersWidth * RenderScale, NameplateHeight, NameplateCenter.U, NameplateCenter.V, NameplateCenter.UL, NameplateCenter.VL);
	DrawPosX += NumDaggersWidth * RenderScale;

	Canvas->DrawTile(UT3GHudTexture, DrawPosX, Pos.Y, NameplateWidth * RenderScale, NameplateHeight, NameplateRight.U, NameplateRight.V, NameplateRight.UL, NameplateRight.VL);
}

float UUTBetrayalHUDWidget_TeamInfo::Canvas_DrawText(const UFont* InFont, const FString& InText, float X, float Y, float XScale, float YScale, const FFontRenderInfo& RenderInfo)
{
	return Canvas_DrawText(InFont, FText::FromString(InText), X, Y, XScale, YScale, RenderInfo);
}

float UUTBetrayalHUDWidget_TeamInfo::Canvas_DrawText(const UFont* InFont, const FText& InText, float X, float Y, float XScale, float YScale, const FFontRenderInfo& RenderInfo)
{
#if UE_BUILD_DEBUG
	// draw additional line on top of the text, showing the alignment
	float XL, YL;
	Canvas->StrLen(InFont, InText.ToString(), XL, YL);
	Canvas->K2_DrawLine(FVector2D(X, Y), FVector2D(X + XL, Y), 1.5f, FLinearColor::Red);
#endif

	return Canvas->DrawText(InFont, InText, X, Y, XScale, YScale, RenderInfo);
}

#if UE_BUILD_DEBUG

void UUTBetrayalHUDWidget_TeamInfo::DrawFontDebugOffline()
{
	FVector2D BasePos = FVector2D(10.0f, 10.0f);
	FVector2D DrawPos = BasePos;
	float PaddingX = 10.0f;
	float PaddingY = 5.0f;
	float FontSize = 36.0f;
	float FontScale = 1.0;
	float MaxWidth = 0.0f;
	for (auto Font : Fonts)
	{
		if (Font == NULL || Font->IsPendingKill()) continue;

		// skip runtime fonts
		//if (Font->FontCacheType == EFontCacheType::Runtime) continue;

		float XL, YL;

		FFormatNamedArguments Args;
		Args.Add(TEXT("FontName"), FText::FromString(Font->ImportOptions.FontName));
		Args.Add(TEXT("FontAsset"), FText::FromString(Font->GetName()));
		FText FontName = FText::Format(FText::FromString(TEXT("{FontName} ({FontAsset})")), Args);

		Canvas->TextSize(Font, FontName.ToString(), XL, YL);
		//Canvas->StrLen(Font, FontName.ToString(), XL, YL);
		FontScale = YL == 0.0 ? 1.0 : FontSize / YL;
		XL *= FontScale;
		YL *= FontScale;

		if (DrawPos.Y + YL + PaddingY > Canvas->ClipY)
		{
			DrawPos.X += MaxWidth + PaddingX;
			DrawPos.Y = BasePos.Y;
			MaxWidth = 0.0;
		}
		MaxWidth = FMath::Max<float>(MaxWidth, XL);

		Canvas->K2_DrawLine(DrawPos, DrawPos + FVector2D(XL, 0.0f));
		Canvas->K2_DrawLine(DrawPos + FVector2D(0.0f, YL), DrawPos + FVector2D(XL, YL));
		Canvas->DrawText(Font, FontName, DrawPos.X, DrawPos.Y, FontScale, FontScale);
		DrawPos.Y += YL + PaddingY;
	}
}

void UUTBetrayalHUDWidget_TeamInfo::DrawFontDebugRuntime()
{
	FVector2D BasePos = FVector2D(10.0f, 10.0f);
	FVector2D DrawPos = BasePos;
	float PaddingX = 10.0f;
	float PaddingY = 5.0f;
	float MaxWidth = 0.0f;

	FFontRenderInfo TextRenderInfo = FFontRenderInfo();
	TextRenderInfo.bClipText = true;
	float FontScale = 1.0f;

	for (auto Font : Fonts)
	{
		if (Font == NULL || Font->IsPendingKill()) continue;

		// skip offline fonts
		if (Font->FontCacheType != EFontCacheType::Runtime) continue;

		float XL, YL;

		FFormatNamedArguments Args;
		Args.Add(TEXT("FontName"), FText::FromString(Font->ImportOptions.FontName));
		Args.Add(TEXT("FontAsset"), FText::FromString(Font->GetName()));
		FText FontName = FText::Format(FText::FromString(TEXT("{FontName} ({FontAsset})")), Args);

		Canvas->StrLen(Font, FontName.ToString(), XL, YL);
		if (DrawPos.Y + YL + PaddingY > Canvas->ClipY)
		{
			DrawPos.X += MaxWidth + PaddingX;
			DrawPos.Y = BasePos.Y;
			MaxWidth = 0.0;
		}
		MaxWidth = FMath::Max<float>(MaxWidth, XL);

		Canvas->K2_DrawLine(DrawPos, DrawPos + FVector2D(XL, 0.0f));
		Canvas->K2_DrawLine(DrawPos + FVector2D(0.0f, YL), DrawPos + FVector2D(XL, YL));
		Canvas->DrawText(Font, FontName, DrawPos.X, DrawPos.Y, FontScale, FontScale, TextRenderInfo);
		DrawPos.Y += YL + PaddingY;
	}
}

#endif