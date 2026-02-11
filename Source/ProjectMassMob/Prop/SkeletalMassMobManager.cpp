// Fill out your copyright notice in the Description page of Project Settings.


#include "Prop/SkeletalMassMobManager.h"
#include "Kismet/GameplayStatics.h"
#include "Async/ParallelFor.h"

ASkeletalMassMobManager::ASkeletalMassMobManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.0f;
}

void ASkeletalMassMobManager::BeginPlay()
{
	Super::BeginPlay();

	PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	if (MonsterClass)
	{
		MobActorPool.Reserve(MaxPoolSize);
		MobDataArray.Reserve(MaxPoolSize);

		for (int32 i = 0; i < MaxPoolSize; ++i)
		{
			FVector TempLoc(0.0f, 0.0f, -10000.0f);
			FRotator TempRot = FRotator::ZeroRotator;

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			// SkeletalMassMonster 생성
			ASkeletalMassMonster* NewMob = GetWorld()->SpawnActor<ASkeletalMassMonster>(MonsterClass, TempLoc, TempRot, SpawnParams);

			if (NewMob)
			{
				NewMob->SetActiveState(false);
				MobActorPool.Add(NewMob);

				FSkeletalMassMobData NewData;
				NewData.ActorIndex = i;
				NewData.bIsActive = false;
				NewData.MoveSpeed = DefaultMoveSpeed;

				MobDataArray.Add(NewData);
			}
		}
	}
}

void ASkeletalMassMobManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!PlayerPawn.IsValid()) return;

	FVector PlayerLocation = PlayerPawn->GetActorLocation();
	int32 TotalCount = MobDataArray.Num();

	// 병렬 처리 (이동 로직 계산)
	ParallelFor(TotalCount, [&](int32 Index)
		{
			FSkeletalMassMobData& CurrentData = MobDataArray[Index];

			if (!CurrentData.bIsActive) return;

			// 방향 계산 (Z축 무시)
			FVector DirectionToPlayer = PlayerLocation - CurrentData.Location;
			DirectionToPlayer.Z = 0.0f;

			FVector NormalizedDir = DirectionToPlayer.GetSafeNormal2D();

			// 위치 업데이트
			CurrentData.Location += NormalizedDir * CurrentData.MoveSpeed * DeltaTime;

			// 회전 업데이트
			if (!NormalizedDir.IsNearlyZero())
			{
				CurrentData.Direction = NormalizedDir;
			}
		});

	// 메인 스레드 동기화 (액터 이동)
	for (int32 i = 0; i < TotalCount; ++i)
	{
		const FSkeletalMassMobData& Data = MobDataArray[i];

		if (Data.bIsActive)
		{
			ASkeletalMassMonster* TargetActor = MobActorPool[Data.ActorIndex];
			if (TargetActor)
			{
				TargetActor->UpdateTransform(Data.Location, Data.Direction.Rotation());
			}
		}
	}
}

void ASkeletalMassMobManager::SpawnMonster(const FVector& SpawnLocation)
{
	// 빈 몬스터 찾기
	for (int32 i = 0; i < MobDataArray.Num(); ++i)
	{
		if (!MobDataArray[i].bIsActive)
		{
			// 데이터 활성화
			MobDataArray[i].bIsActive = true;
			MobDataArray[i].Location = SpawnLocation;
			MobDataArray[i].Location.Z = 88.0f; // 높이 보정

			// 액터 활성화
			ASkeletalMassMonster* TargetActor = MobActorPool[i];
			if (TargetActor)
			{
				UE_LOG(LogTemp, Log, TEXT("Spawning Skeletal Monster Num: %d"), count++);
				TargetActor->UpdateTransform(SpawnLocation, FRotator::ZeroRotator);
				TargetActor->SetActiveState(true);
			}
			return;
		}
	}
}

void ASkeletalMassMobManager::ClearAllMonsters()
{
	for (int32 i = 0; i < MobDataArray.Num(); ++i)
	{
		MobDataArray[i].bIsActive = false;
		if (MobActorPool[i])
		{
			MobActorPool[i]->SetActiveState(false);
		}
	}
}

