// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SkeletalMassMonster.generated.h"

UCLASS()
class PROJECTMASSMOB_API ASkeletalMassMonster : public AActor
{
	GENERATED_BODY()
	
public:
	ASkeletalMassMonster();

protected:
	virtual void BeginPlay() override;

public:
	// 매니저가 위치를 강제로 옮길 때 사용할 함수
	void UpdateTransform(const FVector& NewLocation, const FRotator& NewRotation);

	// 몬스터 활성화/비활성화 (오브젝트 풀링용)
	void SetActiveState(bool bActive);

public:
	// [변경점] 스태틱 메쉬 대신 스켈레탈 메쉬 사용
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<class USkeletalMeshComponent> MeshComp;

	// 충돌 처리를 위한 캡슐
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<class UCapsuleComponent> CapsuleComp;

};
