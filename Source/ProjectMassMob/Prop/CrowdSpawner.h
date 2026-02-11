// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSpawner.generated.h"

UCLASS()
class PROJECTMASSMOB_API ACrowdSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ACrowdSpawner();

protected:
	virtual void BeginPlay() override;

	// 랜덤 위치 찾기
	bool GetRandomSpawnLocation(FVector& OutLocation);

public:
	UPROPERTY(EditAnywhere, Category = "Settings")
	TObjectPtr<class ACrowdFinalManager> Manager;

	// 시작 시 소환할 마릿수
	UPROPERTY(EditAnywhere, Category = "Settings")
	int32 InitialCount = 1000;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float MinDist = 1500.0f;

	UPROPERTY(EditAnywhere, Category = "Settings")
	float MaxDist = 2500.0f;

};
