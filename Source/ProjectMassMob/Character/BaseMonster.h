// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseMonster.generated.h"


DECLARE_STATS_GROUP(TEXT("MonsterStats"), STATGROUP_Monster, STATCAT_Advanced);

DECLARE_CYCLE_STAT(TEXT("Monster AI Move Logic"), STAT_Monster_MoveLogic, STATGROUP_Monster);

DECLARE_CYCLE_STAT(TEXT("Monster Total Tick"), STAT_Monster_Tick, STATGROUP_Monster);

UCLASS()
class PROJECTMASSMOB_API ABaseMonster : public ACharacter
{
	GENERATED_BODY()

public:
    ABaseMonster();

    // 풀에서 꺼낼 때 초기화하는 함수
    void Activate(FVector StartLocation);

    // 풀로 되돌릴 때 비활성화하는 함수
    void Deactivate();

    // 몹 고유 인덱스 (매니저 관리용)
    int32 PoolIndex;

    // LOD 레벨 설정 (0: High, 1: Low, 2: Sleep)
    void SetLODLevel(int32 Level);

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, Category = "Settings")
    int32 Interval_High = 1; // 가까운 유닛은 매 프레임 이동

    UPROPERTY(EditAnywhere, Category = "Settings")
    int32 Interval_Low = 10;  // 중간 유닛은 3프레임에 한 번 이동

    UPROPERTY(EditAnywhere, Category = "Settings")
    int32 Interval_Far = 30;  // 먼 유닛은 5프레임에 한 번 이동

    //void Tick(float DeltaTime);

    void CheckGridLocation();
private:
    // AI 컨트롤러 캐싱 (매번 GetController 방지)
    UPROPERTY()
    class AAIController* MyController;

    void MoveToPlayer();

	FTimerHandle MoveTimerHandle;

    // 이전 프레임 위치 저장용
    FVector LastGridLocation;

    // 매니저 포인터 캐싱 (BeginPlay에서 설정 필요)
    UPROPERTY()
    TObjectPtr<class AMobPoolManager> PoolManager;

    FTimerHandle GridCheckTimerHandle;
    int32 CurrentLODLevel = -1;

    UPROPERTY()
    AActor* TargetPlayer = nullptr;
};
