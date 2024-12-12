/**
 * @brief derivative calculator implementation file
 */

#include <string.h>
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
    // custom syntax highlight?
#define _DL_ (epNodeDerivative(lhs, var))
#define _DR_ (epNodeDerivative(lhs, var))
#define _L_ (epNodeCopy(lhs))
#define _R_ (epNodeCopy(rhs))

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
            return EP_ADD(_DL_, _DR_);

        case EP_BINARY_OPERATOR_SUB:
            return EP_SUB(_DL_, _DR_);

        case EP_BINARY_OPERATOR_MUL: {
            if (epNodeDerivativeIsConstant(lhs, var))
                return EP_MUL(_L_, _DR_);
            else if (epNodeDerivativeIsConstant(rhs, var))
                return EP_MUL(_R_, _DL_);
            else
                return EP_ADD(
                    EP_MUL(_L_, _DR_),
                    EP_MUL(_R_, _DL_)
                );
        }

        case EP_BINARY_OPERATOR_DIV:
            if (epNodeDerivativeIsConstant(rhs, var))
                return EP_DIV(_DL_, _R_);
            else
                return EP_DIV(
                    EP_SUB(
                        EP_MUL(_DL_, _R_),
                        EP_MUL(_DR_, _L_)
                    ),
                    EP_MUL(_R_, _R_)
                );

        case EP_BINARY_OPERATOR_POW: {
            bool lConst = epNodeDerivativeIsConstant(lhs, var);
            bool rConst = epNodeDerivativeIsConstant(rhs, var);

            if (!lConst && !rConst) {
                return EP_MUL(
                    EP_POW(_L_, _R_),
                    EP_ADD(
                        EP_MUL(_DR_, EP_LN(_L_)),
                        EP_MUL(EP_DIV(_DL_, _L_), _R_)
                    )
                );
            }

            if (lConst && !rConst)
                return EP_MUL(
                    EP_MUL(_DR_, EP_LN(_L_)),
                    EP_POW(_L_, _R_)
                );

            if (!lConst && rConst)
                return EP_MUL(
                    EP_MUL(_R_, _DL_),
                    EP_POW(_L_, EP_SUB(_R_, EP_CONST(1.0))
                    )
                );

            if (lConst && rConst)
                return EP_POW(_L_, _R_);

            assert(false &&
                "All combinations of (bool, bool) pairs was checked in code above. "
                "Looks like one of functions somehow returned invalid boolean."
            );
            return NULL; // something defenetely got wrong
        }

        }
    }

    case EP_NODE_UNARY_OPERATOR: {
        const EpNode *operand = node->unaryOperator.operand;
        EpNode *derivative = epNodeDerivative(operand, var);

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_NEG:
            return EP_NEG(derivative);

        case EP_UNARY_OPERATOR_LN:
            return EP_DIV(derivative, epNodeCopy(operand));

        case EP_UNARY_OPERATOR_SIN:
            return EP_MUL(derivative, EP_COS(epNodeCopy(operand)));

        case EP_UNARY_OPERATOR_COS:
            return EP_MUL(derivative, EP_NEG(EP_SIN(epNodeCopy(operand))));

        case EP_UNARY_OPERATOR_TAN:
            return EP_DIV(derivative, EP_POW(EP_COS(epNodeCopy(operand)), EP_CONST(2.0)));

        case EP_UNARY_OPERATOR_COT:
            return EP_DIV(EP_NEG(derivative), EP_POW(EP_SIN(epNodeCopy(operand)), EP_CONST(2.0)));

        case EP_UNARY_OPERATOR_ASIN:
            return EP_DIV(
                derivative,
                EP_POW(
                    EP_SUB(EP_CONST(1.0), EP_POW(epNodeCopy(operand), EP_CONST(2.0))),
                    EP_CONST(0.5)
                )
            );

        case EP_UNARY_OPERATOR_ACOS:
            return EP_DIV(
                EP_NEG(derivative),
                EP_POW(
                    EP_SUB(EP_CONST(1.0), EP_POW(epNodeCopy(operand), EP_CONST(2.0))),
                    EP_CONST(0.5)
                )
            );

        case EP_UNARY_OPERATOR_ATAN:
            return EP_DIV(
                derivative,
                EP_ADD(
                    EP_CONST(1.0),
                    EP_POW(epNodeCopy(operand), EP_CONST(2.0))
                )
            );

        case EP_UNARY_OPERATOR_ACOT:
            return EP_DIV(
                EP_NEG(derivative),
                EP_ADD(
                    EP_CONST(1.0),
                    EP_POW(epNodeCopy(operand), EP_CONST(2.0))
                )
            );
        }
    }
    }

    // panic here?
#undef _DL_
#undef _DR_
#undef _L_
#undef _R_
} // epNodeDerivative

// ep_derivative.c
