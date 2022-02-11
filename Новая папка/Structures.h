#pragma once

struct Indexer
{
	int id;
	int address;
	int exists;
};

struct Virus
{
	int id;
	char name[64];
	char date[128];
	int status;
	long firstVariantAddress;
	int variantsCount;
};

struct Variant
{
	int virusId;
	int variantId;
	char origin[256];
	char name[64];
	char dateDiscovered[128];
	int exists;
	long selfAddress;
	long nextAddress;
};
