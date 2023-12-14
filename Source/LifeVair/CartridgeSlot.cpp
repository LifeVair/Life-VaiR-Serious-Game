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
}


// Called when the game starts
void UCartridgeSlot::BeginPlay()
{
	Super::BeginPlay();

	// ...
	OnComponentBeginOverlap.AddDynamic(this, &UCartridgeSlot::OnOverlapBegin);
	OnComponentEndOverlap.AddDynamic(this,&UCartridgeSlot::OnOverlapEnd);
}

void UCartridgeSlot::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}
