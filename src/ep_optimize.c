/**
 * @brief optimizer implementation file
 */

#include <assert.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#define _EP_NODE_SHORT_OPERATORS
#include "ep.h"

/**
 * @brief raising to a power optimization function
 * 
 * @param[in] lhs left operand (nullable)
 * @param[in] rhs right operand (nullable)
 * 
 * @note operands assumed to be already optimal
 * 
 * @return created node
 */
static EpNode * epOptimizedPow( EpNode *lhs, EpNode *rhs ) {
    if (lhs == NULL || rhs == NULL) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return NULL;
    }

    // check for lhs being neutral element
    if (lhs->type == EP_NODE_CONSTANT && epDoubleIsSame(lhs->constant, 1.0)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(1.0);
    }

    // check for rhs being neutral element
    if (rhs->type == EP_NODE_CONSTANT && epDoubleIsSame(rhs->constant, 0.0)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(1.0);
    }

    // check for rhs being neutral element
    if (rhs->type == EP_NODE_CONSTANT && epDoubleIsSame(rhs->constant, 1.0)) {
        epNodeDtor(rhs);
        return lhs;
    }

    // check for rhs being constants
    if (lhs->type == EP_NODE_CONSTANT && rhs->type == EP_NODE_CONSTANT) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(pow(lhs->constant, rhs->constant));
    }

    return EP_POW(lhs, rhs);
} // epOptimizedPow

/**
 * @brief multiplication optimization function
 * 
 * @param[in] lhs left operand (nullable)
 * @param[in] rhs right operand (nullable)
 * 
 * @note operands assumed to be already optimal
 * 
 * @return created node
 */
static EpNode * epOptimizedMul( EpNode *lhs, EpNode *rhs ) {
    if (lhs == NULL || rhs == NULL) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return NULL;
    }

    // check for lhs being neutral element
    if (lhs->type == EP_NODE_CONSTANT && epDoubleIsSame(lhs->constant, 1.0)) {
        epNodeDtor(lhs);
        return rhs;
    }

    // check for rhs being neutral element
    if (rhs->type == EP_NODE_CONSTANT && epDoubleIsSame(rhs->constant, 1.0)) {
        epNodeDtor(rhs);
        return lhs;
    }

    // check for lhs being neutral element
    if (lhs->type == EP_NODE_CONSTANT && epDoubleIsSame(lhs->constant, 0.0)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(0.0);
    }

    // check for rhs being neutral element
    if (rhs->type == EP_NODE_CONSTANT && epDoubleIsSame(rhs->constant, 0.0)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(0.0);
    }

    // check for rhs being constants
    if (lhs->type == EP_NODE_CONSTANT && rhs->type == EP_NODE_CONSTANT) {
        EpNode *result = EP_CONST(lhs->constant * rhs->constant);

        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return result;
    }

    if (epNodeIsSame(lhs, rhs)) {
        epNodeDtor(rhs);
        return EP_POW(lhs, EP_CONST(2.0));
    }

    return EP_MUL(lhs, rhs);
} // epOptimizedMul

/**
 * @brief division optimization function
 * 
 * @param[in] lhs left operand (nullable)
 * @param[in] rhs right operand (nullable)
 * 
 * @note operands assumed to be already optimal
 * 
 * @return created node
 */
static EpNode * epOptimizedDiv( EpNode *lhs, EpNode *rhs ) {
    if (lhs == NULL || rhs == NULL) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return NULL;
    }

    // check for rhs being zero
    if (lhs->type == EP_NODE_CONSTANT && epDoubleIsSame(lhs->constant, 0.0)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(0.0);
    }

    // check for rhs being neutral element
    if (rhs->type == EP_NODE_CONSTANT && epDoubleIsSame(rhs->constant, 1.0)) {
        epNodeDtor(rhs);
        return lhs;
    }

    if (epNodeIsSame(lhs, rhs)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(1.0);
    }

    // check for rhs being constants
    if (lhs->type == EP_NODE_CONSTANT && rhs->type == EP_NODE_CONSTANT) {
        EpNode *result = EP_CONST(lhs->constant / rhs->constant);

        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return result;
    }

    return EP_DIV(lhs, rhs);
} // epOptimizedDiv

/**
 * @brief addition optimization function
 * 
 * @param[in] lhs left operand (nullable)
 * @param[in] rhs right operand (nullable)
 * 
 * @note operands assumed to be already optimal
 * 
 * @return created node
 */
static EpNode * epOptimizedAdd( EpNode *lhs, EpNode *rhs ) {
    if (lhs == NULL || rhs == NULL) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return NULL;
    }

    // check for lhs being neutral element
    if (lhs->type == EP_NODE_CONSTANT && epDoubleIsSame(lhs->constant, 0.0)) {
        epNodeDtor(lhs);
        return rhs;
    }

    // check for rhs being neutral element
    if (rhs->type == EP_NODE_CONSTANT && epDoubleIsSame(rhs->constant, 0.0)) {
        epNodeDtor(rhs);
        return lhs;
    }

    if (epNodeIsSame(lhs, rhs)) {
        epNodeDtor(rhs);
        return epOptimizedMul(EP_CONST(2.0), lhs);
    }

    // check for rhs being constants
    if (lhs->type == EP_NODE_CONSTANT && rhs->type == EP_NODE_CONSTANT) {
        EpNode *result = EP_CONST(lhs->constant * rhs->constant);

        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return result;
    }

    return EP_ADD(lhs, rhs);
} // epOptimizedAdd

/**
 * @brief substraction optimization function
 * 
 * @param[in] lhs left operand (nullable)
 * @param[in] rhs right operand (nullable)
 * 
 * @note operands assumed to be already optimal
 * 
 * @return created node
 */
static EpNode * epOptimizedSub( EpNode *lhs, EpNode *rhs ) {
    if (lhs == NULL || rhs == NULL) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return NULL;
    }

    // check for lhs being neutral element
    if (lhs->type == EP_NODE_CONSTANT && epDoubleIsSame(lhs->constant, 0.0)) {
        epNodeDtor(lhs);
        return EP_NEG(rhs);
    }

    // check for rhs being neutral element
    if (rhs->type == EP_NODE_CONSTANT && epDoubleIsSame(rhs->constant, 0.0)) {
        epNodeDtor(rhs);
        return lhs;
    }

    if (epNodeIsSame(lhs, rhs)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(0.0);
    }

    // check for rhs being constants
    if (lhs->type == EP_NODE_CONSTANT && rhs->type == EP_NODE_CONSTANT) {
        EpNode *result = EP_CONST(lhs->constant - rhs->constant);

        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return result;
    }

    return EP_SUB(lhs, rhs);
} // epOptimizedMul

EpNode * epNodeOptimize( const EpNode *node ) {
    switch (node->type) {
    case EP_NODE_VARIABLE:
    case EP_NODE_CONSTANT:
        return epNodeCopy(node);

    case EP_NODE_BINARY_OPERATOR: {
        EpNode *lhs = epNodeOptimize(node->binaryOperator.lhs);
        EpNode *rhs = epNodeOptimize(node->binaryOperator.rhs);

        switch (node->binaryOperator.op) {
        case EP_BINARY_OPERATOR_ADD: return epOptimizedAdd(lhs, rhs);
        case EP_BINARY_OPERATOR_SUB: return epOptimizedSub(lhs, rhs);
        case EP_BINARY_OPERATOR_MUL: return epOptimizedMul(lhs, rhs);
        case EP_BINARY_OPERATOR_DIV: return epOptimizedDiv(lhs, rhs);
        case EP_BINARY_OPERATOR_POW: return epOptimizedPow(lhs, rhs);
        }
    }

    case EP_NODE_UNARY_OPERATOR: {
        EpNode *op = epNodeOptimize(node->unaryOperator.operand);

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_NEG: {
            if (op->type == EP_NODE_CONSTANT) {
                EpNode *result = EP_CONST(-op->constant);

                epNodeDtor(op);
                return result;
            }

            return EP_NEG(op);
        }

        case EP_UNARY_OPERATOR_SIN: {
            if (op->type == EP_NODE_CONSTANT) {
                EpNode *result = EP_CONST(sin(op->constant));

                epNodeDtor(op);
                return result;
            }

            return EP_SIN(op);
        }

        case EP_UNARY_OPERATOR_COS: {
            if (op->type == EP_NODE_CONSTANT) {
                EpNode *result = EP_CONST(cos(op->constant));

                epNodeDtor(op);
                return result;
            }

            return EP_COS(op);
        }

        case EP_UNARY_OPERATOR_LN: {
            if (op->type == EP_NODE_CONSTANT) {
                EpNode *result = EP_CONST(log(op->constant));

                epNodeDtor(op);
                return result;
            }

            return EP_LN(op);
        }
        }
    }
    }
} // epNodeOptimize

// ep_optimize.c
