#pragma once
#include <stdio.h>
#include <stdlib.h>
#include "Variants.h"

int checkFileExistence(FILE* indexTable, FILE* database, char* error)
{
	// DB files do not exist yet
	if (indexTable == NULL || database == NULL)
	{
		strcpy(error, "database files are not created yet");
		return 0;
	}

	return 1;
}

int checkIndexExistence(FILE* indexTable, char* error, int id)
{
	fseek(indexTable, 0, SEEK_END);

	long indexTableSize = ftell(indexTable);

	if (indexTableSize == 0 || id * sizeof(struct Indexer) > indexTableSize)
	{
		strcpy(error, "no such ID in table");
		return 0;
	}

	return 1;
}

int checkRecordExistence(struct Indexer indexer, char* error)
{
	// Record's been removed
	if (!indexer.exists)
	{
		strcpy(error, "the record you\'re looking for has been removed");
		return 0;
	}

	return 1;
}


int checkKeyPairUniqueness(struct Virus virus, int productId)
{
	FILE* variantsDb = fopen(VARIANT_DATA, "r+b");
	struct Variant variant;

	fseek(variantsDb, virus.firstVariantAddress, SEEK_SET);

	for (int i = 0; i < virus.variantsCount; i++)
	{
		fread(&variant, VARIANT_SIZE, 1, variantsDb);
		fclose(variantsDb);

		if (variant.variantId == productId)
		{
			return 0;
		}

		variantsDb = fopen(VARIANT_DATA, "r+b");
		fseek(variantsDb, variant.nextAddress, SEEK_SET);
	}

	fclose(variantsDb);

	return 1;
}

void info()
{
	FILE* indexTable = fopen("virus.ind", "rb");

	if (indexTable == NULL)
	{
		printf("Error: database files are not created yet\n");
		return;
	}

	int virusCount = 0;
	int variantCount = 0;

	fseek(indexTable, 0, SEEK_END);
	int indAmount = ftell(indexTable) / sizeof(struct Indexer);

	struct Virus virus;

	char dummy[64];

	for (int i = 1; i <= indAmount; i++)
	{
		if (getVirus(&virus, i, dummy) && virus.id > 0)
		{
			virusCount++;
			variantCount += virus.variantsCount;
			
			printf("virus #%d has %d variants(s)\n", i, virus.variantsCount);
		}
	}

	fclose(indexTable);

	printf("Total viruses: %d\n", virusCount);
	printf("Total variants: %d\n", variantCount);
}
