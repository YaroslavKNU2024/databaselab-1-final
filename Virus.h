#pragma once
#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Checks.h"
#include "Variants.h"

#define VIRUS_IND "virus.ind"
#define VIRUS_DATA "virus.fl"
#define VIRUS_GARBAGE "virus_garbage.txt"
#define INDEXER_SIZE sizeof(struct Indexer)
#define VIRUS_SIZE sizeof(struct Virus)


void overwriteGarbageId(int garbageCount, FILE* garbageZone, struct Virus* record)
{
	int* delIds = (int*)malloc(garbageCount * sizeof(int));		// Виділяємо місце під список "сміттєвих" індексів

	for (int i = 0; i < garbageCount; i++)
		fscanf(garbageZone, "%d", delIds + i);				// Заповнюємо його

	record->id = delIds[0];									// Для запису замість логічно видаленого "сміттєвого"

	fclose(garbageZone);									// За допомогою цих двох команд
	fopen(VIRUS_GARBAGE, "wb");							// повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%d", garbageCount - 1);			// Записуємо нову кількість "сміттєвих" індексів

	for (int i = 1; i < garbageCount; i++)
		fprintf(garbageZone, " %d", delIds[i]);				// Записуємо решту "сміттєвих" індексів

	free(delIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

void noteDeletedVirus(int id)
{
	FILE* garbageZone = fopen(VIRUS_GARBAGE, "rb");		// "rb": відкриваємо бінарний файл для читання

	int garbageCount;
	fscanf(garbageZone, "%d", &garbageCount);

	int* delIds = (int*)malloc(garbageCount * sizeof(int));		// Виділяємо місце під список "сміттєвих" індексів

	for (int i = 0; i < garbageCount; i++)
		fscanf(garbageZone, "%d", delIds + i);				// Заповнюємо його

	fclose(garbageZone);									// За допомогою цих двох команд
	garbageZone = fopen(VIRUS_GARBAGE, "wb");				// повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%d", garbageCount + 1);			// Записуємо нову кількість "сміттєвих" індексів

	for (int i = 0; i < garbageCount; i++)
		fprintf(garbageZone, " %d", delIds[i]);				// Заносимо "сміттєві" індекси назад...

	fprintf(garbageZone, " %d", id);						// ...і дописуємо до них індекс щойно видаленого запису
	free(delIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

int insertVirus(struct Virus record)
{
	FILE* indexTable = fopen(VIRUS_IND, "a+b");			// "a+b": відкрити бінарний файл для запису в кінець та читання
	FILE* database = fopen(VIRUS_DATA, "a+b");	
	FILE* garbageZone = fopen(VIRUS_GARBAGE, "rb");		// "rb": відкрити бінарний файл лише для читання

	struct Indexer indexer;
	int garbageCount;

	fscanf(garbageZone, "%d", &garbageCount);

	// Якщо є "сміттєві" записи, перепишемо перший з них
	if (garbageCount)										
	{
		overwriteGarbageId(garbageCount, garbageZone, &record);

		fclose(indexTable);									// Закриваємо файли для зміни режиму доступу в подальшому
		fclose(database);

		indexTable = fopen(VIRUS_IND, "r+b");				// Знову відкриваємо і змінюємо режим на "читання та запис у довільне місце файлу"
		database = fopen(VIRUS_DATA, "r+b");

		fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET);
		fread(&indexer, INDEXER_SIZE, 1, indexTable);
		fseek(database, indexer.address, SEEK_SET);			// Ставимо курсор на "сміття" для подальшого перезапису	
	}
	// Якщо немає видалених записів
	else                                                   
	{
		long indexerSize = INDEXER_SIZE;

		fseek(indexTable, 0, SEEK_END);						// Ставимо курсор у кінець файлу таблички

		// Якщо розмір індексної таблички ненульовий (позиція від початку)
		if (ftell(indexTable))
		{
			fseek(indexTable, -indexerSize, SEEK_END);		// Ставимо курсор на останній індексатор
			fread(&indexer, INDEXER_SIZE, 1, indexTable);	// Читаємо останній індексатор

			record.id = indexer.id + 1;						// Нумеруємо запис наступним індексом
		}
		// Якщо індексна табличка порожня індексуємо наш запис як перший
		else
			record.id = 1;
	}

	record.firstVariantAddress = -1;
	record.variantsCount = 0;

	fwrite(&record, VIRUS_SIZE, 1, database);				// Записуємо в потрібне місце БД-таблички передану структуру

	indexer.id = record.id;									// Вносимо номер запису в індексатор
	indexer.address = (record.id - 1) * VIRUS_SIZE;		// Вносимо адресу запису в індексатор
	indexer.exists = 1;										// Змінна, яка вказує на існування запису

	printf("Your virus\'s id is %d\n", record.id);

	fseek(indexTable, (record.id - 1) * INDEXER_SIZE, SEEK_SET);
	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			// Записуємо індексатор у відповідну табличку, куди треба
	fclose(indexTable);
	fclose(database);

	return 1;
}

int getVirus(struct Virus* virus, int id, char* error)
{
	FILE* indexTable = fopen(VIRUS_IND, "rb");				// "rb": відкрити бінарний файл
	FILE* database = fopen(VIRUS_DATA, "rb");				// тільки для читання

	if (!checkFileExistence(indexTable, database, error))
		return 0;

	struct Indexer indexer;

	if (!checkIndexExistence(indexTable, error, id))
		return 0;

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// Отримуємо індексатор шуканого запису за вказаним номером
	fread(&indexer, INDEXER_SIZE, 1, indexTable);

	if (!checkRecordExistence(indexer, error))
		return 0;

	fseek(database, indexer.address, SEEK_SET);				// Отримуємо шуканий запис з БД-таблички  за знайденою адресою
	fread(virus, sizeof(struct Virus), 1, database);
	fclose(indexTable);
	fclose(database);

	return 1;
}

int updateVirus(struct Virus virus, char* error)
{
	FILE* indexTable = fopen(VIRUS_IND, "r+b");			// "r+b": відкрити бінарний файл для читання та запису
	FILE* database = fopen(VIRUS_DATA, "r+b");	

	if (!checkFileExistence(indexTable, database, error))
		return 0;

	struct Indexer indexer;
	int id = virus.id;

	if (!checkIndexExistence(indexTable, error, id))
		return 0;

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// Отримуємо індексатор шуканого запису
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			// за вказаним номером

	if (!checkRecordExistence(indexer, error))
		return 0;

	fseek(database, indexer.address, SEEK_SET);				// Позиціонуємося за адресою запису в БД
	fwrite(&virus, VIRUS_SIZE, 1, database);				// Оновлюємо запис
	fclose(indexTable);
	fclose(database);

	return 1;
}

int deleteVirus(int id, char* error)
{
	FILE* indexTable = fopen(VIRUS_IND, "r+b");			// "r+b": відкрити бінарний файл
															// для читання та запису	
	if (indexTable == NULL)
	{
		strcpy(error, "database files are not created yet");
		return 0;
	}

	if (!checkIndexExistence(indexTable, error, id))
		return 0;

	struct Virus virus;
	getVirus(&virus, id, error);

	struct Indexer indexer;

	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);	// Отримуємо індексатор шуканого запису
	fread(&indexer, INDEXER_SIZE, 1, indexTable);			// за вказаним номером

	indexer.exists = 0;										// Запис логічно не існуватиме...
	fseek(indexTable, (id - 1) * INDEXER_SIZE, SEEK_SET);

	fwrite(&indexer, INDEXER_SIZE, 1, indexTable);			// ...але фізично буде присутній
	fclose(indexTable);										// Закриваємо файл [NB: якщо не закрити, значення не оновиться]

	noteDeletedVirus(id);									// Заносимо індекс видаленого запису до "сміттєвої зони"


	if (virus.variantsCount)								// Були штами, видаляємо всі
	{
		FILE* variantsDb = fopen(VARIANT_DATA, "r+b");
		struct Variant variant;

		fseek(variantsDb, virus.firstVariantAddress, SEEK_SET);

		for (int i = 0; i < virus.variantsCount; i++)
		{
			fread(&variant, VARIANT_SIZE, 1, variantsDb);
			fclose(variantsDb);
			deleteVariant(virus, variant, variant.variantId, error);

			variantsDb = fopen(VARIANT_DATA, "r+b");
			fseek(variantsDb, variant.nextAddress, SEEK_SET);
		}

		fclose(variantsDb);
	}
	return 1;
}