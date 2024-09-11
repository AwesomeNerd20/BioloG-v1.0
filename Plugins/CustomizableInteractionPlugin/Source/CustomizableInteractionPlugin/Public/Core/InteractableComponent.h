// Copyright Vinipi Studios 2024. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "Utils/InteractableData.h"
#include "InteractableComponent.generated.h"

class UInteractableData;
class APawn;
class UInteractorComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateWidgetInfo, UInteractableComponent*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnOverlapUpdated, bool, bIsOverlapping, UInteractorComponent*, OverlappingInteractor, UInteractableComponent*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnHoverUpdated, bool, bIsHovering, UInteractorComponent*, HoveringInteractor, UInteractableComponent*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnInteractionInputToggled, bool, bIsInputPressed, UInteractorComponent*, HoveringInteractor, UInteractableComponent*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractionPercentageUpdated, float, Percentage, UInteractableComponent*, Interactable);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteract, UInteractorComponent*, Interactor, UInteractableComponent*, Interactable);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), AutoExpandCategories = ("Behaviour"))
class CUSTOMIZABLEINTERACTIONPLUGIN_API UInteractableComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*
	 * Make sure we are replicating the desired data
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; 

protected:
	virtual void BeginPlay() override;

	//////////////////////////////////////////// Member Variables ////////////////////////////////////////////////////

/*	
 * Behaviour
*/

	// Determine if the detection should be activated by default
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Behaviour")
	bool bStartWithInteractionActive = true;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = "OnReplicated_InteractableData", Category = "Behaviour")
	FInteractableData InteractableData{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Behaviour")
	TArray<TObjectPtr<AActor>> ActorsToIgnoreOnObstructionTrace{};
	
private:
	
	float CurrentInteractionTime = 0.f;
	float TimeSinceInteraction = 0.5f;

	UPROPERTY(ReplicatedUsing = "OnReplicated_bInteractionActive")
	bool bInteractionActive = true;
	
	bool bIsBeingOverlapped = false;
	bool bIsBeingHovered = false;

	bool bIsObstructed = false;
	
// Delegates
public:
	UPROPERTY(BlueprintAssignable, Category = "Behaviour")
	FOnUpdateWidgetInfo OnUpdateWidgetInfo;
	UPROPERTY(BlueprintAssignable, Category = "Behaviour")
	FOnOverlapUpdated OnOverlapUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Behaviour")
	FOnHoverUpdated OnHoverUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Behaviour")
	FOnInteractionInputToggled OnInteractionInputToggled;
	UPROPERTY(BlueprintAssignable, Category = "Behaviour")
	FOnInteractionPercentageUpdated OnInteractionPercentageUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Behaviour")
	FOnInteract OnInteract;

//////////////////////////////////////////// Member Functions ////////////////////////////////////////////////////

/*
 * Behaviour
 */

protected:
	void RefreshInteractableData();

	void InitializeWidgetReferences();

	UFUNCTION()
	void OnReplicated_InteractableData();

public:
	UFUNCTION()
	void UpdateOverlap(const bool bIsOverlapping, UInteractorComponent* OverlappingInteractorComponent);

	UFUNCTION()
	void UpdateHover(const bool bIsHovering, UInteractorComponent* HoveringInteractorComponent);

	UFUNCTION()
	void Interact(UInteractorComponent* Interactor);

	UFUNCTION()
	void OnReplicated_bInteractionActive();

	UFUNCTION()
	bool ExecuteObstructionTrace(UInteractorComponent* Interactor);

/*	
 * Getter's
 */

public:
	UFUNCTION(BlueprintCallable, Category = "Data")
	FORCEINLINE FInteractableData GetInteractableData() const { return InteractableData; }

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	FORCEINLINE bool IsInteractionActive() const { return bInteractionActive; }

	UFUNCTION(BlueprintPure, Category = "Detection")
	FORCEINLINE bool GetIsBeingOverlapped() const { return bIsBeingOverlapped; }

	UFUNCTION(BlueprintPure, Category = "Detection")
	FORCEINLINE bool GetIsBeingHovered() const { return bIsBeingHovered; }

	UFUNCTION()
	FORCEINLINE bool GetIsObstructed() const { return bIsObstructed; }

	UFUNCTION()
	FORCEINLINE float GetInteractionTime() const { return CurrentInteractionTime; }
	
/*
 * Setter's
 */

public:
	UFUNCTION()
	float SetCurrentInteractionTime(const float NewInteractionTime, UInteractorComponent* Interactor);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractionActive(bool bNewActive);

/*
 * Extras
 */

public:
	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetInteractableDataBasedOnDataTable(UDataTable* DataTable, FName RowName);

	// Set Display Text
	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetDisplayText(const FString NewText = "Placeholder Text");

	// Set Tooltip Text
	UFUNCTION(BlueprintCallable, Category = "Data")
	void SetTooltipText(const FString NewText = "Placeholder Text");
};
