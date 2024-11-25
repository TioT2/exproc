/**
 * @brief derivative calculator implementation file
 */

#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define _EP_NODE_SHORT_OPERATORS
#include "ep.h"

bool epNodeDerivativeIsConstant( const EpNode *node, const char *var ) {
    assert(node != NULL);
    assert(var != NULL);

    switch (node->type) {
    case EP_NODE_VARIABLE:
        return strcmp(node->variable, var) != 0;

    case EP_NODE_CONSTANT:
        return true;

    case EP_NODE_BINARY_OPERATOR:
        return true
            && epNodeDerivativeIsConstant(node->binaryOperator.lhs, var)
            && epNodeDerivativeIsConstant(node->binaryOperator.rhs, var)
        ;

    case EP_NODE_UNARY_OPERATOR:
        return epNodeDerivativeIsConstant(node->unaryOperator.operand, var);
    }
}


EpNode * epNodeDerivative( const EpNode *node, const char *var ) {
    assert(var != NULL);

    if (node == NULL)
        return NULL;

    switch (node->type) {
    case EP_NODE_VARIABLE:
        return epNodeConstant(
            strcmp(node->variable, var) == 0
                ? 1.0
                : 0.0
        );

    case EP_NODE_CONSTANT:
        return epNodeConstant(0.0);

    case EP_NODE_BINARY_OPERATOR: {
        const EpNode *lhs = node->binaryOperator.lhs;
        const EpNode *rhs = node->binaryOperator.rhs;

        switch (node->binaryOperator.op) {
        case EP_BINARY_OPERATOR_ADD:
            return EP_ADD(
                epNodeDerivative(lhs, var),
                epNodeDerivative(rhs, var)
            );

        case EP_BINARY_OPERATOR_SUB:
            return EP_SUB(
                epNodeDerivative(lhs, var),
                epNodeDerivative(rhs, var)
            );

        case EP_BINARY_OPERATOR_MUL:
            return EP_ADD(
                EP_MUL(epNodeCopy(lhs), epNodeDerivative(rhs, var)),
                EP_MUL(epNodeCopy(rhs), epNodeDerivative(lhs, var))
            );

        case EP_BINARY_OPERATOR_DIV:
            return EP_DIV(
                EP_SUB(
                    EP_MUL(epNodeDerivative(lhs, var), epNodeCopy(rhs)),
                    EP_MUL(epNodeDerivative(rhs, var), epNodeCopy(lhs))
                ),
                EP_MUL(epNodeCopy(rhs), epNodeCopy(rhs))
            );

        case EP_BINARY_OPERATOR_POW:
            return EP_MUL(
                EP_POW(epNodeCopy(lhs), epNodeCopy(rhs)),
                EP_ADD(
                    EP_MUL(epNodeDerivative(rhs, var), EP_LN(epNodeCopy(lhs))),
                    EP_MUL(
                        EP_DIV(epNodeDerivative(lhs, var), epNodeCopy(lhs)),
                        epNodeCopy(rhs)
                    )
                )
            );

        }
        break;
    }

    case EP_NODE_UNARY_OPERATOR: {
        const EpNode *op = node->unaryOperator.operand;

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_NEG:
            return EP_NEG(epNodeDerivative(op, var));

        case EP_UNARY_OPERATOR_SIN:
            return EP_MUL(epNodeCopy(op), EP_COS(epNodeDerivative(op, var)));

        case EP_UNARY_OPERATOR_COS:
            return EP_MUL(epNodeCopy(op), EP_NEG(EP_SIN(epNodeDerivative(op, var))));

        case EP_UNARY_OPERATOR_LN:
            return EP_DIV(epNodeCopy(op), epNodeDerivative(op, var));
        }
    }


        // panic in default?

    }
} // epNodeDerivative

// ep_derivative.c
