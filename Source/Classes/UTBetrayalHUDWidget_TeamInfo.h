#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalHUDWidget_TeamInfo.generated.h"

struct FTeammateHudInfo
{
	FString TeammateName;
	float TeammateNameStrWidth;
	int32 NumGoldDaggers;
	int32 NumSilverDaggers;
	int32 NumRawDaggers;
};

UCLASS()
class UUTBetrayalHUDWidget_TeamInfo : public UUTHUDWidget
{
	GENERATED_UCLASS_BODY()

	/** Coin count at the time of the previous display */
	UPROPERTY()
	int32 LastCoinCount;

	// TODO: Unused (even in UT3 code). Remove.
	/** Position to draw the player's current coin count */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "General")
	FVector2D CoinCountPosition;

	/** width of dagger icon  **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	int32 DaggerWidth;

	/** height of dagger icon  **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	int32 DaggerHeight;

	/** spacing between individual daggers **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float DaggerSpacing;

	/** spacing between silver and gold daggers  **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float SilverDaggerOffset;

	/** spacing between top of background and names **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float NameYPadding;

	/** spacing between top of background and daggers **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float DaggerYPadding;

	/** spacing between individual teammates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float TeammateSpacing;

	/** Max width of the daggers plate (percentage to current screen width) **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float DaggersPlateMaxWidth;

	/** Min width of the daggers plate (percentage to current screen width) **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float DaggersPlateMinWidth;

	/** font size for teammates **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render", Meta = (EditCondition = "!bOverrideNameFont"))
	int32 NameFontSize;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	bool bOverrideNameFont;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render", Meta = (EditCondition = "bOverrideNameFont"))
	UFont* NameFont;

	/** spacing from above hud **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	int32 YFudgeValue;

	/** padding for the last row of text **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	int32 PotValPadding;

	/** Coordinates of dagger texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	FTextureUVs DaggerTexCoords;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Message")
	FText PotString;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Message")
	FText RogueString;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Message")
	FText FreelanceString;

	// TODO: Use from UTHUD

	/** width of background on either side of the nameplate */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	float NameplateWidth;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	float NameplateBubbleWidth;

	/** Coordinates of the nameplate background*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	FTextureUVs NameplateLeft;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	FTextureUVs NameplateCenter;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	FTextureUVs NameplateBubble;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	FTextureUVs NameplateRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	FLinearColor GoldLinearColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	FLinearColor SilverLinearColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	FLinearColor BlackBackgroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	UTexture* UT3GHudTexture;


public:

	virtual void Draw_Implementation(float DeltaTime);

protected:

	// TODO: Remove debug code
	float Canvas_DrawText(const UFont* InFont, const FString& InText, float X, float Y, float XScale = 1.f, float YScale = 1.f, const FFontRenderInfo& RenderInfo = FFontRenderInfo());
	float Canvas_DrawText(const UFont* InFont, const FText& InText, float X, float Y, float XScale = 1.f, float YScale = 1.f, const FFontRenderInfo& RenderInfo = FFontRenderInfo());

#if UE_BUILD_DEBUG

	void DrawFontDebugOffline();
	void DrawFontDebugRuntime();
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<UFont*> Fonts;

#endif

	virtual void DrawTeamInfo(float DeltaTime, FVector2D Pos);

	virtual FVector2D ResolveHUDPosition(FVector2D Position, float Width, float Height);

	/**
	*   Draws a nameplate behind text
	*
	* @param Pos				top center of the nameplate
	* @param Wordwidth			width the name takes up (already accounts for resolution)
	* @param NameplateColor		linear color for the background texture
	* @param WordHeight			height of the nameplate (already accounts for resolution)
	*/
	virtual void DrawNameplateBackground(FVector2D Pos, float WordWidth, FLinearColor NameplateColor, float WordHeight = 0.0);
	
	/**
	* Draws the nameplate behind the teammate names/daggers
	*
	* @param Pos					center of the 'hud'
	* @param TeammateNameWidth		width the name takes up (already accounts for resolution)
	* @param NumDaggersWidth		width the betrayal daggers take up
	*/
	virtual void DrawTeammateBackground(FVector2D Pos, float TeammateNameWidth, float NumDaggersWidth);

protected:

};