#pragma once
#pragma warning(disable:4996)

#include <stdio.h>
#include <stdlib.h>
#include "Structures.h"
#include "Checks.h"
#include "Virus.h"

#define VARIANT_DATA "variant.fl"
#define VARIANT_GARBAGE "variant_garbage.txt"
#define VARIANT_SIZE sizeof(struct Variant)

void reopenDatabase(FILE* database)
{
	fclose(database);
	database = fopen(VARIANT_DATA, "r+b");
}

void linkAddresses(FILE* database, struct Virus virus, struct Variant variant)
{
	reopenDatabase(database);								// Змінюємо режим на "читання з та запис у будь-яке місце"

	struct Variant previous;

	fseek(database, virus.firstVariantAddress, SEEK_SET);

	for (int i = 0; i < virus.variantsCount; i++)		    // Пробігаємомо зв'язаний список до останньої поставки
	{
		fread(&previous, VARIANT_SIZE, 1, database);
		fseek(database, previous.nextAddress, SEEK_SET);
	}

	previous.nextAddress = variant.selfAddress;				// Зв'язуємо адреси
	fwrite(&previous, VARIANT_SIZE, 1, database);				// Заносимо оновлений запис назад до файлу
}

void relinkAddresses(FILE* database, struct Variant previous, struct Variant variant, struct Virus* virus)
{
	// Якщо нема попередника (перший запис)...
	if (variant.selfAddress == virus->firstVariantAddress)		
	{
		if (variant.selfAddress == variant.nextAddress)			// та ще й немає наступника (запис лише один)
			virus->firstVariantAddress = -1;					// Неможлива адреса для безпеки
		else                                                // та наступник є,
			virus->firstVariantAddress = variant.nextAddress;  // робимо його першим
	}
	// Якщо попередник є...
	else              
	{
		if (variant.selfAddress == variant.nextAddress)			// але немає наступника (останній запис)
		{
			previous.nextAddress = previous.selfAddress;    // Робимо попередник останнім
		}
		else                                                // ... а разом з ним і наступник
		{
			previous.nextAddress = variant.nextAddress;		// Робимо наступник наступником попередника
		}

		fseek(database, previous.selfAddress, SEEK_SET);	// Записуємо оновлений попередник
		fwrite(&previous, VARIANT_SIZE, 1, database);			// назад до таблички
	}
}



void overwriteGarbageAddress(int garbageCount, FILE* garbageZone, struct Variant* record)
{
	long* delIds = (long*)malloc(garbageCount * sizeof(long));		// Виділяємо місце під список "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
		fscanf(garbageZone, "%d", delIds + i);				// Заповнюємо його

	record->selfAddress = delIds[0];						// Для запису замість логічно видаленої "сміттєвої"
	record->nextAddress = delIds[0];

	fclose(garbageZone);									// За допомогою цих двох команд
	fopen(VARIANT_GARBAGE, "wb");							    // повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%d", garbageCount - 1);			// Записуємо нову кількість "сміттєвих" адрес

	for (int i = 1; i < garbageCount; i++)
		fprintf(garbageZone, " %d", delIds[i]);				// Записуємо решту "сміттєвих" адрес

	free(delIds);											// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

int insertVariant(struct Virus virus, struct Variant variant, char* error)
{
	variant.exists = 1;

	FILE* database = fopen(VARIANT_DATA, "a+b");
	FILE* garbageZone = fopen(VARIANT_GARBAGE, "rb");

	int garbageCount;

	fscanf(garbageZone, "%d", &garbageCount);

	if (garbageCount)											// Наявні видалені записи
	{
		overwriteGarbageAddress(garbageCount, garbageZone, &variant);
		reopenDatabase(database);								// Змінюємо режим доступу файлу
		fseek(database, variant.selfAddress, SEEK_SET);			// Ставимо курсор на "сміття" для подальшого перезапису
	}
	else                                                        // Видалених немає, пишемо в кінець файлу
	{
		fseek(database, 0, SEEK_END);

		int dbSize = ftell(database);

		variant.selfAddress = dbSize;
		variant.nextAddress = dbSize;
	}

	fwrite(&variant, VARIANT_SIZE, 1, database);					// Записуємо штам до свого файлу

	if (!virus.variantsCount)								    // Штамів ще немає, пишемо адресу першої
		virus.firstVariantAddress = variant.selfAddress;
	else                                                        // штами вже є, оновлюємо "адресу наступника" останньої
		linkAddresses(database, virus, variant);

	fclose(database);											// Закриваємо файл

	virus.variantsCount++;										// Стало на один штам більше
	updateVirus(virus, error);								// Оновлюємо запис постачальника

	return 1;
}

void noteDeletedvariant(long address)
{
	FILE* garbageZone = fopen(VARIANT_GARBAGE, "rb");			// "rb": відкриваємо бінарний файл для читання

	int garbageCount;
	fscanf(garbageZone, "%d", &garbageCount);

	long* delAddresses = (long*)malloc(garbageCount * sizeof(long)); // Виділяємо місце під список "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
		fscanf(garbageZone, "%ld", delAddresses + i);		// Заповнюємо його

	fclose(garbageZone);									// За допомогою цих двох команд
	garbageZone = fopen(VARIANT_GARBAGE, "wb");				// повністю очищуємо файл зі "сміттям"
	fprintf(garbageZone, "%ld", garbageCount + 1);			// Записуємо нову кількість "сміттєвих" адрес

	for (int i = 0; i < garbageCount; i++)
		fprintf(garbageZone, " %ld", delAddresses[i]);		// Заносимо "сміттєві" адреси назад...

	fprintf(garbageZone, " %d", address);					// ...і дописуємо до них адресу щойно видаленого запису
	free(delAddresses);										// Звільняємо виділену під масив пам'ять
	fclose(garbageZone);									// Закриваємо файл
}

int getVariant(struct Virus virus, struct Variant* variant, int variantId, char* error)
{
	if (!virus.variantsCount)									// У віруса нема штаму
	{
		strcpy(error, "This virus has no variants yet");
		return 0;
	}

	FILE* database = fopen(VARIANT_DATA, "rb");


	fseek(database, virus.firstVariantAddress, SEEK_SET);		// Отримуємо перший запис
	fread(variant, VARIANT_SIZE, 1, database);

	for (int i = 0; i < virus.variantsCount; i++)				// Шукаємо потрібний запис по коду штаму
	{
		// Якщо знайшли
		if (variant->variantId == variantId)						
		{
			fclose(database);
			return 1;
		}

		fseek(database, variant->nextAddress, SEEK_SET);
		fread(variant, VARIANT_SIZE, 1, database);
	}

	// Якщо не знайшли
	strcpy(error, "No such variant in database");					
	fclose(database);
	return 0;
}

// На вхід подається штам з оновленими значеннями, яку треба записати у файл
int updateVariant(struct Variant variant, int variantId)
{
	FILE* database = fopen(VARIANT_DATA, "r+b");

	fseek(database, variant.selfAddress, SEEK_SET);
	fwrite(&variant, VARIANT_SIZE, 1, database);
	fclose(database);

	return 1;
}

int deleteVariant(struct Virus virus, struct Variant variant, int variantId, char* error)
{
	FILE* database = fopen(VARIANT_DATA, "r+b");
	struct Variant previous;

	fseek(database, virus.firstVariantAddress, SEEK_SET);

	do		                                                    // Шукаємо попередника запису (його може й не бути,
	{															// тоді в попередника занесеться сам запис)
		fread(&previous, VARIANT_SIZE, 1, database);
		fseek(database, previous.nextAddress, SEEK_SET);
	} while (previous.nextAddress != variant.selfAddress && variant.selfAddress != virus.firstVariantAddress);

	relinkAddresses(database, previous, variant, &virus);
	noteDeletedvariant(variant.selfAddress);						// Заносимо адресу видаленого запису у "смітник"

	variant.exists = 0;											// Логічно не існуватиме

	fseek(database, variant.selfAddress, SEEK_SET);				// ...але фізично
	fwrite(&variant, VARIANT_SIZE, 1, database);					// записуємо назад
	fclose(database);

	virus.variantsCount--;										// На один штам меньше
	updateVirus(virus, error);

}