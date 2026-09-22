#include "revert_string.h"

void RevertString(char *str)
{
	char *left = str;
	char *right = str;
	while (*right != '\0'){
		right++;
	}
	right--;

	while (left < right) {
		char tmp = *left;
		*left = *right;
		*right = tmp;

		left++;
		right--;
	}
}

