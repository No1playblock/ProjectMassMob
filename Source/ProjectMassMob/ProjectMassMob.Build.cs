// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectMassMob : ModuleRules
{
    public ProjectMassMob(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "AIModule",
            "NavigationSystem",
            "StateTreeModule",
            "GameplayStateTreeModule",
            "Niagara",
            "UMG",
            "Slate",

            "MassEntity",       
            "MassCommon",       
            "MassMovement",     
            "MassActors",      
            "MassSpawner",      
            "MassRepresentation", 
            "MassSignals",      
            "StructUtils"      
		
    });

        PrivateDependencyModuleNames.AddRange(new string[] { });

        PublicIncludePaths.AddRange(new string[] {
            "ProjectMassMob",
            "ProjectMassMob/Variant_Strategy",
            "ProjectMassMob/Variant_Strategy/UI",
            "ProjectMassMob/Variant_TwinStick",
            "ProjectMassMob/Variant_TwinStick/AI",
            "ProjectMassMob/Variant_TwinStick/Gameplay",
            "ProjectMassMob/Variant_TwinStick/UI"
        });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
