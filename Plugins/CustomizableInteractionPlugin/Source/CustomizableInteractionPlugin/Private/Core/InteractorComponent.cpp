// Copyright Vinipi Studios 2024. All rights reserved.

#include "Core/InteractorComponent.h"

#include "Core/InteractableComponent.h"
#include "Utils/InteractableData.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	FTimerDelegate ToggleDetectionDelegate = FTimerDelegate::CreateUObject(this, &ThisClass::SetDetectionActive, bStartWithInteractionActive);
	GetWorld()->GetTimerManager().SetTimerForNextTick(ToggleDetectionDelegate);
}

void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!bDetectionActive)
		return;

	UpdateDetection(DeltaTime);

	UpdateInteraction(DeltaTime);
}

void UInteractorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

/*
 * Detection
 */

void UInteractorComponent::SetDetectionActive(const bool bNewActive)
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn->IsLocallyControlled())
		return;

	APlayerController* PlayerController = Cast<APlayerController>(Pawn->GetController());
	if (!IsValid(PlayerController))
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Tried Toggling Interaction on a Non-Controlled Character, This could Result in Crashes, Please fix this asap");
		bDetectionActive = false;
		
		return;
	}
	
	bDetectionActive = bNewActive;
	
	if (!bDetectionActive)
	{
		for (auto Temp : OverlappingInteractables)
		{
			Temp->UpdateOverlap(false, this);
			if (Temp == HoveringInteractable)
			{
				Temp->UpdateHover(false, this);
				Temp->SetCurrentInteractionTime(0.f, this);
				HoveringInteractable == nullptr;
			}
		}
		OverlappingInteractables.Empty();
	}
}

void UInteractorComponent::UpdateDetection(float DeltaTime)
{
	TimeSinceLastUpdate += DeltaTime;
	if (TimeSinceLastUpdate >= DetectionFrequency)
	{
		TimeSinceLastUpdate = 0.f;
		
		UpdateOverlappingInteractables();
		UpdateHoveringInteractables();
	}
}

TArray<UActorComponent*> UInteractorComponent::DetectionTrace()
{
	// Create temporary variables
	FVector WorldLocation = GetOwner()->GetActorLocation();
	ETraceTypeQuery DetectionTraceQuery = UEngineTypes::ConvertToTraceType(DetectionChannel);
	TArray<UActorComponent*> DetectedComponents;

	// Do the Trace
	TArray<FHitResult> HitResults;
	UKismetSystemLibrary::SphereTraceMulti(GetWorld(), WorldLocation, WorldLocation, DetectionRadius, DetectionTraceQuery, false,
		{ GetOwner() }, GetDrawDebugType(), HitResults, true, FLinearColor::Red, FLinearColor::Green, DetectionFrequency);

	// Se if the actors detected by the trace have interactable components	
	for (const FHitResult& HitResult : HitResults)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			TArray<UActorComponent*> TempComponents;
			HitActor->GetComponents(UInteractableComponent::StaticClass(), TempComponents, true);
			DetectedComponents.Append(TempComponents);
		}
	}

	return DetectedComponents;
}

void UInteractorComponent::UpdateOverlappingInteractables()
{
	// get the interactable components detected by the trace
	TArray<UActorComponent*> DetectedInteractables = DetectionTrace();

	// loop for the detected components
	for (auto TempActorComponent : DetectedInteractables)
	{
		// make sure this component was not detected previously
		if (OverlappingInteractables.Contains(TempActorComponent))
			continue;

		// make sure it is an interactable component
		UInteractableComponent* TempInteractable = Cast<UInteractableComponent>(TempActorComponent);
		if (!IsValid(TempInteractable))
			continue;

		// if the interaction is active, we should add the component to the overlapping interactables array
		if (TempInteractable->IsInteractionActive())
		{
			OverlappingInteractables.AddUnique(TempInteractable);
		}
	}

	// loop throught the overlapping interactables array
	OverlappingInteractables.RemoveAllSwap([this](UInteractableComponent* TempInteractable)
	{
		// if the interactable is not valid, we should remove it from the array
		if (!IsValid(TempInteractable))
			return true;
		
		const bool bIsBeingOverlapped = TempInteractable->GetIsBeingOverlapped();
		const bool bIsBeingHovered = TempInteractable->GetIsBeingHovered() && HoveringInteractable == TempInteractable;

		// See if the interactable is obstructed
		TempInteractable->ExecuteObstructionTrace(this);
		const bool bIsObstructed = !TempInteractable->GetInteractableData().bDetectWhenObstructed && TempInteractable->GetIsObstructed();

		// if the interaction for that interactable is not active, we should remove it from the array
		if (!TempInteractable->IsInteractionActive())
		{
			if (bIsBeingOverlapped)
				TempInteractable->UpdateOverlap(false, this);
			
			if (bIsBeingHovered)
			{
				TempInteractable->UpdateHover(false, this);
				HoveringInteractable == nullptr;
			}
			
			return true;
		}

		// variables to make the checks easier
		const float DistanceFromOrigin = FVector::Distance(GetOwner()->GetActorLocation(), TempInteractable->GetComponentLocation());
		const bool bInOverlapRange = DistanceFromOrigin <= TempInteractable->GetInteractableData().MaxOverlapDistance;
		
		if (bInOverlapRange && !bIsBeingOverlapped && !bIsObstructed)
		{
			TempInteractable->UpdateOverlap(true, this);
			return false;
		}
		else if ((!bInOverlapRange || bIsObstructed) && bIsBeingOverlapped)
		{
			TempInteractable->UpdateOverlap(false, this);
			
			if (bIsBeingHovered)
			{
				TempInteractable->UpdateHover(false, this);
				HoveringInteractable == nullptr;
			}
				
			return true;
		}
		
		return false;
	});
}

void UInteractorComponent::UpdateHoveringInteractables()
{
	UInteractableComponent* UpdatedInteractable = SelectHoveringInteractable();

	if (UpdatedInteractable != HoveringInteractable)
	{
		if (IsValid(HoveringInteractable))
		{
			HoveringInteractable->UpdateHover(false, this);
			HoveringInteractable->SetCurrentInteractionTime(0.f, this);
		}

		HoveringInteractable = UpdatedInteractable;
		if (IsValid(HoveringInteractable))
		{
			HoveringInteractable->UpdateHover(true, this);
			HoveringInteractable->SetCurrentInteractionTime(0.f, this);
		}
	}
}

UInteractableComponent* UInteractorComponent::SelectHoveringInteractable()
{
	if (OverlappingInteractables.IsEmpty())
		return nullptr;
	
	UInteractableComponent* TempHoveringComponent {};
	float TargetDistanceFromOrigin = DetectionRadius;
	const FVector CharacterLocation = GetOwner()->GetActorLocation();
	const FVector CharacterDirection = GetOwner()->GetActorRotation().Vector();

	// Used on Camera Proximity detection mode
	FVector2d ScreenSize;
	GEngine->GameViewport->GetViewportSize(ScreenSize);
	const FVector2d ScreenCenter = ScreenSize/2;
	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	
	for (auto Temp : OverlappingInteractables)
	{
		if (!Temp->GetInteractableData().bAllowInteractionWhenObstructed && Temp->GetIsObstructed())
			continue;

		// Initialize default values relative to character
		const float DistanceFromCharacter = FVector3d::Distance(CharacterLocation, Temp->GetComponentLocation());;
		float DistanceFromOrigin = DistanceFromCharacter;

		const bool bInHoverRangeCharacterRelative = DistanceFromCharacter <= Temp->GetInteractableData().MaxHoverDistanceFromCharacter;
		bool bInHoverRangeCameraRelative = true;

		// If detection mode is from camera proximity, we need to recalculate the distance
		if (DetectionMode == EDetectionMode::DM_CameraProximity)
		{
			const bool bInFrontOfCharacter = FVector::DotProduct(CharacterDirection, (Temp->GetComponentLocation() - CharacterLocation).GetSafeNormal()) > 0;
			if (!bInFrontOfCharacter)
				continue;
			
			FVector2d ScreenPosition;
			UGameplayStatics::ProjectWorldToScreen(PlayerController, Temp->GetComponentLocation(), ScreenPosition);

			DistanceFromOrigin = FVector2D::Distance(ScreenCenter, ScreenPosition);
			bInHoverRangeCameraRelative = DistanceFromOrigin <= Temp->GetInteractableData().MaxHoverDistanceFromCharacter;
		}
		
		if (bInHoverRangeCharacterRelative && bInHoverRangeCameraRelative && DistanceFromOrigin <= TargetDistanceFromOrigin)
		{
			TargetDistanceFromOrigin = DistanceFromOrigin;
			TempHoveringComponent = Temp;	
		}
	}

	return TempHoveringComponent;
}

/*
 * Interaction
 */

void UInteractorComponent::InteractionInput(FString Key, bool bPressed)
{
	if (bPressed)
	{
		ActiveKeys.AddUnique(Key);

		if (!IsValid(HoveringInteractable))
			return;

		if (!HoveringInteractable->GetInteractableData().Keys.Contains(Key))
			return;

		if (HoveringInteractable->GetInteractableData().InteractionType == EInteractionType::EIT_Hold)
			return;

		HoveringInteractable->SetCurrentInteractionTime(HoveringInteractable->GetInteractionTime(), this);
		Interact(HoveringInteractable);
	}
	else
	{
		ActiveKeys.Remove(Key);

		if (!IsValid(HoveringInteractable))
			return;
		
		if (!HoveringInteractable->GetInteractableData().Keys.Contains(Key))
			return;

		HoveringInteractable->SetCurrentInteractionTime(0.f, this);
	}
}

void UInteractorComponent::UpdateInteraction(float DeltaTime)
{
	if (ActiveKeys.IsEmpty())
		return;

	if (!IsValid(HoveringInteractable))
		return;

	for (auto Key : ActiveKeys)
	{
		if (!HoveringInteractable->GetInteractableData().Keys.Contains(Key))
			continue;

		if (HoveringInteractable->GetInteractableData().InteractionType == EInteractionType::EIT_Press)
			continue;

		const float Progress = HoveringInteractable->SetCurrentInteractionTime(HoveringInteractable->GetInteractionTime() + DeltaTime, this);

		if (Progress >= 1.f)
		{
			Interact(HoveringInteractable);
		} 
	}
}

void UInteractorComponent::SetDetectionMode(EDetectionMode NewDetectionMode)
{
	if (DetectionMode == NewDetectionMode)
		return;
	
	DetectionMode = NewDetectionMode;

	OnDetectionModeChanged.Broadcast(DetectionMode);
}

void UInteractorComponent::Interact_Implementation(UInteractableComponent* Interactable)
{
	const FVector CharacterLocation = GetOwner()->GetActorLocation();
	const float DistanceFromCharacter = FVector3d::Distance(CharacterLocation, Interactable->GetComponentLocation());
	const bool bIsObstructed = Interactable->ExecuteObstructionTrace(this) && !Interactable->GetInteractableData().bAllowInteractionWhenObstructed;

	if (bIsObstructed)
		return;
	
	if (DistanceFromCharacter > Interactable->GetInteractableData().MaxHoverDistanceFromCharacter)
		return;

	if (!Interactable->IsInteractionActive())
		return;
	
	Interactable->Interact(this);

	Interactable->SetCurrentInteractionTime(0.f, this);
}
