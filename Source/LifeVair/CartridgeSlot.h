// Copyright : OK

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "CartridgeSlot.generated.h"

/*This class is used to create a custom collision box used to the cartridge*/
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LIFEVAIR_API UCartridgeSlot : public UBoxComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCartridgeSlot();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Slot Settings")
	bool isFull;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	                    AActor* OtherActor,
	                    UPrimitiveComponent* OtherComponent,
	                    int32 OtherBodyIndex,
	                    bool bFromSweep,
	                    const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* Over,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
