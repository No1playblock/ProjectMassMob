// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MobPoolManager.generated.h"

UCLASS()
class PROJECTMASSMOB_API AMobPoolManager : public AActor
{
	GENERATED_BODY()
	
public:
    AMobPoolManager();

protected:
    virtual void BeginPlay() override;

	void Tick(float DeltaTime) override;

    void UpdateMobLOD();
public:
    // 풀링 사이즈
    UPROPERTY(EditAnywhere, Category = "Config")
    int32 PoolSize = 500;

    UPROPERTY(EditAnywhere, Category = "Config")
    float CellSize = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Config")
    int32 SpawnCount = 1000; // 테스트하고 싶은 마릿수

    UPROPERTY(EditAnywhere, Category = "Config")
    float ViewportMarginRatio = 0.2f;

    // 스폰할 몬스터 클래스
    UPROPERTY(EditAnywhere, Category = "Config")
    TSubclassOf<class ABaseMonster> MonsterClass;

    // 몬스터 스폰 요청
    UFUNCTION(BlueprintCallable)
    void SpawnMob(FVector Location);

    // 몬스터 반환 요청 (죽었을 때)
    UFUNCTION(BlueprintCallable)
    void ReturnMob(class ABaseMonster* Mob);

protected:

    void DrawDebugViewFrustum(float MarginRatio);

    // LOD 활성화 여부
    UPROPERTY(EditAnywhere, Category = "Config")
	bool IsLOD = true;


private:
    // 전체 풀
    UPROPERTY()
    TArray<class ABaseMonster*> MonsterPool;

    // 사용 가능한(비활성화된) 몹 인덱스 큐
    TQueue<int32> AvailableIndices;

private:
    bool IsInViewportAsync(const FVector& WorldLocation, const FMatrix& ViewProjMatrix, float MarginRatio) const;
    void DrawLODDebugBoundaries();

    int32 CurrentSpawnCount = 0;

    // 분산 스폰을 위한 타이머 핸들
    FTimerHandle InitSpawnTimerHandle;

    // 타이머에 의해 호출될 함수
    void BatchSpawnMobs();

    // 테스트용: 생성 완료 후 몹들을 활성화시키는 함수
    void StartStressTest();
};
