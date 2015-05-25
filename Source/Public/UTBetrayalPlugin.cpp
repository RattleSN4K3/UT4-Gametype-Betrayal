#include "UTBetrayal.h"

#include "ModuleManager.h"
#include "ModuleInterface.h"

class FUTBetrayalPlugin : public IModuleInterface
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE( FUTBetrayalPlugin, UTBetrayal )

void FUTBetrayalPlugin::StartupModule()
{
	
}

void FUTBetrayalPlugin::ShutdownModule()
{
	
}
