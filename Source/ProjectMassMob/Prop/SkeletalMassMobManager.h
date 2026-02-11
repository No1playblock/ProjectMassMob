// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actor/SkeletalMassMonster.h"
#include "SkeletalMassMobManager.generated.h"
struct FSkeletalMassMobData
{
	FVector Location;       // 현재 위치
	FVector Direction;      // 바라보는 방향
	float MoveSpeed;        // 이동 속도
	bool bIsActive;         // 활성화 여부
	int32 ActorIndex;       // 실제 액터 배열 인덱스

	FSkeletalMassMobData()
		: Location(FVector::ZeroVector)
		, Direction(FVector::ForwardVector)
		, MoveSpeed(0.0f)
		, bIsActive(false)
		, ActorIndex(-1)
	{
	}
};

UCLASS()
class PROJECTMASSMOB_API ASkeletalMassMobManager : public AActor
{
	GENERATED_BODY()

public:
	ASkeletalMassMobManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 몬스터 스폰 요청
	UFUNCTION(BlueprintCallable, Category = "MassMob")
	void SpawnMonster(const FVector& SpawnLocation);

	// 전체 몬스터 제거
	UFUNCTION(BlueprintCallable, Category = "MassMob")
	void ClearAllMonsters();

public:
	// [변경점] 스켈레탈 몬스터 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	TSubclassOf<ASkeletalMassMonster> MonsterClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	int32 MaxPoolSize = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	float DefaultMoveSpeed = 300.0f;

private:
	// 데이터 배열
	TArray<FSkeletalMassMobData> MobDataArray;

	// 액터 배열 (SkeletalMassMonster 포인터)
	UPROPERTY()
	TArray<ASkeletalMassMonster*> MobActorPool;

	TWeakObjectPtr<APawn> PlayerPawn;
	int count = 0;
};
