/**
 * @brief taylor series approximation implementation file
 */

#define _EP_NODE_SHORT_OPERATORS
#include "ep.h"

/**
 * @brief factorial calculation function
 * 
 * @param[in] number number to get factorial of
 * 
 * @return number factorial
 */
static unsigned long long epFactorial( unsigned long long number ) {
    unsigned long long result = 1;

    for (unsigned long long i = 2; i <= number; i++)
        result *= i;
    return result;
} // epFactorial

EpNode * epNodeTaylor(
    const EpNode * node,
    const char   * var,
    const EpNode * point,
    unsigned int   count
) {
    EpSubstitution varSubstitution = { .name = var, .node = point };

    EpNode *lhs = epNodeSubstitute(node, &varSubstitution, 1);
    EpNode *derivative = epNodeCopy(node);

    for (unsigned int i = 0; i < count; i++) {
        // calculate next derivative
        derivative = epNodeDerivative(derivative, var);

        // add next taylor series participant
        lhs = EP_ADD(
            lhs,
            EP_MUL(
                EP_DIV(
                    epNodeSubstitute(derivative, &varSubstitution, 1),
                    EP_CONST((double)epFactorial(i + 1))
                ),
                EP_POW(
                    EP_SUB(
                        epNodeVariable(var),
                        epNodeCopy(point)
                    ),
                    EP_CONST((double)(i + 1))
                )
            )
        );
    }

    return lhs;
} // epNodeTaylor

// ep_taylor.c
