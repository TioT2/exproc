/**
 * @brief expression processor main function
 */

#include <stdio.h>

#include "ep.h"

/**
 * @brief main project function
 * 
 * @return exit status
 */
int main( void ) {
    EpParseExpressionResult result = epParseExpression("-sin(xy^2)");

    if (result.status != EP_PARSE_EXPRESSION_OK)
        return 1;
    EpNode *root = result.ok.result;

    epPrintExpression(stdout, root);
    printf("\n");

    epNodeDtor(root);
    return 0;
} // main

// ep_main.c
