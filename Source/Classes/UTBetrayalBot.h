#pragma once

#include "UTBetrayal.h"
#include "UTBot.h"
#include "UTBetrayalBot.generated.h"

UCLASS()
class AUTBetrayalBot : public AUTBot
{
	GENERATED_UCLASS_BODY()

	/** whether this bot should try to betray a team member */
	UPROPERTY()
	bool bBetrayTeam;

	/** return whether passed in Actor is or belongs to a teammate */
	virtual bool IsTeammate(AActor* TestActor) override;
	/** rating for enemy to focus on bot targets enemy with highest rating */
	virtual float RateEnemy(const FBotEnemyInfo& EnemyInfo) override;

};