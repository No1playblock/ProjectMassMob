// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/CrowdFinalManager.h"
#include "Actor/CrowdUnit.h"
#include "Kismet/GameplayStatics.h"
#include "Async/ParallelFor.h"
#include "Engine/LocalPlayer.h"
#include "SceneView.h" 
#include "DrawDebugHelpers.h"

DECLARE_STATS_GROUP(TEXT("Crowd Manager Debug"), STATGROUP_CrowdDebug, STATCAT_Advanced);

// 전체 루프 횟수
DECLARE_DWORD_COUNTER_STAT(TEXT("Total Units Checked"), STAT_Crowd_Total, STATGROUP_CrowdDebug);

// LOD 2라서 스킵된 횟수
DECLARE_DWORD_COUNTER_STAT(TEXT("Skipped : LOD 2 (Hidden)"), STAT_Crowd_Skip_LOD2, STATGROUP_CrowdDebug);
// 스로틀링(내 차례 아님)으로 스킵된 횟수
DECLARE_DWORD_COUNTER_STAT(TEXT("Skipped : Throttling"), STAT_Crowd_Skip_Throttle, STATGROUP_CrowdDebug);
// 실제로 UpdateTransform이 불린 횟수 
DECLARE_DWORD_COUNTER_STAT(TEXT("Executed : Update Transform"), STAT_Crowd_RealUpdate, STATGROUP_CrowdDebug);


ACrowdFinalManager::ACrowdFinalManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f; // 매 프레임 업데이트
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ACrowdFinalManager::BeginPlay()
{
	Super::BeginPlay();

	InitializePool();
}
void ACrowdFinalManager::InitializePool()
{
	if (bIsInitialized) return;

	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (UnitClass)
	{
		UnitActorPool.Reserve(MaxPoolSize);
		UnitDataArray.Reserve(MaxPoolSize);

		for (int32 i = 0; i < MaxPoolSize; ++i)
		{
			FVector TempLoc(0, 0, -10000);
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			ACrowdUnit* NewUnit = GetWorld()->SpawnActor<ACrowdUnit>(UnitClass, TempLoc, FRotator::ZeroRotator, Params);
			if (NewUnit)
			{
				NewUnit->SetActive(false);
				UnitActorPool.Add(NewUnit);

				FCrowdUnitData NewData;
				NewData.ActorIndex = i;
				NewData.MoveSpeed = DefaultMoveSpeed;
				UnitDataArray.Add(NewData);
			}
		}
	}

	// 초기화 완료
	bIsInitialized = true;

	SetActorTickEnabled(true);
}

void ACrowdFinalManager::Tick(float DeltaTime)
{
	
	Super::Tick(DeltaTime);
	
	if (!PlayerPawn.IsValid())
	{
		PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		return;
	}
		
	FrameCount++;

	// 공통 데이터 준비
	FVector PlayerLoc = PlayerPawn->GetActorLocation();
	FMatrix ViewProjMatrix = FMatrix::Identity;
	bool bCanCalcLOD = false;

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (LP->ViewportClient)
			{
				FSceneViewProjectionData ProjectionData;
				if (LP->GetProjectionData(LP->ViewportClient->Viewport, ProjectionData, 0))
				{
					ViewProjMatrix = ProjectionData.ComputeViewProjectionMatrix();
					bCanCalcLOD = true;

				}
			}
		}
	}

	int32 Count = UnitDataArray.Num();

	const float DistCheck_High = 1500.0f * 1500.0f;
	const float DistCheck_Low = 3000.0f * 3000.0f;

	// 워커 쓰레드 병렬 연산
	ParallelFor(Count, [&](int32 Index)
		{
			FCrowdUnitData& Data = UnitDataArray[Index];
			if (!Data.bIsActive) return;

			// LOD 판별
			float DistSq = FVector::DistSquared(PlayerLoc, Data.Location);


			if (DistSq > DistCheck_Low)
			{
				// 너무 멀면 화면 검사 생략
				Data.DesiredLOD = 2;
			}
			else
			{
				//거리 안에는 들어오면 화면 안에 있는지 정밀 검사
				if (bCanCalcLOD)
				{
					if (DistSq < DistCheck_High && IsInViewportAsync(Data.Location, ViewProjMatrix, LODMargin_High))
						Data.DesiredLOD = 0; // 가까움 
					else if (IsInViewportAsync(Data.Location, ViewProjMatrix, LODMargin_Low))
						Data.DesiredLOD = 1; // 중간
					else
						Data.DesiredLOD = 2; // 화면 밖
				}
				else
				{
					Data.DesiredLOD = 0; // 계산 불가 시 기본값
				}
			}

			//Interval 설정
			int32 UpdateInterval = Interval_High; 
			if (Data.DesiredLOD == 1) UpdateInterval = Interval_Low;
			else if (Data.DesiredLOD == 2) UpdateInterval = Interval_Far; 

			// 내 차례인지 확인
			if ((Index + FrameCount) % UpdateInterval == 0)
			{
				float ActualDeltaTime = DeltaTime * UpdateInterval;

				FVector Dir = PlayerLoc - Data.Location;
				Dir.Z = 0.0f;

				FVector NormDir = Dir.GetSafeNormal2D();
				if (!NormDir.IsNearlyZero())
				{
					// 데이터상의 위치를 이동시킴
					Data.Location += NormDir * Data.MoveSpeed * ActualDeltaTime;
					Data.Direction = NormDir;
				}
			}
		});

	// Main Thread 
	for (int32 i = 0; i < Count; ++i)
	{
		const FCrowdUnitData& Data = UnitDataArray[i];
		if (!Data.bIsActive) continue;

		ACrowdUnit* Unit = UnitActorPool[Data.ActorIndex];
		if (!Unit) continue;

		// LOD 상태 최신화
		Unit->SetLODLevel(Data.DesiredLOD);

		if (Data.DesiredLOD == 2)
		{
			INC_DWORD_STAT(STAT_Crowd_Skip_LOD2);
			continue;
		}

		// 스로틀링 스킵
		int32 UpdateInterval = Interval_High;
		if (Data.DesiredLOD == 1) UpdateInterval = Interval_Low;

		if ((i + FrameCount) % UpdateInterval != 0)
		{
			INC_DWORD_STAT(STAT_Crowd_Skip_Throttle);
			continue;
		}

		INC_DWORD_STAT(STAT_Crowd_RealUpdate);
		Unit->UpdateTransform(Data.Location, Data.Direction.Rotation());
	}
	
}

void ACrowdFinalManager::SpawnUnit(const FVector& SpawnLocation)
{
	// 선형 탐색으로 빈 유닛 찾기
	for (int32 i = 0; i < UnitDataArray.Num(); ++i)
	{
		if (!UnitDataArray[i].bIsActive)
		{
			// 데이터 활성화
			UnitDataArray[i].bIsActive = true;
			UnitDataArray[i].Location = SpawnLocation;
			UnitDataArray[i].Location.Z = 88.0f;
			UnitDataArray[i].DesiredLOD = 0;

			// 액터 활성화
			if (ACrowdUnit* Unit = UnitActorPool[i])
			{
				Unit->UpdateTransform(SpawnLocation, FRotator::ZeroRotator);
				Unit->SetActive(true);
			}
			return;
		}
	}
}

bool ACrowdFinalManager::IsInViewportAsync(const FVector& WorldLocation, const FMatrix& ViewProjMatrix, float MarginRatio) const
{
	FPlane Result = ViewProjMatrix.TransformFVector4(FVector4(WorldLocation, 1.0f));
	if (Result.W <= 0.0f) return false;

	float RHW = 1.0f / Result.W;
	float X = Result.X * RHW;
	float Y = Result.Y * RHW;
	float Margin = 1.0f + (MarginRatio * 2.0f);

	return (X >= -Margin && X <= Margin) && (Y >= -Margin && Y <= Margin);
}

