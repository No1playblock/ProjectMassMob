// Fill out your copyright notice in the Description page of Project Settings.


#include "ECS/SimpleMovementTrait.h"
#include "MassEntityTemplateRegistry.h" // 템플릿 빌드 컨텍스트

void USimpleMovementTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	// 이 특성을 가진 엔티티는 FSimpleMovementFragment 데이터를 가짐
	BuildContext.AddFragment<FSimpleMovementFragment>();
}
