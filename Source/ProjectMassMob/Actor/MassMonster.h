// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassMonster.generated.h"

UCLASS()
class PROJECTMASSMOB_API AMassMonster : public AActor
{
	GENERATED_BODY()
	
public:
    AMassMonster();

protected:
    virtual void BeginPlay() override;

public:
    // 매니저가 위치를 강제로 옮길 때 사용할 함수
    void UpdateTransform(const FVector& NewLocation, const FRotator& NewRotation);

    // 몬스터 활성화/비활성화 (오브젝트 풀링용)
    void SetActiveState(bool bActive);

protected:
    // 눈에 보일 메쉬 컴포넌트 (CharacterMovement 같은 거 없음)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
    TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
	TObjectPtr<class USkeletalMeshComponent> SkeletalMeshComp;

    // 충돌 처리를 위한 캡슐 (필요하다면 사용, 여기서는 가볍게 오버랩용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visual")
    TObjectPtr<class UCapsuleComponent> CapsuleComp;

};
