/**
 * @brief optimizer implementation file
 */

#include <assert.h>
#include <math.h>
#include <string.h>

#define _EP_NODE_SHORT_OPERATORS
#include "ep.h"

/**
 * @brief returns true if 'lhs' is constant node that equel to 'num', false if not
 * 
 * @param[in] lhs left hand side (non-null)
 * @param[in] num number to compare with
 * 
 * @return true or false (defined in brief)
 */
static bool epOptimizeIsConstNum( const EpNode *lhs, double num ) {
    return lhs->type == EP_NODE_CONSTANT && epDoubleIsSame(lhs->constant, num);
} // epOptimizeIsConstNum

/**
 * @brief is node constant or not
 * 
 * @param[in] node   true if constant, flase if not
 * @param[in] valDst value destination (non-null, filled if returned true)
 * 
 * @return true if constnat, false if not
 * 
 * @note this functions checks negative by 1 depth
 */
static bool epOptimizeIsConst( const EpNode *node, double *valDst ) {
    if (node->type == EP_NODE_CONSTANT) {
        *valDst = node->constant;
        return true;
    }

    if (node->type == EP_NODE_UNARY_OPERATOR && node->unaryOperator.op == EP_UNARY_OPERATOR_NEG && node->unaryOperator.operand->type == EP_NODE_CONSTANT) {
        *valDst = -node->unaryOperator.operand->constant;
        return true;
    }

    return false;
} // epOptimizeIsConst

/**
 * @brief optimized constant getting function
 * 
 * @param[in] constant to optimize
 * 
 * @return optimized constant (with zero check and -n -> neg(n) optimization)
 */
static EpNode * epOptimizedConstant( double constant ) {
    return epDoubleIsSame(constant, 0.0)
        ? EP_CONST(0.0)
        : constant < 0
            ? EP_NEG(EP_CONST(-constant))
            : EP_CONST(constant);
} // epOptimizedConstant

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
    if (epOptimizeIsConstNum(lhs, 1.0)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(1.0);
    }

    // check for rhs being neutral element
    if (epOptimizeIsConstNum(rhs, 0.0)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(1.0);
    }

    // check for rhs being neutral element
    if (epOptimizeIsConstNum(rhs, 1.0)) {
        epNodeDtor(rhs);
        return lhs;
    }

    return EP_POW(lhs, rhs);
} // epOptimizedPow

/**
 * @brief remove lhs and rhs signs as if they are to be multiplied
 * 
 * @param[in,out] lhsPtr current lhs and new lhs destination (non-null)
 * @param[in,out] rhsPtr current rhs and new rhs destination (non-null)
 * 
 * @return true if product sign should be null, false if not
 */
static bool epOptimizeRemoveSigns( EpNode **lhsPtr, EpNode **rhsPtr ) {
    EpNode *lhs = *lhsPtr;
    EpNode *rhs = *rhsPtr;

    // check for negative element
    bool lhsNeg = lhs->type == EP_NODE_UNARY_OPERATOR && lhs->unaryOperator.op == EP_UNARY_OPERATOR_NEG;
    bool rhsNeg = rhs->type == EP_NODE_UNARY_OPERATOR && rhs->unaryOperator.op == EP_UNARY_OPERATOR_NEG;

    // remove negation
    if (lhsNeg) {
        EpNode *newLhs = epNodeCopy(lhs->unaryOperator.operand);
        epNodeDtor(lhs);
        *lhsPtr = newLhs;
    }

    // remove negation
    if (rhsNeg) {
        EpNode *newRhs = epNodeCopy(rhs->unaryOperator.operand);
        epNodeDtor(rhs);
        *rhsPtr = newRhs;
    }

    return lhsNeg ^ rhsNeg;
} // epOptimizeRemoveSigns

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

    bool isNeg = epOptimizeRemoveSigns(&lhs, &rhs);
    EpNode *result = NULL;

    if (epOptimizeIsConstNum(lhs, 1.0)) { // check for lhs being neutral element
        epNodeDtor(lhs);
        result = rhs;
    } else if (epOptimizeIsConstNum(rhs, 1.0)) { // check for rhs being neutral element
        epNodeDtor(rhs);
        result = lhs;
    } else if (epOptimizeIsConstNum(lhs, 0.0)) { // check for lhs being neutral element
        epNodeDtor(lhs);
        epNodeDtor(rhs);

        isNeg = false;
        result = EP_CONST(0.0);
    } else if (epOptimizeIsConstNum(lhs, 0.0)) { // check for rhs being neutral element
        epNodeDtor(lhs);
        epNodeDtor(rhs);

        isNeg = false;
        result = EP_CONST(0.0);
    } else if (epNodeIsSame(lhs, rhs)) { // check for node duplication
        epNodeDtor(rhs);
        result = EP_POW(lhs, EP_CONST(2.0));
    } else {
        result = EP_MUL(lhs, rhs);
    }

    return isNeg
        ? EP_NEG(result)
        : result;
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

    bool isNeg = epOptimizeRemoveSigns(&lhs, &rhs);
    EpNode *result = NULL;

    if (epOptimizeIsConstNum(lhs, 0.0)) { // check for rhs being zero
        epNodeDtor(lhs);
        epNodeDtor(rhs);

        isNeg = false;
        result = EP_CONST(0.0);
    } else if (epOptimizeIsConstNum(rhs, 1.0)) { // check for rhs being neutral element
        epNodeDtor(rhs);
        result = lhs;
    } else if (epNodeIsSame(lhs, rhs)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);

        isNeg = false;
        result = EP_CONST(1.0);
    } else {
        result = EP_DIV(lhs, rhs);
    }

    return isNeg
        ? EP_NEG(result)
        : result;
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
    if (epOptimizeIsConstNum(lhs, 0.0)) {
        epNodeDtor(lhs);
        return rhs;
    }

    // check for rhs being neutral element
    if (epOptimizeIsConstNum(rhs, 0.0)) {
        epNodeDtor(rhs);
        return lhs;
    }

    if (epNodeIsSame(lhs, rhs)) {
        epNodeDtor(rhs);
        return epOptimizedMul(EP_CONST(2.0), lhs);
    }

    bool isSubstraction = rhs->type == EP_NODE_UNARY_OPERATOR && rhs->unaryOperator.op == EP_UNARY_OPERATOR_NEG;

    if (isSubstraction) {
        EpNode *newRhs = epNodeCopy(rhs->unaryOperator.operand);
        epNodeDtor(rhs);
        rhs = newRhs;
    }

    return isSubstraction
        ? EP_SUB(lhs, rhs)
        : EP_ADD(lhs, rhs);
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
    if (epOptimizeIsConstNum(lhs, 0.0)) {
        epNodeDtor(lhs);
        return EP_NEG(rhs);
    }

    // check for rhs being neutral element
    if (epOptimizeIsConstNum(rhs, 0.0)) {
        epNodeDtor(rhs);
        return lhs;
    }

    if (epNodeIsSame(lhs, rhs)) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return EP_CONST(0.0);
    }

    bool isAddition = rhs->type == EP_NODE_UNARY_OPERATOR && rhs->unaryOperator.op == EP_UNARY_OPERATOR_NEG;

    if (isAddition) {
        EpNode *newRhs = epNodeCopy(rhs->unaryOperator.operand);
        epNodeDtor(rhs);
        rhs = newRhs;
    }

    return isAddition
        ? EP_ADD(lhs, rhs)
        : EP_SUB(lhs, rhs);
} // epOptimizedMul

/**
 * @brief optimized negative
 * 
 * @param[in] node node to get negative of
 */
static EpNode * epNodeOptimizedNeg( EpNode *node ) {
    if (node == NULL)
        return NULL;

    if (node->type == EP_NODE_UNARY_OPERATOR && node->unaryOperator.op == EP_UNARY_OPERATOR_NEG) {
        EpNode *result = epNodeCopy(node->unaryOperator.operand);
        epNodeDtor(node);
        return result;
    }

    return EP_NEG(node);
} // epNodeOptimizedNeg

EpNode * epNodeOptimize( const EpNode *node ) {
    switch (node->type) {
    case EP_NODE_CONSTANT:
        return epOptimizedConstant(node->constant);

    case EP_NODE_VARIABLE:
        return epNodeCopy(node);

    case EP_NODE_BINARY_OPERATOR: {
        EpNode *lhs = epNodeOptimize(node->binaryOperator.lhs);
        EpNode *rhs = epNodeOptimize(node->binaryOperator.rhs);
        double lhsVal = 0.0;
        double rhsVal = 0.0;

        if (epOptimizeIsConst(lhs, &lhsVal) && epOptimizeIsConst(rhs, &rhsVal)) {
            epNodeDtor(lhs);
            epNodeDtor(rhs);

            return epOptimizedConstant(
                epBinaryOperatorApply(node->binaryOperator.op, lhsVal, rhsVal)
            );
        }

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
        double opVal = 0.0;

        if (epOptimizeIsConst(op, &opVal)) {
            epNodeDtor(op);
            return epOptimizedConstant(
                epUnaryOperatorApply(node->unaryOperator.op, opVal)
            );
        }

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_NEG  : return epNodeOptimizedNeg(op);
        case EP_UNARY_OPERATOR_LN   : return EP_LN(op);
        case EP_UNARY_OPERATOR_SIN  : return EP_SIN(op);
        case EP_UNARY_OPERATOR_COS  : return EP_COS(op);
        case EP_UNARY_OPERATOR_TAN  : return EP_TAN(op);
        case EP_UNARY_OPERATOR_COT  : return EP_COT(op);
        case EP_UNARY_OPERATOR_ASIN : return EP_ASIN(op);
        case EP_UNARY_OPERATOR_ACOS : return EP_ACOS(op);
        case EP_UNARY_OPERATOR_ATAN : return EP_ATAN(op);
        case EP_UNARY_OPERATOR_ACOT : return EP_ACOT(op);
        }
    }
    }
} // epNodeOptimize

// ep_optimize.c

