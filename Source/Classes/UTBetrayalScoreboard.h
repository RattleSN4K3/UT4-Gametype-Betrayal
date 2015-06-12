#pragma once

#include "UTBetrayal.h"
//#include "UTScoreboard.h"
#include "UTBetrayalPlayerState.h"
#include "UTBetrayalScoreboard.generated.h"

UCLASS()
class UUTBetrayalScoreboard : public UUTScoreboard
{
	GENERATED_UCLASS_BODY()

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

protected:

	virtual void DrawScoreHeaders(float RenderDelta, float& DrawY) override;
	virtual void DrawPlayer(int32 Index, AUTPlayerState* PlayerState, float RenderDelta, float XOffset, float YOffset) override;

	virtual void DrawDaggers(AUTBetrayalPlayerState* PRI, float RenderDelta, float XOffset, float YOffset);

};