// Copyright : OK
#include "DatabaseManager.h"

TArray<FString>& UDatabaseManager::getData(const FString Path)
{
	
	FSQLiteDatabase* SQLiteDB = new FSQLiteDatabase();
	SQLiteDB->Open(*Path, ESQLiteDatabaseOpenMode::ReadOnly);
	static TArray<FString> ResultColumns;
	UE_LOG(LogTemp, Warning, TEXT("Error while trying to access the DB"))
	
	if(SQLiteDB->IsValid())
	{
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Green, TEXT("COnnected to DB"));
		const FString Query = TEXT("select * from Domains");
		FSQLitePreparedStatement LoadStatement;
		LoadStatement.Reset();
		GEngine->AddOnScreenDebugMessage(-1, 50.0f, FColor::Blue, TEXT("Querying"));
		
		LoadStatement.Create(*SQLiteDB,*Query, ESQLitePreparedStatementFlags::Persistent);

		//IsSuccessful = LoadStatement.IsValid();
		
		ResultColumns = LoadStatement.GetColumnNames();
		
		
		for(FString Column : ResultColumns)
		{
			//We build our table/JSON/whatever here using
		}

		LoadStatement.ClearBindings();
		LoadStatement.Destroy();
		//TODO. Do some checks to avoid crash
		SQLiteDB->Close();  
		delete SQLiteDB;
		
		return ResultColumns;
	}
	UE_LOG(LogTemp, Warning, TEXT("Error while trying to access the DB"))

	return ResultColumns;
}