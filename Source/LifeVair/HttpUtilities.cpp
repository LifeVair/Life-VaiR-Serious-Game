// Fill out your copyright notice in the Description page of Project Settings.


#include "HttpUtilities.h"
#include "HttpModule.h"

// Sets default values
AHttpUtilities::AHttpUtilities()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AHttpUtilities::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AHttpUtilities::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void AHttpUtilities::UpdateData(const FString URI)
{
	//Write my get request here !
	const auto Request = FHttpModule::Get().CreateRequest();
	Request->SetURL(URI);
	Request->OnProcessRequestComplete().BindLambda([&](
		FHttpRequestPtr request,
		FHttpResponsePtr Response,
		const bool Success)  
	{
		if(Success)
		{
			UE_LOG(LogActor, Warning, TEXT("%s"), *Response );
		}
		else {
			switch (Request->GetStatus()) {
			case EHttpRequestStatus::Failed_ConnectionError:
				UE_LOG(LogTemp, Error, TEXT("Connection failed."));
			default:
				UE_LOG(LogTemp, Error, TEXT("Request failed."));
			}
		}
	});
	
	//launch request to the server 
	Request->ProcessRequest();

	
}
