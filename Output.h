#pragma once
#include <stdio.h>
#include "Virus.h"
#include "Structures.h"

void printVirus(struct Virus virus)
{
	printf("Virus\'s id: %d\n", virus.id);
	printf("Virus\'s name: %s\n", virus.name);
	printf("Number of variants: %d\n\n", virus.variantsCount);
}

void printVariant(struct Variant variant, struct Virus virus)
{
	printf("\tVariant Id: %d\n", variant.variantId);
	printf("\tVariant name: %s\n", variant.name);
	printf("\tOrigin: %s\n", variant.origin);
	printf("\tDiscovered: %s\n", variant.dateDiscovered);
}