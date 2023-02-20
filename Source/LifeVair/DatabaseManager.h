// Copyright : OK

#pragma once

#include "SQLiteDatabase.h"
#include "CoreMinimal.h"
#include "DatabaseManager.generated.h"

UCLASS(Blueprintable, DisplayName="SQLite DB Manager")
class LIFEVAIR_API UDatabaseManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, meta=(CompactNodeTitle=""), Category="SQLite DB Manager ")
	static TArray<FString>& getData(const FString Path);
	
};
