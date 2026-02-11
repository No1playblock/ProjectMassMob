// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "Prop/MobPoolManager.h"
#include "Kismet/GameplayStatics.h"

ABaseMonster::ABaseMonster()
{
    // 틱 최적화: 개별 틱은 끄고 매니저에서 관리하거나, 필요한 경우만 킴
    PrimaryActorTick.bCanEverTick = false;

    // 충돌 최적화
    // RVO Avoidance를 켜거나, 몹끼리 충돌 채널을 무시하도록 설정 필요
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	//추후 CrowdFollowing 으로 변경 고려
   

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->bUseRVOAvoidance = true;
        GetCharacterMovement()->AvoidanceWeight = 0.5f;
        // 물리 틱 최적화
        GetCharacterMovement()->bRunPhysicsWithNoController = false;
        GetCharacterMovement()->bEnablePhysicsInteraction = false; // 물체 밀기 끔


        // 안 쓰는 기능 끄기
        GetCharacterMovement()->SetMovementMode(MOVE_Walking);

        // NavMesh 위에서만 다닐 거면 바닥 체크를 덜 하게 설정 가능
        GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;
    }
    if (GetCapsuleComponent())
    {
        // 몬스터끼리 카메라나 가시성 채널을 막을 필요 없음 (LineTrace 비용 절약)
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
        GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
        GetCapsuleComponent()->SetGenerateOverlapEvents(false);
    }

    if (GetMesh())
    {
        // URO(업데이트 빈도 최적화) 강제 활성화
        GetMesh()->bEnableUpdateRateOptimizations = true;

        // 화면에 작게 보일 때 애니메이션 프레임 건너뛰기 허용
        GetMesh()->GlobalAnimRateScale = 1.0f;

        GetMesh()->SetCollisionProfileName(TEXT("NoCollision"));
        GetMesh()->SetGenerateOverlapEvents(false);

        GetMesh()->SetCastShadow(false);
        GetMesh()->bComponentUseFixedSkelBounds = true;
    }
    
}

void ABaseMonster::BeginPlay()
{
    Super::BeginPlay();
    MyController = Cast<AAIController>(GetController());

    TargetPlayer = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    AActor* FoundActor = UGameplayStatics::GetActorOfClass(GetWorld(), AMobPoolManager::StaticClass());
    PoolManager = Cast<AMobPoolManager>(FoundActor);


    // 시작하자마자 일단 숨김
    Deactivate();
}
void ABaseMonster::SetLODLevel(int32 Level)
{
    // 상태가 같으면 무시
    if (CurrentLODLevel == Level) return;
    CurrentLODLevel = Level;

    USkeletalMeshComponent* MeshComp = GetMesh();
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();

    //걍 다 초기화
    GetWorld()->GetTimerManager().ClearTimer(MoveTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(GridCheckTimerHandle);


    switch (Level)
    {
    case 0: // 플레이어 근처(제일가깝)

        SetActorHiddenInGame(false);
        SetActorEnableCollision(true);

        if (GetCapsuleComponent())
        {
            GetCapsuleComponent()->SetGenerateOverlapEvents(true); 
        }

        if (MoveComp)
        {
            MoveComp->SetComponentTickEnabled(true); // 켜기
            MoveComp->SetComponentTickInterval(0.0f);
            MoveComp->bUseRVOAvoidance = true; //RVO 켜기
        }

        if (MeshComp)
        {
            MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
            MeshComp->bEnableUpdateRateOptimizations = false;
            MeshComp->bPauseAnims = false;
        }

        MoveToPlayer();

        GetWorld()->GetTimerManager().SetTimer(MoveTimerHandle, this, &ABaseMonster::MoveToPlayer, (float)Interval_High/60.0f, true);
        break;

    case 1: // 화면 밖 버퍼존(중간 거리)

        SetActorHiddenInGame(true);
        SetActorEnableCollision(true);

        if (MoveComp)
        {
            MoveComp->SetComponentTickEnabled(true); // 이동은 해야 함
            MoveComp->SetComponentTickInterval(0.2f); // 0.2초마다 계산
            MoveComp->bUseRVOAvoidance = false; //RVO는 끄기
        }

        if (MeshComp)
        {
            // 애니메이션은 끄기
            MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
            MeshComp->bEnableUpdateRateOptimizations = true;
            MeshComp->bPauseAnims = false; // 포즈는 유지
        }

        MoveToPlayer();


        GetWorld()->GetTimerManager().SetTimer(MoveTimerHandle, this, &ABaseMonster::MoveToPlayer, (float)Interval_Low / 60.0f, true);
        break;
    case 2: // 아주 먼 곳(끄기)
    default:
        // 렌더링만 끄기
        SetActorHiddenInGame(true);
        SetActorEnableCollision(true); // 땅에 붙어 있어야 하므로 충돌 필요

        if (GetCapsuleComponent())
        {
            GetCapsuleComponent()->SetGenerateOverlapEvents(false); 
        }

        if (MoveComp)
        {
            MoveComp->SetComponentTickEnabled(true);
            MoveComp->SetComponentTickInterval(0.5f); // 이동 계산도 드문드문
            MoveComp->bUseRVOAvoidance = false; // 서로 밀치기 끔
            MoveComp->MaxWalkSpeed = 300.0f; // 멀리서는 좀 천천히 오게 할 수도 있음
            MoveComp->MaxSimulationTimeStep = 0.6f;
            MoveComp->MaxSimulationIterations = 1;
        }

        if (MeshComp)
        {
            MeshComp->bPauseAnims = true;
        }

        MoveToPlayer();

        GetWorld()->GetTimerManager().SetTimer(MoveTimerHandle, this, &ABaseMonster::MoveToPlayer, (float)Interval_Far / 60.0f, true);


        break;
    }
}
void ABaseMonster::Activate(FVector StartLocation)
{
    SetActorLocation(StartLocation);

    // 초기화 시엔 일단 켜둠
    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    GetCharacterMovement()->SetComponentTickEnabled(true);
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);
    // 강제 LOD 리셋
    CurrentLODLevel = -1;

    LastGridLocation = StartLocation;

    MoveToPlayer(); // 즉시이동

    SetLODLevel(0);
}

void ABaseMonster::Deactivate()
{
    // 이동 정지 및 숨김
    if (MyController) MyController->StopMovement();

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);

    // 틱 소모 방지
    GetCharacterMovement()->SetComponentTickEnabled(false);
    
    GetWorld()->GetTimerManager().ClearTimer(MoveTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(GridCheckTimerHandle);
}

void ABaseMonster::MoveToPlayer()
{
    SCOPE_CYCLE_COUNTER(STAT_Monster_MoveLogic);

    if (MyController && TargetPlayer)
    {
        MyController->MoveToActor(TargetPlayer, 5.0f);
    }
}
