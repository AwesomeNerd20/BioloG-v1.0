// Copyright Vinipi Studios 2024. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionIndicatorInterface.generated.h"

class UInteractableComponent;
// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UInteractionIndicatorInterface : public UInterface
{
	GENERATED_BODY()
};

class CUSTOMIZABLEINTERACTIONPLUGIN_API IInteractionIndicatorInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void InitializeReferences(UInteractableComponent* InteractableComponent);
};
