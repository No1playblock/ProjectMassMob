// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "SimpleMovementFragment.h"
#include "SimpleMovementTrait.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "MySimple Movement"))
class PROJECTMASSMOB_API USimpleMovementTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()
	
protected:
	// 엔티티 템플릿을 만들 때 호출됨
	virtual void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
