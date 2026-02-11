// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MobSpawner.generated.h"

UCLASS()
class PROJECTMASSMOB_API AMobSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMobSpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // 랜덤한 땅 위치 찾기 함수
    bool GetRandomSpawnLocation(FVector& OutLocation);

public:
    // 자동으로 찾을 거지만, 수동 설정도 가능하게 열어둠
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TObjectPtr<class AMassMobManager> MobManager;

    // 소환 주기 (초 단위)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float SpawnInterval = 0.1f;

    // 한 번에 소환할 마리 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 SpawnCountPerInterval = 5;

    // 플레이어로부터 최소 거리 (화면 밖)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MinSpawnDistance = 1500.0f;

    // 플레이어로부터 최대 거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxSpawnDistance = 2500.0f;

private:
    float TimeSinceLastSpawn = 0.0f;
};
