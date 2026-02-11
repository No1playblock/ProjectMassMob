// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/MobPoolManager.h"
#include "Character/BaseMonster.h"
#include "DrawDebugHelpers.h"         // 디버그 드로잉용
#include "Kismet/GameplayStatics.h"   // 플레이어 위치 찾기용
#include "SceneView.h"          // FSceneViewProjectionData, EStereoscopicPass 정의
#include "Engine/LocalPlayer.h" // ULocalPlayer 정의
#include "Async/ParallelFor.h"  // ParallelFor 정의


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

	//for (int32 i = 0; i < PoolSize; i++)
	//{
	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//	ABaseMonster* NewMob = GetWorld()->SpawnActor<ABaseMonster>(MonsterClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);

	//	if (NewMob)
	//	{
	//		NewMob->PoolIndex = i;
	//		NewMob->Deactivate(); // 바로 비활성화
	//		MonsterPool.Add(NewMob);
	//		AvailableIndices.Enqueue(i);
	//	}
	//}
	//FTimerHandle SpawnTimerHandle;

	//for (int i = 0; i < 50; i++)
	//{
	//	FVector SpawnLocation = FVector(FMath::RandRange(-1000.f, 1000.f), FMath::RandRange(-1000.f, 1000.f), 100.f);
	//	SpawnMob(SpawnLocation);
	//}

	/*GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, [this]()
		{
			FVector SpawnLocation = FVector(FMath::RandRange(-1000.f, 1000.f), FMath::RandRange(-1000.f, 1000.f), 100.f);
			SpawnMob(SpawnLocation);
		}, 0.5f, true);*/
}
void AMobPoolManager::BatchSpawnMobs()
{
	// 한 프레임당 생성할 마릿수 (렉이 걸리면 이 숫자를 줄이면 됨)
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

			// [중요] 초기화가 끝났으니 이제 몹을 뿌려서 테스트 시작
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
	//ShowDebugGrid();
	SCOPE_CYCLE_COUNTER(STAT_MobPool_Tick);
	//DrawDebugViewFrustum(0.0f);
	//DrawLODDebugBoundaries();
	if (IsLOD)
	{
		UpdateMobLOD();
	}

}
void AMobPoolManager::DrawLODDebugBoundaries()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// 뷰포트 사이즈 가져오기
	int32 ViewportSizeX, ViewportSizeY;
	PC->GetViewportSize(ViewportSizeX, ViewportSizeY);

	// 그릴 경계선 비율과 색상 정의
	// 0.0f: 실제 뷰포트, ViewportMarginRatio: LOD1 경계
	const float Ratios[] = { 0.0f, ViewportMarginRatio };
	const FColor Colors[] = { FColor::Green, FColor::Yellow };

	// 플레이어 캐릭터의 높이를 기준으로 Z 평면 설정
	float DrawPlaneZ = 50.0f; // 기본 높이
	APawn* PlayerPawn = PC->GetPawn();
	if (PlayerPawn)
	{
		DrawPlaneZ = PlayerPawn->GetActorLocation().Z;
	}

	// 각 경계선(뷰포트, LOD 마진)을 순회하며 그리기
	for (int i = 0; i < 2; ++i)
	{
		const float CurrentRatio = Ratios[i];
		const FColor CurrentColor = Colors[i];

		// 마진 값 계산
		const float MarginX = ViewportSizeX * CurrentRatio;
		const float MarginY = ViewportSizeY * CurrentRatio;

		// 마진을 적용한 4개의 화면 코너 좌표
		const FVector2D ScreenCorners[4] = {
			FVector2D(0 - MarginX, 0 - MarginY),                          // 좌상단
			FVector2D(ViewportSizeX + MarginX, 0 - MarginY),              // 우상단
			FVector2D(ViewportSizeX + MarginX, ViewportSizeY + MarginY),  // 우하단
			FVector2D(0 - MarginX, ViewportSizeY + MarginY)               // 좌하단
		};

		FVector WorldCorners[4];
		bool bAllCornersValid = true;

		// 화면 좌표를 월드 좌표로 변환 (Deprojection)
		for (int j = 0; j < 4; ++j)
		{
			FVector WorldLocation, WorldDirection;
			if (PC->DeprojectScreenPositionToWorld(ScreenCorners[j].X, ScreenCorners[j].Y, WorldLocation, WorldDirection))
			{
				// 카메라 위치에서 Z 평면까지의 교차점 계산
				float t = (DrawPlaneZ - WorldLocation.Z) / WorldDirection.Z;
				WorldCorners[j] = WorldLocation + (WorldDirection * t);
			}
			else
			{
				bAllCornersValid = false;
				break;
			}
		}

		// 월드 좌표를 기준으로 디버그 라인 그리기
		if (bAllCornersValid)
		{
			for (int j = 0; j < 4; ++j)
			{
				DrawDebugLine(
					GetWorld(),
					WorldCorners[j],
					WorldCorners[(j + 1) % 4], // 다음 코너와 연결 (순환)
					CurrentColor,
					false,    // 프레임마다 그림
					-1.0f,    // 지속 시간 (1프레임)
					0,        // 깊이 우선순위
					2.0f      // 두께
				);
			}
		}
	}
}
void AMobPoolManager::UpdateMobLOD()
{
	SCOPE_CYCLE_COUNTER(STAT_MobPool_UpdateLOD);

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// 스레드에 넘겨줄 읽기 전용 데이터 준비
	// Actor 함수나 PC 함수는 스레드에서 못 쓰니까 미리 변수로 추출해야 함
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

	// 5000개를 코어 수만큼 나눠서 동시에 실행함
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

void AMobPoolManager::DrawDebugViewFrustum(float MarginRatio)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	// 뷰포트 크기 가져오기
	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);

	float MarginX = SizeX * MarginRatio;
	float MarginY = SizeY * MarginRatio;

	//화면의 4개 꼭짓점 좌표 정의 (Margin 적용)
	// 화면 좌표계: (0,0)이 좌상단
	FVector2D ScreenCorners[4];
	ScreenCorners[0] = FVector2D(-MarginX, -MarginY);               // 좌상
	ScreenCorners[1] = FVector2D(SizeX + MarginX, -MarginY);        // 우상
	ScreenCorners[2] = FVector2D(SizeX + MarginX, SizeY + MarginY); // 우하
	ScreenCorners[3] = FVector2D(-MarginX, SizeY + MarginY);        // 좌하

	FVector WorldCorners[4];

	// 화면 좌표를 월드 좌표로 변환 (Deprojection)
	for (int32 i = 0; i < 4; i++)
	{
		FVector WorldLoc, WorldDir;
		if (PC->DeprojectScreenPositionToWorld(ScreenCorners[i].X, ScreenCorners[i].Y, WorldLoc, WorldDir))
		{
			float PlaneZ = 50.0f; // 몬스터 허리 높이
			if (FMath::IsNearlyZero(WorldDir.Z)) return;

			float t = (PlaneZ - WorldLoc.Z) / WorldDir.Z;
			WorldCorners[i] = WorldLoc + (WorldDir * t);
		}
	}

	// 선 그리기
	for (int32 i = 0; i < 4; i++)
	{
		DrawDebugLine(GetWorld(), WorldCorners[i], WorldCorners[(i + 1) % 4], FColor::Yellow, false, -1.0f, 0, 5.0f);
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

	// 마진 적용해서 범위 체크
	// X, Y가 -1 ~ 1 사이면 화면 안임.
	// MarginRatio가 0.2면 -1.2 ~ 1.2 까지 허용
	float Margin = 1.0f + (MarginRatio * 2.0f); // -1~1 구간이므로 비율 보정

	return (X >= -Margin && X <= Margin) && (Y >= -Margin && Y <= Margin);
}

