// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/SkeletalMobSpawner.h"
#include "Prop/SkeletalMassMobManager.h" // 매니저 헤더
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ASkeletalMobSpawner::ASkeletalMobSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ASkeletalMobSpawner::BeginPlay()
{
	Super::BeginPlay();

	if (!MobManager)
	{
		// [변경점] SkeletalMassMobManager 찾기
		AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), ASkeletalMassMobManager::StaticClass());
		MobManager = Cast<ASkeletalMassMobManager>(FoundActor);
	}

	// [추가됨] 초기 대량 소환 로직
	if (MobManager && InitialSpawnCount > 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("=== Initial Spawn Started: %d Mobs ==="), InitialSpawnCount);

		for (int32 i = 0; i < InitialSpawnCount; ++i)
		{
			FVector SpawnPos;
			// 랜덤 위치 찾기에 성공하면 소환
			if (GetRandomSpawnLocation(SpawnPos))
			{
				MobManager->SpawnMonster(SpawnPos);
			}
		}

		UE_LOG(LogTemp, Warning, TEXT("=== Initial Spawn Finished ==="));
	}
}

void ASkeletalMobSpawner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!MobManager) return;

	TimeSinceLastSpawn += DeltaTime;

	if (TimeSinceLastSpawn >= SpawnInterval)
	{
		TimeSinceLastSpawn = 0.0f;

		for (int32 i = 0; i < SpawnCountPerInterval; ++i)
		{
			FVector SpawnPos;
			if (GetRandomSpawnLocation(SpawnPos))
			{
				MobManager->SpawnMonster(SpawnPos);
			}
		}
	}
}

bool ASkeletalMobSpawner::GetRandomSpawnLocation(FVector& OutLocation)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!PlayerPawn) return false;

	FVector PlayerLoc = PlayerPawn->GetActorLocation();

	FVector RandomDir = FMath::VRand();
	RandomDir.Z = 0.0f;
	RandomDir.Normalize();

	float RandomDist = FMath::RandRange(MinSpawnDistance, MaxSpawnDistance);
	FVector RandomPoint = PlayerLoc + (RandomDir * RandomDist);

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	if (NavSys)
	{
		FNavLocation ResultLocation;
		if (NavSys->ProjectPointToNavigation(RandomPoint, ResultLocation, FVector(500.0f, 500.0f, 500.0f)))
		{
			OutLocation = ResultLocation.Location;
			return true;
		}
	}

	return false;
}

