// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/MobSpawner.h"
#include "MassMobManager.h" // 매니저 헤더
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h" // 땅 찾기용

AMobSpawner::AMobSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMobSpawner::BeginPlay()
{
    Super::BeginPlay();

    // 레벨에 배치된 MassMobManager를 자동으로 찾아서 연결
    if (!MobManager)
    {
        AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMassMobManager::StaticClass());
        MobManager = Cast<AMassMobManager>(FoundActor);
    }
}

void AMobSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 매니저가 없으면 아무것도 못함
    if (!MobManager) return;

    TimeSinceLastSpawn += DeltaTime;

    // 스폰 주기가 되었는가?
    if (TimeSinceLastSpawn >= SpawnInterval)
    {
        TimeSinceLastSpawn = 0.0f;

        // 설정한 마리 수만큼 소환 시도
        for (int32 i = 0; i < SpawnCountPerInterval; ++i)
        {
            FVector SpawnPos;

            // 유효한 땅 위치를 찾았다면?
            if (GetRandomSpawnLocation(SpawnPos))
            {
                // 매니저에게 "여기 소환해!" 명령
                MobManager->SpawnMonster(SpawnPos);
            }
        }
    }
}

bool AMobSpawner::GetRandomSpawnLocation(FVector& OutLocation)
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return false;

    FVector PlayerLoc = PlayerPawn->GetActorLocation();

    // 1. 랜덤 방향 구하기
    FVector RandomDir = FMath::VRand();
    RandomDir.Z = 0.0f; // 높이는 무시 (평면)
    RandomDir.Normalize();

    // 2. 랜덤 거리 구하기 (도넛 모양: 최소 ~ 최대 사이)
    float RandomDist = FMath::RandRange(MinSpawnDistance, MaxSpawnDistance);

    // 3. 임시 목표 지점 계산
    FVector RandomPoint = PlayerLoc + (RandomDir * RandomDist);

    // 4. 네비게이션 시스템을 이용해 "진짜 땅" 위치 찾기
    // (이걸 안 하면 벽 속이나 허공에 몬스터가 낌)
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation ResultLocation;
        // 목표 지점 반경 500.0f 안에서 가장 가까운 NavMesh 위 좌표 찾기
        if (NavSys->ProjectPointToNavigation(RandomPoint, ResultLocation, FVector(500.0f, 500.0f, 500.0f)))
        {
            OutLocation = ResultLocation.Location;
            return true; // 성공
        }
    }

    return false; // 땅을 못 찾음 (이번엔 스폰 패스)
}

