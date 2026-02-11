// Fill out your copyright notice in the Description page of Project Settings.


#include "ECS/SimpleMovementProcessor.h"
#include "SimpleMovementFragment.h"     // 내 속도 데이터
#include "MassCommonFragments.h"        // 언리얼 기본 Transform 데이터
#include "MassExecutionContext.h"

USimpleMovementProcessor::USimpleMovementProcessor()
{
	// 실행 순서 설정 (물리 엔진 돌기 전에 이동 처리)
	bAutoRegisterWithProcessingPhases = true;
	ProcessingPhase = EMassProcessingPhase::PrePhysics;
}

void USimpleMovementProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{

	//EntityQuery = FMassEntityQuery();

	//// 2. 데이터 요구사항 추가
	//EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	//EntityQuery.AddRequirement<FSimpleMovementFragment>(EMassFragmentAccess::ReadOnly);

	//// 3. [필수] 프로세서에 쿼리 등록 (이걸 해야 실제로 동작함)
	//EntityQuery.RegisterWithProcessor(*this);
}

void USimpleMovementProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	// 쿼리에 해당하는 모든 엔티티들을 덩어리(Chunk) 단위로 가져와서 처리
	EntityQuery.ForEachEntityChunk(EntityManager, Context, [](FMassExecutionContext& Context)
		{
			// 데이터 배열 뷰 가져오기
			TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
			TConstArrayView<FSimpleMovementFragment> Movements = Context.GetFragmentView<FSimpleMovementFragment>();

			// 델타 타임
			const float DeltaTime = Context.GetDeltaTimeSeconds();
			const int32 NumEntities = Context.GetNumEntities();

			// 반복문 실행 (여기서 CPU 최적화가 일어남)
			for (int32 i = 0; i < NumEntities; ++i)
			{
				FTransform& Tr = Transforms[i].GetMutableTransform();
				const float MoveSpeed = Movements[i].Speed;

				// 현재 바라보는 방향으로 전진
				FVector CurrentLoc = Tr.GetLocation();
				FVector ForwardDir = Tr.GetRotation().GetForwardVector();

				// 위치 갱신
				FVector NewLoc = CurrentLoc + (ForwardDir * MoveSpeed * DeltaTime);
				Tr.SetLocation(NewLoc);
			}
		});
}
