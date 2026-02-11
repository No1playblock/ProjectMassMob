// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdUnit.generated.h"

UCLASS()
class PROJECTMASSMOB_API ACrowdUnit : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACrowdUnit();

public:
	// 매니저가 위치를 옮길 때 호출
	void UpdateTransform(const FVector& NewLocation, const FRotator& NewRotation);

	// 매니저가 LOD(거리/화면) 상태를 변경할 때 호출
	// 0: 가까움(풀옵션), 1: 중간(최적화), 2: 멀음(숨김)
	void SetLODLevel(int32 Level);

	// 풀링용 활성/비활성 함수
	void SetActive(bool bActive);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class USkeletalMeshComponent> MeshComp;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UCapsuleComponent> CapsuleComp;

	

private:
	int32 CurrentLOD = -1;
};
