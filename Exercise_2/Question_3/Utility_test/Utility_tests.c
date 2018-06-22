#include <math.h>
#include <stdio.h>

#define TRUE 1
#define FALSE 0
#define U32_T unsigned int

// U=0.7333
U32_T ex0_period[] = {2, 10, 15,0};
U32_T ex0_wcet[] = {1, 1, 2};

// U=0.9857
U32_T ex1_period[] = {2, 5, 7,0};
U32_T ex1_wcet[] = {1, 1, 2};

// U=0.9967
U32_T ex2_period[] = {2, 5, 7, 13,0};
U32_T ex2_wcet[] = {1, 1, 1, 2};

// U=0.93
U32_T ex3_period[] = {3, 5, 15,0};
U32_T ex3_wcet[] = {1, 2, 3};

// U=1.0
U32_T ex4_period[] = {2, 4, 16,0};
U32_T ex4_wcet[] = {1, 1, 4};

U32_T ex5_period[] = {2, 5, 10,0};
U32_T ex5_wcet[] = {1, 2, 1};

U32_T ex6_period[] = {2, 5, 7, 13,0};
U32_T ex6_wcet[] = {1, 1, 1, 2};

U32_T ex7_period[] = {3, 5, 15,0};
U32_T ex7_wcet[] = {1, 2, 4};

U32_T ex8_period[] = {2, 5, 7, 13,0};
U32_T ex8_wcet[] = {1, 2, 1, 2};

U32_T ex9_period[] = {6, 8, 12, 24,0};
U32_T ex9_wcet[] = {1, 2, 2, 6};


int utility_test(U32_T period[], U32_T wcet[])
{
	int i=0, j; 
	float utility=0,temp_period=0,temp_wcet=0;
	U32_T an, anext;
	int set_feasible=TRUE;
	while((period[i]))
	{
		temp_period = period[i];
		temp_wcet = wcet[i];
		utility += temp_wcet/temp_period;	
		i++; 
	}
	printf(" Utility = %f \t",utility);
	if (utility > 1)
       	{
        	set_feasible=FALSE;
       	}
  	return set_feasible;
}


int main(void)
{ 
    int i;
	U32_T numServices;
    
    printf("******** EDF & LLF Feasibility Test\nIf Utility > 1.00, LLF and EDF are feasilble\n");
   
    printf("Ex-0 (C1=1, C2=1, C3=2; T1=2, T2=10, T3=15; T=D): \t\t");
	numServices = sizeof(ex0_period)/sizeof(U32_T);
    if(utility_test(ex0_period, ex0_wcet) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    printf("Ex-1 (C1=1, C2=1, C3=2; T1=2, T2=5, T3=7; T=D): \t\t");
	numServices = sizeof(ex1_period)/sizeof(U32_T);
    if(utility_test(ex1_period, ex1_wcet) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");
	
    printf("Ex-2 (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): \t");
	numServices = sizeof(ex2_period)/sizeof(U32_T);
    if(utility_test(ex2_period, ex2_wcet) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    printf("Ex-3 (C1=1, C2=2, C3=3; T1=3, T2=5, T3=15; T=D): \t\t");
	numServices = sizeof(ex3_period)/sizeof(U32_T);
    if(utility_test(ex3_period, ex3_wcet) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");
	
    printf("Ex-4 (C1=1, C2=1, C3=4; T1=2, T2=4, T3=16; T=D): \t\t");
	numServices = sizeof(ex4_period)/sizeof(U32_T);
    if(utility_test(ex4_period, ex4_wcet) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");
	
    printf("Ex-5 (C1=1, C2=2, C3=1; T1=2, T2=5, T3=10; T=D): \t\t");
	numServices = sizeof(ex5_period)/sizeof(U32_T);
    if(utility_test(ex5_period, ex5_wcet) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

	
    printf("Ex-6 (C1=1, C2=1, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): \t");
	numServices = sizeof(ex6_period)/sizeof(U32_T);
    if(utility_test(ex6_period, ex6_wcet) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

	
    printf("Ex-7 (C1=1, C2=2, C3=4; T1=3, T2=5, T3=15; T=D): \t\t");
	numServices = sizeof(ex7_period)/sizeof(U32_T);
    if(utility_test(ex7_period, ex7_wcet) == TRUE)
 
        printf("FEASIBLE\n");

    else
        printf("INFEASIBLE\n");

	
    printf("Ex-8 (C1=1, C2=2, C3=1, C4=2; T1=2, T2=5, T3=7, T4=13; T=D): \t");
	numServices = sizeof(ex8_period)/sizeof(U32_T);
    if(utility_test(ex8_period, ex8_wcet) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

    printf("Ex-9 (C1=1, C2=2, C3=2, C4=6; T1=6, T2=8, T3=12, T4=24; T=D): \t");
	numServices = sizeof(ex9_period)/sizeof(U32_T);
    if(utility_test(ex9_period, ex9_wcet) == TRUE)
        printf("FEASIBLE\n");
    else
        printf("INFEASIBLE\n");

}

