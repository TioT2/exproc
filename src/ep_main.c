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
    EpParseExpressionResult result = epParseExpression("((x + 3) / (1000 - 7))");

    if (result.status != EP_PARSE_EXPRESSION_OK)
        return 1;
    EpNode *root = result.ok.result;

    epNodeDtor(root);

    int tridcat = 30;
    printf("CGSG FOREVER!!! %d!!!", tridcat);
    return 0;
} // main

// ep_main.c
