// Copyright Vinipi Studios 2024. All rights reserved.

#include "Core/InteractableComponent.h"

#include "Components/WidgetComponent.h"
#include "Utils/InteractableData.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "Core/InteractionIndicatorInterface.h"
#include "Core/InteractorComponent.h"
#include "Kismet/KismetSystemLibrary.h"

UInteractableComponent::UInteractableComponent()
{
	// Set component to be replicated by default, avoiding confusion in the BP part
	SetIsReplicatedByDefault(true);
	
	// Set the widget default settings
	SetWidgetSpace(EWidgetSpace::Screen);
	SetDrawAtDesiredSize(true);
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	// Set the obstruction trace to ignore itself
	ActorsToIgnoreOnObstructionTrace.Add(GetOwner());
	
	// Initialize the widget
	InitializeWidgetReferences();

	// Toggle interaction
	SetInteractionActive(bStartWithInteractionActive);
}

// Set values to replicate
inline void UInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInteractableComponent, InteractableData);
	DOREPLIFETIME(UInteractableComponent, bInteractionActive);
	DOREPLIFETIME(UInteractableComponent, ActorsToIgnoreOnObstructionTrace)
}

void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update the time since the last interaction. Used to avoid players interacting multiple times with the object at once
	TimeSinceInteraction += DeltaTime;
}

/*	
 * Behaviour
 */

void UInteractableComponent::RefreshInteractableData()
{
	// Refresh the widget
	InitializeWidgetReferences();

	// Refresh the widget with the new data
	OnUpdateWidgetInfo.Broadcast(this);
}

void UInteractableComponent::InitializeWidgetReferences()
{
	// Create the widget
	InitWidget();

	// Make Sure the widget is valid
	if (!IsValid(GetWidget()))
		return;

	// Initialize the references inside the widget
	if (GetWidget()->Implements<UInteractionIndicatorInterface>())
	{
		IInteractionIndicatorInterface::Execute_InitializeReferences(GetWidget(), this);
	}

	// Toggle the widget visibility based on the active state of the interaction
	if (IsValid(GetWidget()))
		GetWidget()->SetVisibility(bInteractionActive ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

void UInteractableComponent::UpdateOverlap(const bool bIsOverlapping, UInteractorComponent* OverlappingInteractorComponent)
{
	// get a reference to the overlapping pawn
	APawn* OverlappingPawn = Cast<APawn>(OverlappingInteractorComponent->GetOwner());
	if (!IsValid(OverlappingPawn))
		return;

	// set if this interactable is being overlapped
	bIsBeingOverlapped = bIsOverlapping;

	// update the widget info if the interactable get's overlapped
	if (bIsBeingOverlapped)
		OnUpdateWidgetInfo.Broadcast(this);

	// Say that this interactable have been overlapped
	OnOverlapUpdated.Broadcast(bIsOverlapping, OverlappingInteractorComponent, this);
}

void UInteractableComponent::UpdateHover(const bool bIsHovering, UInteractorComponent* HoveringInteractorComponent)
{
	// get a reference to the hovering pawn
	APawn* HoveringPawn = Cast<APawn>(HoveringInteractorComponent->GetOwner());
	if (!IsValid(HoveringPawn))
		return;

	// set if this interactable is being hovered
	bIsBeingHovered = bIsHovering;

	// update the widget info if the interactable get's hovered
	if (bIsHovering)
		OnUpdateWidgetInfo.Broadcast(this);	

	// warn that this interactable have been hovered
	OnHoverUpdated.Broadcast(bIsHovering, HoveringInteractorComponent, this);
}

// Used for replication
void UInteractableComponent::OnReplicated_InteractableData()
{
	RefreshInteractableData();
}

/*
 * Interaction
 */

void UInteractableComponent::Interact(UInteractorComponent* Interactor)
{
	OnInteract.Broadcast(Interactor, this);
}

float UInteractableComponent::SetCurrentInteractionTime(const float NewInteractionTime, UInteractorComponent* Interactor)
{
	// make sure that the interaction is between the desired time
	if (TimeSinceInteraction < GetInteractableData().DelayBetweenInteraction)
		return 0.f;
	
	CurrentInteractionTime = NewInteractionTime;

	// convert time to percentage
	const FVector2d Range{0.0f, 1};
	const FVector2d Time{0, GetInteractableData().InteractionDuration};
	const float Percent = FMath::GetMappedRangeValueClamped(Time, Range, NewInteractionTime);

	// if the interaction is complete
	if (Percent >= 1.f)
	{
		// reset the time since last interaction and the current interaction time
		TimeSinceInteraction = 0.f;
		CurrentInteractionTime = 0.f;
		
		// warn that the interaction percentage has been updated
		OnInteractionPercentageUpdated.Broadcast(0.f, this);
		
		return Percent;
	}

	// warn that the interaction percentage has been updated
	OnInteractionPercentageUpdated.Broadcast(Percent, this);

	return Percent;
}

// Activate/Deactivate Interaction
void UInteractableComponent::SetInteractionActive(bool bNewActive)
{
	// Make sure this function only executes on the server.
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
		return;
	
	// update the values
	bInteractionActive = bNewActive;
	
	OnReplicated_bInteractionActive();
}

void UInteractableComponent::OnReplicated_bInteractionActive()
{
	// reset the interaction values
	if (!bInteractionActive)
	{
		bIsBeingOverlapped = false;
		bIsBeingHovered = false;
	}
	
	// update the widget's visibility
	if (IsValid(GetWidget()))
		GetWidget()->SetVisibility(bInteractionActive ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
}

bool UInteractableComponent::ExecuteObstructionTrace(UInteractorComponent* Interactor)
{
	// If obstruction doesn't matter for this interactable, we don't need to perform the trace
	if (InteractableData.bDetectWhenObstructed && InteractableData.bAllowInteractionWhenObstructed)
	{
		bIsObstructed = false;
		return false;
	}

	// Interactor must be valid
	ensure(IsValid(Interactor));
	
	APlayerController* Controller = Cast<APlayerController>(Interactor->GetOwner()->GetInstigatorController());
	// Controller must be valid
	ensure(IsValid(Controller));

	// get the trace transform
	FVector StartLocation;
	FRotator StartRotation;
	Controller->GetActorEyesViewPoint(StartLocation, StartRotation);

	// get the trace collision
	const ETraceTypeQuery TraceTypeQuery =  UEngineTypes::ConvertToTraceType(InteractableData.ObstructionTraceChannel);
	FHitResult ObstructionResult{};

	// Make sure the interactor will not interfere in the trace
	ActorsToIgnoreOnObstructionTrace.AddUnique(Interactor->GetOwner());

	// perform the trace
	UKismetSystemLibrary::LineTraceSingle(GetWorld(), StartLocation, GetComponentLocation(), TraceTypeQuery, false, ActorsToIgnoreOnObstructionTrace, Interactor->GetDrawDebugType(), ObstructionResult, true, FLinearColor::Red, FLinearColor::Green, Interactor->GetDetectionFrequency());

	// update the obstruction state
	bIsObstructed = ObstructionResult.bBlockingHit;
	
	ActorsToIgnoreOnObstructionTrace.Remove(Interactor->GetOwner());
	
	return bIsObstructed;
}

/*
 * Extra
 */

void UInteractableComponent::SetInteractableDataBasedOnDataTable(UDataTable* DataTable, FName RowName)
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
		return;
	
	// try find the interactable data inside the desired data table. if it is not valid we should just ignore this function to avoid crashes.
	FInteractableData* TempData = DataTable->FindRow<FInteractableData>(RowName, "Couldn't Find Row", false) ;
	if (!TempData)
	{
		UKismetSystemLibrary::PrintString(GetWorld(), "Failed to Change Interactable Data. Data Table Row is Invalid");
		return;
	}

	InteractableData = *TempData;

	// warn the user that he needs to update the widget data
	OnUpdateWidgetInfo.Broadcast(this);
}

void UInteractableComponent::SetDisplayText(const FString NewText)
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
		return;
	
	if (InteractableData.DisplayText.ToString() == NewText)
		return;

	InteractableData.DisplayText = FText::FromString(NewText);
	OnReplicated_InteractableData();
}

void UInteractableComponent::SetTooltipText(const FString NewText)
{
	if (GetOwner()->GetLocalRole() < ROLE_Authority)
		return;
	
	if (InteractableData.TooltipText.ToString() == NewText)
		return;

	InteractableData.TooltipText = FText::FromString(NewText);
	OnReplicated_InteractableData();
}
