#include <stdio.h>

int main()
{
    int a = 99;
    printf("a => Address: %p, value: %d\n", &a, a);
    
    int *p = &a;
    // or just do:
    // int *p;
    // p = &a;
    
    printf("p => Address: %p, Address pointed: %p, value: %d\n", &p, p, *p);
    
    // -----------------------
    
    int b = 100;
    int *p1 = &b;
    int **p2 = &p1;
    
    printf("b =>   %3d\n", b);
    printf("p1 =>  %3d\n", *p1);
    printf("p2 =>  %3d\n",**p2);
    
    // -----------------------

    return 0;
}
