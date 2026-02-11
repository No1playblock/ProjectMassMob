// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassProcessor.h"
#include "SimpleMovementProcessor.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTMASSMOB_API USimpleMovementProcessor : public UMassProcessor
{
	GENERATED_BODY()

public:
	USimpleMovementProcessor();

protected:
	// 어떤 데이터를 가진 엔티티를 쿼리할지 설정
	virtual void ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager) override;
	// 실제 실행 함수 (Tick 역할)
	virtual void Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context) override;

private:
	// 쿼리 객체
	FMassEntityQuery EntityQuery;
};
