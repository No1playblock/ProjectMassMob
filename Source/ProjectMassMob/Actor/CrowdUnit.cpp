// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/CrowdUnit.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

ACrowdUnit::ACrowdUnit()
{
	// 1. 액터 틱 완전 제거
	PrimaryActorTick.bCanEverTick = false;


	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	RootComponent = CapsuleComp;
	CapsuleComp->SetCapsuleHalfHeight(88.0f);
	CapsuleComp->SetCapsuleRadius(34.0f);

	// 물리/충돌 최적화
	CapsuleComp->SetSimulatePhysics(false);
	CapsuleComp->SetCollisionProfileName(TEXT("NoCollision")); // 이동 비용 최소화
	CapsuleComp->SetGenerateOverlapEvents(false);
	CapsuleComp->SetCanEverAffectNavigation(false); // 내비게이션 영향 X

	CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	// 스켈레탈 메쉬

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->SetGenerateOverlapEvents(false);

	// 렌더링 최적화
	MeshComp->SetCastShadow(false); // 그림자 끄기 (성능 향상 큼)
	MeshComp->bEnableUpdateRateOptimizations = true; // URO 활성화
	MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	MeshComp->bComponentUseFixedSkelBounds = true;
	MeshComp->Bounds = FBoxSphereBounds(FVector(0, 0, 88), FVector(100, 100, 100), 100.0f);
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

}

void ACrowdUnit::UpdateTransform(const FVector& NewLocation, const FRotator& NewRotation)
{
	// 물리 연산 없이 좌표만 강제 이동 (Teleport)
	SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ACrowdUnit::SetLODLevel(int32 Level)
{
	if (CurrentLOD == Level) return;
	CurrentLOD = Level;

	switch (Level)
	{
	case 0: // 가까움
		SetActorHiddenInGame(false);
		if (MeshComp)
		{
			// 부드러운 애니메이션
			MeshComp->VisibilityBasedAnimTickOption 
				= EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
			MeshComp->bEnableUpdateRateOptimizations = false;
		}
		break;
	case 1: // 중간
		SetActorHiddenInGame(false);
		if (MeshComp)
		{
			// 화면에 보일 때만 갱신 + 프레임 스킵 허용
			MeshComp->VisibilityBasedAnimTickOption 
				= EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
			MeshComp->bEnableUpdateRateOptimizations = true;
		}
		break;
	case 2: //숨김
	default:
		SetActorHiddenInGame(true); // 렌더링 파이프라인에서 제외
		if (MeshComp)
		{
			// 애니메이션 연산 중지
			MeshComp->VisibilityBasedAnimTickOption 
				= EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		}
		break;
	}
}

void ACrowdUnit::SetActive(bool bActive)
{
	if (bActive)
	{
		CurrentLOD = -1; // LOD 강제 갱신 유도
	}
	else
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
		//CurrentLOD = 2;
	}
}

