#pragma once

#include "UTBetrayal.h"
#include "UTBetrayalCharacter.generated.h"

// Betrayal Character is the main character class for Betrayal which is handling all
// the logic. The 2 Blueprint character classes BaseUTCharacter_Betrayal and DefaultCharacter_Betrayal
// are plain copies of the non-Betrayal classes where BaseUTCharacter_Betrayal has this class parent
// class.

// In order to create clean copies, first duplicate BaseUTCharacter and reparent to UTBetrayalCharacter. Then duplicate
// DefaultCharacter and reparent to BaseUTCharacter_Betrayal.

UCLASS()
class AUTBetrayalCharacter : public AUTCharacter
{
	GENERATED_UCLASS_BODY()

	/** set when character materials are already swapped once to team materials */
	bool bTeamMaterialHookedOnce;

	virtual void PostRenderFor(APlayerController *PC, UCanvas *Canvas, FVector CameraPosition, FVector CameraDir) override;

	/** Checks whether the character is in line-of-sight */
	virtual bool IsPawnVisible(APlayerController* PC, FVector CameraPosition);

	virtual void ApplyCharacterData(TSubclassOf<class AUTCharacterContent> Data) override;

	virtual void SetBodyColorFlash(const UCurveLinearColor* ColorCurve, bool bRimOnly) override;
	virtual void UpdateBodyColorFlash(float DeltaTime) override;

	/** Updates player color according to the relation of the local player */
	virtual void UpdateTeamColor();
	/** Apply player color for the given team */
	virtual void ApplyTeamColorFor(bool bIsTeam);

};