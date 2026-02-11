#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h" // 
#include "SimpleMovementFragment.generated.h"

// 이동 속도 데이터를 담은 프래그먼트
USTRUCT()
struct FSimpleMovementFragment : public FMassFragment
{
	GENERATED_BODY()

	// 기본 속도 300
	UPROPERTY(EditAnywhere)
	float Speed = 300.0f;
};