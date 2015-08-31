#pragma once

#include "UTBetrayal.h"
#include "UTHUDWidgetMessage_KillIconMessages.h"
#include "UTBetrayalHUDWidget_KillIconMessages.generated.h"

UCLASS()
class UUTBetrayalHUDWidget_KillIconMessages : public UUTHUDWidgetMessage_KillIconMessages
{
	GENERATED_UCLASS_BODY()

protected:
	virtual void DrawMessages(float DeltaTime) override;
};