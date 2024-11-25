/**
 * @brief expression processor main function
 */

#include <stdio.h>
#include <math.h>

#include "ep.h"

/**
 * @brief main project function
 * 
 * @return exit status
 */
int main( void ) {
    EpNode *root = NULL;
    {
        EpParseExpressionResult result = epParseExpression("-sin(xy^20.0e-1)");
        if (result.status != EP_PARSE_EXPRESSION_OK) {
            printf("Expression parsing failed.\n");
            return 1;
        }

        root = result.ok.result;
    }

    printf("function: ");
    epPrintExpression(stdout, root);
    printf("\n");

    EpNode *rootDerivative = epNodeDerivative(root, "xy");
    if (rootDerivative != NULL) {
        printf("derivative: ");
        epPrintExpression(stdout, rootDerivative);
        printf("\n");
        epNodeDtor(rootDerivative);
    } else {
        printf("Derivation failed");
    }

    {
        EpVariable varTable[] = {
            {"xy", sqrt(3.14159265 / 2.0)},
        };
        EpNodeComputeResult result = epNodeCompute(root, varTable, 1);
        if (result.status == EP_NODE_COMPUTE_OK)
            printf("result: %lf\n", result.ok);

    }

    epNodeDtor(root);
    return 0;
} // main

// ep_main.c
