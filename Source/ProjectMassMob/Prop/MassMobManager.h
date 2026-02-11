// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actor/MassMonster.h"
#include "MassMobManager.generated.h"


struct FMassMobData
{
    FVector Location;       // 현재 위치
    FVector Direction;      // 바라보는 방향 (회전용)
    float MoveSpeed;        // 이동 속도
    bool bIsActive;         // 활성화 여부 (True면 화면에 보이고 움직임)
    int32 ActorIndex;       // 실제 액터(MassMonster)가 배열의 몇 번째에 있는지 저장

    // 생성자 (기본값 초기화)
    FMassMobData()
        : Location(FVector::ZeroVector)
        , Direction(FVector::ForwardVector)
        , MoveSpeed(0.0f)
        , bIsActive(false)
        , ActorIndex(-1)
    {
    }
};

UCLASS()
class PROJECTMASSMOB_API AMassMobManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMassMobManager();

protected:
    virtual void BeginPlay() override;

public:
    // 매니저가 유일하게 Tick을 돌며 모든 몬스터를 제어합니다.
    virtual void Tick(float DeltaTime) override;

    // 외부(스포너 등)에서 몬스터 소환을 요청할 때 사용하는 함수
    UFUNCTION(BlueprintCallable, Category = "MassMob")
    void SpawnMonster(const FVector& SpawnLocation);

    // 모든 몬스터 즉시 제거 (게임 오버 시 등)
    UFUNCTION(BlueprintCallable, Category = "MassMob")
    void ClearAllMonsters();

public:
    // 스폰할 몬스터 블루프린트 클래스 (BP_MassMonster)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TSubclassOf<AMassMonster> MonsterClass;

    // 풀링할 최대 몬스터 수 (예: 2000 ~ 5000)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxPoolSize = 2000;

    // 몬스터 기본 이동 속도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DefaultMoveSpeed = 300.0f;

private:
    // [데이터 배열] CPU 캐시 효율(Cache Hit)을 위해 데이터를 일렬로 배치합니다.
    // ParallelFor 계산 시 이 배열만 건드립니다.
    TArray<FMassMobData> MobDataArray;

    // [액터 배열] 실제 눈에 보이는 껍데기(Actor)들을 저장합니다.
    // 계산이 끝난 후, 이 액터들의 위치를 업데이트합니다.
    UPROPERTY()
    TArray<AMassMonster*> MobActorPool;

    // 플레이어(타겟) 참조 캐싱
    TWeakObjectPtr<APawn> PlayerPawn;

    int count = 0;
};
