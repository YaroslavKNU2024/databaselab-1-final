#include <stdio.h>
#include "Structures.h"
#include "Virus.h"
#include "Variants.h"
#include "Input.h"
#include "Output.h"


int main()
{
	struct Virus virus;
	struct Variant variant;

	while (1)
	{
		int id;
		int key;
		char error[64];

		printf("Choose option:\n0 - Quit\n1 - Insert virus\n2 - Get virus\n3 - Update virus\n4 - Delete virus\n5 - Insert variant\n6 - Get variant\n7 - Update variant\n8 - Delete variant\n9 - Info\n");
		scanf("%d", &key);

		switch (key) {
		case 0:
			return 0;
		case 1:
			readVirus(&virus);
			insertVirus(virus);
			break;

		case 2:
			printf("Enter ID: ");
			scanf("%d", &id);
			getVirus(&virus, id, error) ? printVirus(virus) : printf("Error: %s\n", error);
			break;

		case 3:
			printf("Enter ID: ");
			scanf("%d", &id);
			virus.id = id;

			readVirus(&virus);
			updateVirus(virus, error) ? printf("Updated successfully\n") : printf("Error: %s\n", error);
			break;

		case 4:
			printf("Enter ID: ");
			scanf("%d", &id);
			deleteVirus(id, error) ? printf("Deleted successfully\n") : printf("Error: %s\n", error);
			break;

		case 5:
			printf("Enter virus\'s ID: ");
			scanf("%d", &id);
			if (getVirus(&virus, id, error))
			{
				variant.virusId = id;
				printf("Enter variant ID: ");
				scanf("%d", &id);
				variant.variantId = id;

				readVariant(&variant);
				insertVariant(virus, variant, error);
				printf("Inserted successfully. To access, use virus\'s and variant\'s IDs\n");
			}
			else
				printf("Error: %s\n", error);
			break;

		case 6:
			printf("Enter virus\'s ID: ");
			scanf("%d", &id);

			if (getVirus(&virus, id, error))
			{
				printf("Enter variant ID: ");
				scanf("%d", &id);
				getVariant(virus, &variant, id, error) ? printVariant(variant, virus) : printf("Error: %s\n", error);
			}
			else
				printf("Error: %s\n", error);
			break;

		case 7:
			printf("Enter virus\'s ID: ");
			scanf("%d", &id);

			if (getVirus(&virus, id, error))
			{
				printf("Enter variant ID: ");
				scanf("%d", &id);

				if (getVariant(virus, &variant, id, error))
				{
					readVariant(&variant);
					updateVariant(variant, id, error);
					printf("Updated successfully\n");
				}
				else
					printf("Error: %s\n", error);
			}
			else
				printf("Error: %s\n", error);
			break;

		case 8:
			printf("Enter Virus\'s ID: ");
			scanf("%d", &id);

			if (getVirus(&virus, id, error))
			{
				printf("Enter variant ID: ");
				scanf("%d", &id);

				if (getVariant(virus, &variant, id, error))
				{
					deleteVariant(virus, variant, id, error);
					printf("Deleted successfully\n");
				}
				else
					printf("Error: %s\n", error);
			}
			else
				printf("Error: %s\n", error);
			break;

		case 9:
			info();
			break;

		default:
			printf("Invalid input, please try again\n");
		}

		printf("---------\n");
	}

	return 0;
}
