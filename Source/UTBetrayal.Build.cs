namespace UnrealBuildTool.Rules
{
	public class UTBetrayal : ModuleRules
	{
		public UTBetrayal(TargetInfo Target)
		{
			PrivateIncludePaths.Add("UTBetrayal/Private");

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"UnrealTournament",
					"InputCore",
					"SlateCore",
					"AIModule", // For Bot subclassing
				}
			);
			if (Target.Type != TargetRules.TargetType.Server)
			{
				PublicDependencyModuleNames.AddRange(new string[] { "Slate" });
			}
		}
	}
}
