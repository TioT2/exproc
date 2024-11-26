/**
 * @brief substitution function
 */

#include <string.h>

#include "ep.h"

EpNode * epNodeSubstitute(
    const EpNode         * node,
    const EpSubstitution * substitutions,
    size_t                 substitutionCount
) {
    // yeah
    if (substitutionCount == 0)
        return epNodeCopy(node);

    switch (node->type) {
    case EP_NODE_VARIABLE:
        for (size_t i = 0; i < substitutionCount; i++)
            if (strcmp(node->variable, substitutions[i].name) == 0)
                return epNodeCopy(substitutions[i].node);
        return epNodeCopy(node);

    case EP_NODE_CONSTANT:
        return epNodeCopy(node);

    case EP_NODE_BINARY_OPERATOR:
        return epNodeBinaryOperator(
            node->binaryOperator.op,
            epNodeSubstitute(node->binaryOperator.lhs, substitutions, substitutionCount),
            epNodeSubstitute(node->binaryOperator.rhs, substitutions, substitutionCount)
        );

    case EP_NODE_UNARY_OPERATOR:
        return epNodeUnaryOperator(
            node->unaryOperator.op,
            epNodeSubstitute(node->unaryOperator.operand, substitutions, substitutionCount)
        );
    }
} // epNodeSubstitute

// ep_substitute.c
