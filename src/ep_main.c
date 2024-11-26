/**
 * @brief expression processor main function
 */

#include <math.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "ep.h"

/**
 * @brief main project function
 * 
 * @return exit status
 */
int main( void ) {
    EpNode *root = NULL;
    {
        EpParseExpressionResult result = epParseExpression("-(sin(xy^2.0) / t)");
        if (result.status != EP_PARSE_EXPRESSION_OK) {
            printf("Expression parsing failed.\n");
            return 1;
        }

        root = result.ok.result;
    }

    printf("function: ");
    epNodeDump(stdout, root, EP_DUMP_INFIX_EXPRESSION);
    printf("\n");

    EpNode *rootDerivative = epNodeDerivative(root, "xy");
    assert(rootDerivative != NULL);

    printf("derivative: ");
    epNodeDump(stdout, rootDerivative, EP_DUMP_INFIX_EXPRESSION);
    printf("\n");

    EpNode *optimizedRootDerivative = epNodeOptimize(rootDerivative);

    printf("optimized derivative: ");
    epNodeDump(stdout, optimizedRootDerivative, EP_DUMP_TEX);
    printf("\n");

    epNodeDtor(optimizedRootDerivative);
    epNodeDtor(rootDerivative);

    {
        EpSubstitution substs[] = {
            {"xy", epNodeConstant(sqrt(3.14159265 / 2.0))},
        };
        EpNode *result = epNodeSubstitute(root, substs, 1);
        assert(result != NULL);

        printf("substituted function: ");
        epNodeDump(stdout, result, EP_DUMP_INFIX_EXPRESSION);
        printf("\n");

        EpNode *optResult = epNodeOptimize(result);
        assert(optResult != NULL);

        if (optResult->type == EP_NODE_CONSTANT) {
            printf("substitution optimization result: %lf\n", optResult->constant);
        } else {
            printf("substitution does not evaluate to a constant: ");
            epNodeDump(stdout, optResult, EP_DUMP_INFIX_EXPRESSION);
            printf("\n");
        }

        epNodeDtor((EpNode *)substs[0].node);
        epNodeDtor(result);
        epNodeDtor(optResult);
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
