/**
 * @brief derivative calculator implementation file
 */

#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define _EP_NODE_SHORT_OPERATORS
#include "ep.h"

/**
 * @brief is this node constant for differentiation checking function
 * 
 * @param[in] node node to check
 * @param[in] var  variable
 * 
 * @return true if node is constant, false if not
 */
static bool epNodeDerivativeIsConstant( const EpNode *node, const char *var ) {
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
} // epNodeDerivativeIsConstant

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

        case EP_BINARY_OPERATOR_MUL: {
            if (epNodeDerivativeIsConstant(lhs, var) || epNodeDerivativeIsConstant(rhs, var))
                return EP_MUL(epNodeDerivative(lhs, var), epNodeDerivative(rhs, var));
            else
                return EP_ADD(
                    EP_MUL(epNodeCopy(lhs), epNodeDerivative(rhs, var)),
                    EP_MUL(epNodeCopy(rhs), epNodeDerivative(lhs, var))
                );
        }

        case EP_BINARY_OPERATOR_DIV:
            if (epNodeDerivativeIsConstant(rhs, var))
                return EP_DIV(epNodeDerivative(lhs, var), epNodeCopy(rhs));
            else
                return EP_DIV(
                    EP_SUB(
                        EP_MUL(epNodeDerivative(lhs, var), epNodeCopy(rhs)),
                        EP_MUL(epNodeDerivative(rhs, var), epNodeCopy(lhs))
                    ),
                    EP_MUL(epNodeCopy(rhs), epNodeCopy(rhs))
                );

        case EP_BINARY_OPERATOR_POW: {
            bool lConst = epNodeDerivativeIsConstant(lhs, var);
            bool rConst = epNodeDerivativeIsConstant(rhs, var);

            if (!lConst && !rConst)
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

            if (lConst && !rConst)
                return EP_MUL(
                    EP_MUL(
                        epNodeDerivative(rhs, var),
                        EP_LN(epNodeCopy(lhs))
                    ),
                    EP_POW(
                        epNodeCopy(lhs),
                        epNodeCopy(rhs)
                    )
                );

            if (!lConst && rConst)
                return EP_MUL(
                    EP_MUL(
                        epNodeCopy(rhs),
                        epNodeDerivative(lhs, var)
                    ),
                    EP_POW(
                        epNodeCopy(lhs),
                        EP_SUB(epNodeCopy(rhs), EP_CONST(1.0))
                    )
                );

            if (lConst && rConst)
                return EP_POW(epNodeCopy(lhs), epNodeCopy(rhs));

            assert(false &&
                "All combinations of (bool, bool) pairs was checked in code above. "
                "Looks like one of functions somehow returned invalid boolean."
            );
            return NULL; // something defenetely got wrong
        }

        }
    }

    case EP_NODE_UNARY_OPERATOR: {
        const EpNode *op = node->unaryOperator.operand;

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_NEG:
            return EP_NEG(epNodeDerivative(op, var));

        case EP_UNARY_OPERATOR_SIN:
            return EP_MUL(epNodeDerivative(op, var), EP_COS(epNodeCopy(op)));

        case EP_UNARY_OPERATOR_COS:
            return EP_MUL(epNodeDerivative(op, var), EP_NEG(EP_SIN(epNodeCopy(op))));

        case EP_UNARY_OPERATOR_LN:
            return EP_DIV(epNodeDerivative(op, var), epNodeCopy(op));
        }
    }
    }

    // panic here?
} // epNodeDerivative

// ep_derivative.c
