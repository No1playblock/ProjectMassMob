// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkeletalMobSpawner.generated.h"

UCLASS()
class PROJECTMASSMOB_API ASkeletalMobSpawner : public AActor
{
	GENERATED_BODY()
	
public:
	ASkeletalMobSpawner();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	bool GetRandomSpawnLocation(FVector& OutLocation);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TObjectPtr<class ASkeletalMassMobManager> MobManager;

	// [추가됨] 게임 시작 시 즉시 소환할 마릿수 (기본 1000마리)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 InitialSpawnCount = 1000;

	// 소환 주기 (초 단위)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float SpawnInterval = 0.1f;

	// 한 번에 소환할 마리 수 (주기적 소환용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 SpawnCountPerInterval = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MinSpawnDistance = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float MaxSpawnDistance = 2500.0f;

private:
	float TimeSinceLastSpawn = 0.0f;

};
