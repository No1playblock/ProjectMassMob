// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdFinalManager.generated.h"

struct FCrowdUnitData
{
	FVector Location;
	FVector Direction;
	float MoveSpeed;
	bool bIsActive;
	int32 ActorIndex; // 실제 액터 풀 인덱스
	int32 DesiredLOD; // 계산된 LOD 결과

	FCrowdUnitData()
		: Location(FVector::ZeroVector), Direction(FVector::ForwardVector)
		, MoveSpeed(0.0f), bIsActive(false), ActorIndex(-1), DesiredLOD(2)
	{
	}
};

UCLASS()
class PROJECTMASSMOB_API ACrowdFinalManager : public AActor
{
	GENERATED_BODY()
	
public:
	ACrowdFinalManager();

	void InitializePool();

	// 유닛 소환 요청
	void SpawnUnit(const FVector& SpawnLocation);
protected:
	virtual void BeginPlay() override;
	
	// 여기서 모든 계산(이동+LOD)을 수행
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 Interval_High = 1; // 가까운 유닛은 매 프레임 이동

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 Interval_Low = 10;  // 중간 유닛은 3프레임에 한 번 이동

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 Interval_Far = 30;  // 먼 유닛은 5프레임에 한 번 이동

	UPROPERTY(EditAnywhere, Category = "Settings")
	TSubclassOf<class ACrowdUnit> UnitClass;

	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 MaxPoolSize = 2000;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float DefaultMoveSpeed = 300.0f;

	// LOD 거리 비율 (화면 크기 대비 마진)
	UPROPERTY(EditAnywhere, Category = "Settings")
	float LODMargin_High = 0.1f;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float LODMargin_Low = 0.3f;

private:
	// 화면 안에 있는지 검사 (비동기 안전)
	bool IsInViewportAsync(const FVector& WorldLocation, const FMatrix& ViewProjMatrix, float MarginRatio) const;

private:
	// 데이터 배열 (CPU 캐시 히트율 극대화)
	TArray<FCrowdUnitData> UnitDataArray;

	// 실제 액터 배열 (껍데기)
	UPROPERTY()
	TArray<class ACrowdUnit*> UnitActorPool;

	TWeakObjectPtr<APawn> PlayerPawn;

	bool bIsInitialized = false;

	int32 FrameCount = 0;
};
