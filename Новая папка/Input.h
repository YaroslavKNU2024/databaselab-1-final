#pragma once
#include <stdio.h>
#include <string.h>
#include "Structures.h"
#include "Virus.h"

void readVirus(struct Virus* virus)
{
	char name[32];
	int status;

	name[0] = '\0';

	printf("Enter virus\'s name: ");
	scanf("%s", name);
	strcpy(virus->name, name);

	printf("Enter virus\'s status: ");
	scanf("%d", &status);
	virus->status = status;
	virus->variantsCount = 0;
}

void readVariant(struct Variant* variant)
{

	char origin[256];
	char name[64];
	char dateDiscovered[128];
	name[0] = dateDiscovered[0] = origin[0] = '\0';
	printf("Enter variant\'s name: ");
	scanf("%s", name);
	strcpy(variant->name, name);

	printf("Enter variant\'s origin: ");
	scanf("%s", origin);
	strcpy(variant->origin, origin);

	printf("Date discovered: ");
	scanf("%s", dateDiscovered);
	strcpy(variant->dateDiscovered, dateDiscovered);

}