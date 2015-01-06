#include <stdio.h>
#include <stdlib.h>
#include <time.h>

extern void gikoMain(void);

void print(int x)
{
  printf("%d\n", x);
}

int scan(void)
{
  int x;

  printf("? ");
  scanf("%d", &x);

  return x;
}

int main(void)
{
  srand(time(NULL));
  gikoMain();

  return 0;
}
