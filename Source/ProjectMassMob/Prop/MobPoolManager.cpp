// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/MobPoolManager.h"
#include "Character/BaseMonster.h"
#include "DrawDebugHelpers.h"        
#include "Kismet/GameplayStatics.h"   
#include "SceneView.h"         
#include "Engine/LocalPlayer.h" 
#include "Async/ParallelFor.h"  


DECLARE_STATS_GROUP(TEXT("MobPoolManager"), STATGROUP_MobPool, STATCAT_Advanced);

// 전체 틱 측정용
DECLARE_CYCLE_STAT(TEXT("MobPool Total Tick"), STAT_MobPool_Tick, STATGROUP_MobPool);
// LOD 계산 로직 측정용
DECLARE_CYCLE_STAT(TEXT("MobPool UpdateLOD"), STAT_MobPool_UpdateLOD, STATGROUP_MobPool);
// 화면 판별 로직 측정용 (필요하면)
DECLARE_CYCLE_STAT(TEXT("MobPool IsInViewport"), STAT_MobPool_ViewportCheck, STATGROUP_MobPool);

AMobPoolManager::AMobPoolManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AMobPoolManager::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->ConsoleCommand(TEXT("stat fps"));
	}

	if (!MonsterClass) return;

	FVector SpawnLoc(0, 0, -10000); // 안 보이는 곳에 생성

	GetWorld()->GetTimerManager().SetTimer(InitSpawnTimerHandle, this, &AMobPoolManager::BatchSpawnMobs, 0.1f, true);

}
void AMobPoolManager::BatchSpawnMobs()
{
	// 한 프레임당 생성할 마릿수
	const int32 BatchSize = 10;

	FVector SpawnLoc(0, 0, -10000); // 안 보이는 곳

	for (int32 i = 0; i < BatchSize; i++)
	{
		// 목표 개수 채웠으면 종료 로직 실행
		if (CurrentSpawnCount >= PoolSize)
		{
			GetWorld()->GetTimerManager().ClearTimer(InitSpawnTimerHandle);

			// 초기화 완료 로그 출력
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("=== POOL INIT COMPLETE: START TEST ==="));
			UE_LOG(LogTemp, Warning, TEXT("Pool Generation Finished. Total: %d"), CurrentSpawnCount);

			//  초기화가 끝났으니 이제 몹을 뿌려
			StartStressTest();
			return;
		}

		// 몹 생성 로직
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ABaseMonster* NewMob = GetWorld()->SpawnActor<ABaseMonster>(MonsterClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);

		if (NewMob)
		{
			NewMob->PoolIndex = CurrentSpawnCount;
			NewMob->Deactivate();
			MonsterPool.Add(NewMob);
			AvailableIndices.Enqueue(CurrentSpawnCount);

			CurrentSpawnCount++;
		}
	}
}
void AMobPoolManager::StartStressTest()
{
	for (int i = 0; i < SpawnCount; i++)
	{
		FVector SpawnLocation = FVector(FMath::RandRange(-3000.f, 3000.f), FMath::RandRange(-3000.f, 3000.f), 100.f);
		SpawnMob(SpawnLocation);
	}
}

void AMobPoolManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SCOPE_CYCLE_COUNTER(STAT_MobPool_Tick);

	if (IsLOD)
	{
		UpdateMobLOD();
	}

}


void AMobPoolManager::UpdateMobLOD()
{
	SCOPE_CYCLE_COUNTER(STAT_MobPool_UpdateLOD);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// 스레드에 넘겨줄 읽기 전용 데이터 준비
	FVector PlayerLoc = PC->GetPawnOrSpectator() ? PC->GetPawnOrSpectator()->GetActorLocation() : FVector::ZeroVector;

	// 카메라 행렬 추출
	ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
	if (!LocalPlayer || !LocalPlayer->ViewportClient) return;

	FSceneViewProjectionData ProjectionData;
	if (!LocalPlayer->GetProjectionData(LocalPlayer->ViewportClient->Viewport, ProjectionData, 0))
		return;
	FMatrix ViewProjMatrix = ProjectionData.ComputeViewProjectionMatrix();

	// 결과 저장용 배열
	int32 TotalMobs = MonsterPool.Num();
	TArray<int32> CalculatedLODs;
	CalculatedLODs.AddUninitialized(TotalMobs); // 메모리만 확보

	float InnerMargin = 0.1f;
	float OuterMargin = ViewportMarginRatio; // 0.2


	ParallelFor(TotalMobs, [&](int32 Index)
		{
			ABaseMonster* Mob = MonsterPool[Index];

			if (!Mob)
			{
				CalculatedLODs[Index] = 2;
				return;
			}
			FVector MobLoc = Mob->GetActorLocation();

			// 로직 수행
			if (IsInViewportAsync(MobLoc, ViewProjMatrix, InnerMargin))
			{
				CalculatedLODs[Index] = 0; // High
			}
			else if (IsInViewportAsync(MobLoc, ViewProjMatrix, OuterMargin))
			{
				CalculatedLODs[Index] = 1; // Low
			}
			else
			{
				CalculatedLODs[Index] = 2; // Off
			}
		});

	// 실제 Actor 상태 변경은 무조건 메인 스레드에서 해야 함
	for (int32 i = 0; i < TotalMobs; i++)
	{
		if (MonsterPool[i])
		{
			MonsterPool[i]->SetLODLevel(CalculatedLODs[i]);
		}
	}
}


void AMobPoolManager::SpawnMob(FVector Location)
{
	if (AvailableIndices.IsEmpty())
	{
		return;
	}

	int32 Index;
	if (AvailableIndices.Dequeue(Index))
	{
		ABaseMonster* Mob = MonsterPool[Index];
		if (Mob)
		{
			Mob->Activate(Location);
		}
	}
}

void AMobPoolManager::ReturnMob(ABaseMonster* Mob)
{
	if (Mob && MonsterPool.IsValidIndex(Mob->PoolIndex))
	{
		Mob->Deactivate();
		AvailableIndices.Enqueue(Mob->PoolIndex);
	}
}

bool AMobPoolManager::IsInViewportAsync(const FVector& WorldLocation, const FMatrix& ViewProjMatrix, float MarginRatio) const
{
	SCOPE_CYCLE_COUNTER(STAT_MobPool_ViewportCheck);

	FPlane Result = ViewProjMatrix.TransformFVector4(FVector4(WorldLocation, 1.0f));

	if (Result.W <= 0.0f) return false;

	float RHW = 1.0f / Result.W;
	float X = Result.X * RHW;
	float Y = Result.Y * RHW;

	// 마진 적용해서 범위 
	// X, Y가 -1 ~ 1 사이면 화면 안임.
	// MarginRatio가 0.2면 -1.2 ~ 1.2 까지 허용
	float Margin = 1.0f + (MarginRatio * 2.0f); // -1~1 구간이므로 비율 보정

	return (X >= -Margin && X <= Margin) && (Y >= -Margin && Y <= Margin);
}

