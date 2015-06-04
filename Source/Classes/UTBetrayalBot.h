#pragma once

#include "UTBetrayal.h"
#include "UTBot.h"
#include "UTBetrayalBot.generated.h"

UCLASS()
class AUTBetrayalBot : public AUTBot
{
	GENERATED_UCLASS_BODY()

private:

	bool bPickProcessing;

public:

	/** whether this bot should try to betray a team member */
	UPROPERTY()
	bool bBetrayTeam;

	//virtual void UpdateEnemyInfo(APawn* NewEnemy, EAIEnemyUpdateType UpdateType) override;
	virtual bool IsImportantEnemyUpdate(APawn* TestEnemy, EAIEnemyUpdateType UpdateType) override;
	virtual void PickNewEnemy();

	UFUNCTION()
	virtual bool GetPawnByPRI(AUTPlayerState* PRI, APawn*& P);

};