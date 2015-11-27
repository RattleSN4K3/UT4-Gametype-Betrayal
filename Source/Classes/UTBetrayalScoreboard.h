#pragma once

#include "UTBetrayal.h"
//#include "UTScoreboard.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalScoreboard.generated.h"

UCLASS()
class UUTBetrayalScoreboard : public UUTScoreboard
{
	GENERATED_UCLASS_BODY()

	// Begin UUTScoreboard Interface.
	virtual FLinearColor GetPlayerColorFor(AUTPlayerState* InPS) const override;
protected:
	virtual void DrawScoreHeaders(float RenderDelta, float& DrawY) override;
	virtual void DrawPlayer(int32 Index, AUTPlayerState* PlayerState, float RenderDelta, float XOffset, float YOffset) override;
	// End UUTScoreboard Interface

	// Begin UUTHUDWidget Interface.
public:
	virtual FString GetClampedName(AUTPlayerState* PS, UFont* NameFont, float NameScale, float MaxWidth);
	// End UUTHUDWidget Interface


	// Custom properties below

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoreboard")
	float ColumnHeaderKillsX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
	FColor AllyColor;

	/** Coordinates of dagger texture */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	FTextureUVs DaggerTexCoords;

	// UNUSED: Remove. Daggers are scaled to column height
	/** width of dagger icon  **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	int32 DaggerWidth;

	// UNUSED: Remove. Daggers are scaled to column height
	/** height of dagger icon  **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	int32 DaggerHeight;

	/** spacing between individual daggers **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float DaggerSpacing;

	/** spacing between player name name and daggers **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float DaggerXPadding;

	/** spacing between silver and gold daggers  **/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render")
	float SilverDaggerOffset;

	// TODO: Use from UTHUD

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	FLinearColor GoldLinearColor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	FLinearColor SilverLinearColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render UT3 HUD")
	UTexture* UT3GHudTexture;

private:

	/** Last drawn name used to calculate size for drawing diggers directly after the PlayerName */
	FString LastPlayerName;

protected:

	virtual void DrawDaggers(AUTBetrayalPlayerState* PRI, float RenderDelta, float XOffset, float YOffset);

};