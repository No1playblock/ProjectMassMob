// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/SkeletalMassMonster.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"

ASkeletalMassMonster::ASkeletalMassMonster()
{
	// 틱을 아예 끕니다. (매니저가 관리함)
	PrimaryActorTick.bCanEverTick = false;

	// 1. 캡슐 컴포넌트 설정 (루트)
	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	RootComponent = CapsuleComp;
	CapsuleComp->SetCapsuleHalfHeight(88.0f);
	CapsuleComp->SetCapsuleRadius(34.0f);

	// 물리 시뮬레이션 끄기 (최적화)
	CapsuleComp->SetSimulatePhysics(false);

	// 충돌 프리셋: 겹침만 허용 (OverlapAllDynamic)
	CapsuleComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CapsuleComp->SetGenerateOverlapEvents(false);
	CapsuleComp->SetCanEverAffectNavigation(false);

	// 2. [변경점] 스켈레탈 메쉬 컴포넌트 설정
	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);

	// 메쉬 충돌은 끕니다 (캡슐로만 충돌 처리)
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->SetGenerateOverlapEvents(false);
	MeshComp->SetCastShadow(false);
	// [최적화] 화면에 안 보이면 애니메이션 계산 중지
	// (Mass 시스템에서는 필수적인 최적화 옵션입니다)
	MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
	MeshComp->bEnableUpdateRateOptimizations = true;
}

void ASkeletalMassMonster::BeginPlay()
{
	Super::BeginPlay();
}

void ASkeletalMassMonster::UpdateTransform(const FVector& NewLocation, const FRotator& NewRotation)
{
	// 텔레포트 옵션을 켜서 물리 연산 없이 좌표만 이동
	SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void ASkeletalMassMonster::SetActiveState(bool bActive)
{
	SetActorHiddenInGame(!bActive);
	SetActorEnableCollision(bActive);

	// 스켈레탈 메쉬는 숨겨지면 애니메이션 틱도 멈추도록 설정해뒀으므로(OnlyTickPoseWhenRendered)
	// 별도의 PauseAnims 호출은 필요 없으나, 확실히 하려면 아래 코드를 추가해도 됩니다.
	/*
	if (MeshComp)
	{
		MeshComp->bPauseAnims = !bActive;
	}
	*/
}

