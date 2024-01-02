// Copyright : OK


#include "CartridgeSlot.h"

// Sets default values for this component's properties
UCartridgeSlot::UCartridgeSlot()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	isFull = true;
	isEnabled = true;
}


// Called when the game starts
void UCartridgeSlot::BeginPlay()
{
	Super::BeginPlay();

	// ...
//	OnComponentBeginOverlap.AddDynamic(this, &UCartridgeSlot::OnOverlapBegin);
}

void UCartridgeSlot::SetSlotEnabled(bool EnableValue)
{
	isEnabled = EnableValue;
}

bool UCartridgeSlot::GetSlotEnabled() const
{
	return isEnabled;
}

void UCartridgeSlot::SetSlotIsFull(bool isFullValue)
{
	if (isFullValue)
	{
		CartridgeFilled();
	}
	else
	{
		CartridgeEmptied();
	}
	isFull = isFullValue;
}

bool UCartridgeSlot::GetSlotIsFull() const
{
	return isFull;
}

void UCartridgeSlot::CartridgeFilled() const
{
	OnCartridgeFilled.Broadcast();
}

void UCartridgeSlot::CartridgeEmptied() const
{
	OnCartridgeEmptied.Broadcast();
}