// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/MassMobManager.h"
#include "Kismet/GameplayStatics.h"
#include "Async/ParallelFor.h" // 병렬 처리를 위한 필수 헤더

AMassMobManager::AMassMobManager()
{
    // 매니저는 매 프레임 계산을 해야 하므로 Tick을 켭니다.
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f; // 매 프레임 실행
}

void AMassMobManager::BeginPlay()
{
    Super::BeginPlay();

    // 플레이어 캐릭터 찾기
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    //오브젝트 풀 초기화
    if (MonsterClass)
    {
        // 메모리 재할당 방지를 위해 미리 예약(Reserve)
        MobActorPool.Reserve(MaxPoolSize);
        MobDataArray.Reserve(MaxPoolSize);

        // 지정된 수만큼 몬스터를 미리 생성해서 비활성 상태로 둠
        for (int32 i = 0; i < MaxPoolSize; ++i)
        {
            FVector TempLoc(0.0f, 0.0f, -10000.0f); // 땅 밑 먼 곳
            FRotator TempRot = FRotator::ZeroRotator;

            // 액터 생성 (지연 생성 방지)
            FActorSpawnParameters SpawnParams;
            SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

            AMassMonster* NewMob = GetWorld()->SpawnActor<AMassMonster>(MonsterClass, TempLoc, TempRot, SpawnParams);

            if (NewMob)
            {
                // 초기 상태: 비활성화 (숨김)
                NewMob->SetActiveState(false);

                // 풀에 등록
                MobActorPool.Add(NewMob);

                // 데이터 구조체 생성 및 매핑
                FMassMobData NewData;
                NewData.ActorIndex = i; // 짝꿍 액터의 번호를 기억
                NewData.bIsActive = false;
                NewData.MoveSpeed = DefaultMoveSpeed;

                MobDataArray.Add(NewData);
            }
        }
    }
}

void AMassMobManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 플레이어가 없으면 아무것도 안 함
    if (!PlayerPawn.IsValid()) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 TotalCount = MobDataArray.Num();

    ParallelFor(TotalCount, [&](int32 Index)
        {
            // 데이터 참조 가져오기
            FMassMobData& CurrentData = MobDataArray[Index];

            // 비활성화된 몬스터는 계산 생략
            if (!CurrentData.bIsActive) return;

            //방향 벡터 계산 (몬스터 -> 플레이어)
            // Z축은 무시 평면 이동만
            FVector DirectionToPlayer = PlayerLocation - CurrentData.Location;
            DirectionToPlayer.Z = 0.0f;

            // 정규화 (거리가 0에 가까우면 안전하게 ZeroVector 반환
            FVector NormalizedDir = DirectionToPlayer.GetSafeNormal2D();

            // 이동할 위치 계산 (P = P0 + V * t)
            CurrentData.Location += NormalizedDir * CurrentData.MoveSpeed * DeltaTime;

            // 바라보는 방향 업데이트
            if (!NormalizedDir.IsNearlyZero())
            {
                CurrentData.Direction = NormalizedDir;
            }
        });


    // SetActorLocation는 메인 스레드에서만 호출 가능하므로 여기서 for문

    for (int32 i = 0; i < TotalCount; ++i)
    {
        const FMassMobData& Data = MobDataArray[i];

        // 활성화된 몬스터만 화면 갱신
        if (Data.bIsActive)
        {
            AMassMonster* TargetActor = MobActorPool[Data.ActorIndex];
            if (TargetActor)
            {
                // MassMonster에 만들어둔 최적화된 이동 함수 호출
                // (Physics 연산 없이 좌표만 텔레포트)
                TargetActor->UpdateTransform(Data.Location, Data.Direction.Rotation());
            }
        }
    }
}

void AMassMobManager::SpawnMonster(const FVector& SpawnLocation)
{
    // 사용 가능한(비활성) 몬스터를 찾아서 소환 (Linear Search)
    // 2000~5000개 정도는 for문으로 찾아도 순식간입니다.
    for (int32 i = 0; i < MobDataArray.Num(); ++i)
    {
        if (!MobDataArray[i].bIsActive)
        {
            // 데이터 활성화
            MobDataArray[i].bIsActive = true;
            MobDataArray[i].Location = SpawnLocation;
            MobDataArray[i].Location.Z = 88.0f; // 바닥 높이 보정

            //액터 활성화
            AMassMonster* TargetActor = MobActorPool[i];
            if (TargetActor)
            {
				UE_LOG(LogTemp, Log, TEXT("Spawning Monster Num: %d"), count++);
                TargetActor->UpdateTransform(SpawnLocation, FRotator::ZeroRotator);
                TargetActor->SetActiveState(true);
            }

            // 하나 찾았으면 즉시 종료
            return;
        }
    }

    // 풀이 꽉 찼을 경우
    // UE_LOG(LogTemp, Warning, TEXT("Mob Pool is Full! Cannot spawn more monsters."));
}

void AMassMobManager::ClearAllMonsters()
{
    // 모든 몬스터 즉시 비활성화
    for (int32 i = 0; i < MobDataArray.Num(); ++i)
    {
        MobDataArray[i].bIsActive = false;

        if (MobActorPool[i])
        {
            MobActorPool[i]->SetActiveState(false);
        }
    }
}
