// Copyright Vinipi Studios 2024. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "InteractorComponent.generated.h"

class UInteractableComponent;

UENUM(BlueprintType)
enum class EDetectionMode : uint8
{
	DM_CameraProximity UMETA(DisplayName="Camera Proximity"),
	DM_CharacterProximity UMETA(DisplayName="Character Proximity")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionModeChanged, EDetectionMode, NewDetectionMode);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CUSTOMIZABLEINTERACTIONPLUGIN_API UInteractorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractorComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; 
	
protected:
	virtual void BeginPlay() override;

//////////////////////////////////////////// Member Variables ////////////////////////////////////////////////////

/*
 * Detection
 */ 

private:
	
	// Detection's Properties
	UPROPERTY(EditDefaultsOnly, Category = "Detection")
	EDetectionMode DetectionMode = EDetectionMode::DM_CameraProximity;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	TEnumAsByte<ECollisionChannel> DetectionChannel = ECC_Visibility;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float DetectionFrequency = 0.1f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
	float DetectionRadius = 700.f;

protected:

	// Determine if the interaction should be enabled by default
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Detection")
	bool bStartWithInteractionActive = true;
	
	// Used Internally
	bool bDetectionActive = false;
	float TimeSinceLastUpdate = 0.f;

	// Reference to the interactables in the overlap range
	UPROPERTY()
	TArray<TObjectPtr<UInteractableComponent>> OverlappingInteractables;

	// Reference to the interactable in the overlap range
	UPROPERTY()
	TObjectPtr<UInteractableComponent> HoveringInteractable;

/*
 * Interaction
 */

private:	
	// The keys that are active for interaction
	UPROPERTY()
	TArray<FString> ActiveKeys = {};

/*
 * Delegates
 */

public:
	UPROPERTY(BlueprintAssignable, Category = "Detection")
	FOnDetectionModeChanged OnDetectionModeChanged;
	
/*
 * Debug
 */

protected:
	// Used for debug
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bShowDebugTraces = false;
	
//////////////////////////////////////////// Member Functions ////////////////////////////////////////////////////

/*
 * Detection
 */

public:

	// Toggle's the interaction on/off
	UFUNCTION(BlueprintCallable, Category = "Detection")
	void SetDetectionActive(const bool bNewActive);

private:

	// Used to update the whole interaction
	UFUNCTION()
	void UpdateDetection(float DeltaTime);

	// Execute the detection trace
	UFUNCTION()
	TArray<UActorComponent*> DetectionTrace();

	// Updates the Overlapping Interactables
	UFUNCTION()
	void UpdateOverlappingInteractables();

	// Updates the Hovering Interactable
	UFUNCTION()
	void UpdateHoveringInteractables();

	
	// Used to get the interactable available to be hovered
	UFUNCTION()
	UInteractableComponent* SelectHoveringInteractable();

/*
 * Interaction
 */
	
public:

	// Used to update the inputs
	UFUNCTION(BlueprintCallable, Category = "Input")
	void InteractionInput(FString Key = "DefaultKey", bool bPressed = false);

private:

	// Called every frame to update the interaction progress
	UFUNCTION()
	void UpdateInteraction(float DeltaTime);

	// Make's sure the interaction is allowed and perform the interaction. Called on the server
	UFUNCTION(Server, Reliable)
	void Interact(UInteractableComponent* Interactable);

/*
 * Setter's
 */

public:

	UFUNCTION(BlueprintCallable, Category = "Detection")
	void SetDetectionMode(EDetectionMode NewDetectionMode);
	
/*
 * Getter's
 */

public:

	// Used for Debug
	UFUNCTION()
	FORCEINLINE EDrawDebugTrace::Type GetDrawDebugType() const { return bShowDebugTraces ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None; }

	UFUNCTION()
	FORCEINLINE float GetDetectionFrequency() const { return DetectionFrequency; }

	UFUNCTION(BlueprintCallable, Category = "Detection")
	FORCEINLINE EDetectionMode GetDetectionMode() const { return DetectionMode; }
	
	// Get a reference to all the interactables that are currently being hovered
	UFUNCTION(BlueprintCallable, Category = "Detection")
	TArray<UInteractableComponent*> GetOverlappingInteractables() const { return OverlappingInteractables; }
	
	// Get a reference to the interactable in the overlap range
	UFUNCTION(BlueprintCallable, Category = "Detection")
	UInteractableComponent* GetHoveringInteractable() const { return HoveringInteractable; }
};
