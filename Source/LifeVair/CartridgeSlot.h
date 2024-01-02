// Copyright : OK

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "CartridgeSlot.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMyCustomDelegate);
/*This class is used to create a custom collision box used to the cartridge*/
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class LIFEVAIR_API UCartridgeSlot : public UBoxComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCartridgeSlot();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cartridge Slot Settings")
	bool isFull;

	UPROPERTY(EditDefaultsOnly ,BlueprintSetter="SetSlotEnabled", BlueprintGetter="GetSlotEnabled",Category = "Cartridge Slot Settings")
	bool isEnabled;
	
	UPROPERTY(BlueprintAssignable, Category = "Cartridge Slot Manipulation")
	FMyCustomDelegate OnCartridgeEmptied;
	
	UPROPERTY(BlueprintAssignable, Category = "Cartridge Slot Manipulation")
	FMyCustomDelegate OnCartridgeFilled;
	
	

	UFUNCTION(BlueprintCallable, Category = "Cartridge Slot Settings")
	void SetSlotEnabled(bool EnableValue);

	UFUNCTION(BlueprintPure, Category = "Cartridge Slot Settings")
	bool GetSlotEnabled() const; 

	UFUNCTION(BlueprintCallable, Category = "Cartridge Slot Settings")
	void SetSlotIsFull(bool isFullValue);

	UFUNCTION(BlueprintPure, Category = "Cartridge Slot Settings")
	bool GetSlotIsFull() const;

	UFUNCTION(Category = "Cartridge Slot Settings")
	void CartridgeFilled() const;

	UFUNCTION(Category = "Cartridge Slot Settings")
	void CartridgeEmptied() const;
	

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
/*
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent,
	                    AActor* OtherActor,
	                    UPrimitiveComponent* OtherComponent,
	                    int32 OtherBodyIndex,
	                    bool bFromSweep,
	                    const FHitResult& SweepResult);
*/
};
