// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/CrowdSpawner.h"
#include "CrowdFinalManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ACrowdSpawner::ACrowdSpawner()
{
	PrimaryActorTick.bCanEverTick = false; // 틱 안 씀
}

void ACrowdSpawner::BeginPlay()
{
	Super::BeginPlay();

	// 매니저 찾기
	if (!Manager)
	{
		AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), ACrowdFinalManager::StaticClass());
		Manager = Cast<ACrowdFinalManager>(Found);
	}

	// 대량 소환
	if (Manager && InitialCount > 0)
	{

		Manager->InitializePool();
		for (int32 i = 0; i < InitialCount; ++i)
		{
			FVector Pos;
			if (GetRandomSpawnLocation(Pos))
			{
				Manager->SpawnUnit(Pos);
			}
		}
	}
}

bool ACrowdSpawner::GetRandomSpawnLocation(FVector& OutLocation)
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) return false;

	FVector PLoc = Player->GetActorLocation();
	FVector Dir = FMath::VRand();
	Dir.Z = 0.0f; Dir.Normalize();

	FVector TargetPos = PLoc + (Dir * FMath::RandRange(MinDist, MaxDist));

	UNavigationSystemV1* Nav = UNavigationSystemV1::GetCurrent(GetWorld());
	if (Nav)
	{
		FNavLocation Res;
		// 검색 반경을 줄여서 부하 감소
		if (Nav->ProjectPointToNavigation(TargetPos, Res, FVector(200, 200, 200)))
		{
			OutLocation = Res.Location;
			return true;
		}
	}
	return false;
}

