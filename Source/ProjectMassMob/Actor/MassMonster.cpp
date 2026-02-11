// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/MassMonster.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CapsuleComponent.h"

AMassMonster::AMassMonster()
{
    // 틱을 아예 끕니다. (가장 중요)
    PrimaryActorTick.bCanEverTick = false;

    // 캡슐 컴포넌트 설정 (루트)
    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    RootComponent = CapsuleComp;
    CapsuleComp->SetCapsuleHalfHeight(88.0f);
    CapsuleComp->SetCapsuleRadius(34.0f);
    // 물리 시뮬레이션 끄기
    CapsuleComp->SetSimulatePhysics(false);
    // 충돌 프리셋: 겹침만 허용하거나 필요 없으면 NoCollision
    CapsuleComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

     //메쉬 컴포넌트 설정
    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    MeshComp->SetupAttachment(RootComponent);
    MeshComp->SetCollisionProfileName(TEXT("NoCollision")); // 메쉬 충돌은 끔
    MeshComp->SetGenerateOverlapEvents(false);

	//SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	//SkeletalMeshComp->SetupAttachment(RootComponent);
	//SkeletalMeshComp->SetCollisionProfileName(TEXT("NoCollision")); // 메쉬 충돌은 끔
	//SkeletalMeshComp->SetGenerateOverlapEvents(false);

}

void AMassMonster::BeginPlay()
{
    Super::BeginPlay();
    // 추가 초기화 로직이 필요하면 여기에 작성
}

void AMassMonster::UpdateTransform(const FVector& NewLocation, const FRotator& NewRotation)
{
    // 텔레포트 옵션을 켜서 물리 연산 없이 좌표만 이동
    SetActorLocationAndRotation(NewLocation, NewRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

void AMassMonster::SetActiveState(bool bActive)
{
    SetActorHiddenInGame(!bActive);
    SetActorEnableCollision(bActive);
    // 활성화 여부에 따라 틱을 켜거나 끌 필요 없음 (애초에 꺼져있음)
}

