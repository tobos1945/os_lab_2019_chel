#include "swap.h"

void Swap(char *left, char *right)
{
	char temporary = *right;
	*right = *left;
	*left = temporary;
}
