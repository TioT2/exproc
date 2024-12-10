/**
 * @brief expression processor 'core' functionality implementation file
 */

#include <stdlib.h>

#include <string.h>
#include <assert.h>
#include <math.h>

#include "ep.h"

bool epDoubleIsSame( double lhs, double rhs ) {
    return fabs(lhs - rhs) < EP_DOUBLE_EPSILON;
} // epDoubleIsSame

bool epNodeIsSame( const EpNode *lhs, const EpNode *rhs ) {
    assert(lhs != NULL);
    assert(rhs != NULL);

    if (lhs->type != rhs->type)
        return false;

    switch (lhs->type) {
    case EP_NODE_VARIABLE:
        return strcmp(lhs->variable, rhs->variable) == 0;

    case EP_NODE_CONSTANT:
        return epDoubleIsSame(lhs->constant, rhs->constant);

    case EP_NODE_BINARY_OPERATOR:
        return true
            && lhs->binaryOperator.op == rhs->binaryOperator.op
            && epNodeIsSame(lhs->binaryOperator.lhs, rhs->binaryOperator.lhs)
            && epNodeIsSame(lhs->binaryOperator.rhs, rhs->binaryOperator.rhs)
        ;

    case EP_NODE_UNARY_OPERATOR:
        return lhs->unaryOperator.op == rhs->unaryOperator.op
            && epNodeIsSame(lhs->unaryOperator.operand, rhs->unaryOperator.operand);
    }
} // epNodeIsSame


EpNode * epNodeCopy( const EpNode *const node ) {
    assert(node != NULL);
    EpNode *copy = (EpNode *)calloc(1, sizeof(EpNode));

    if (copy == NULL)
        return NULL;

    copy->type = node->type;

    switch (node->type) {

    // these cases do not require further copying
    case EP_NODE_VARIABLE: {
        memcpy(copy->variable, node->variable, EP_NODE_VAR_MAX);
        return copy;
    }

    case EP_NODE_CONSTANT: {
        copy->constant = node->constant;
        return copy;
    }

    case EP_NODE_BINARY_OPERATOR: {
        copy->binaryOperator.op = node->binaryOperator.op;

        copy->binaryOperator.lhs = epNodeCopy(node->binaryOperator.lhs);
        copy->binaryOperator.rhs = epNodeCopy(node->binaryOperator.rhs);

        if (copy->binaryOperator.lhs == NULL || copy->binaryOperator.rhs == NULL) {
            epNodeDtor(copy->binaryOperator.lhs);
            epNodeDtor(copy->binaryOperator.rhs);
            free(copy);
            return NULL;
        }
        return copy;
    }

    case EP_NODE_UNARY_OPERATOR: {
        copy->unaryOperator.op = node->unaryOperator.op;
        copy->unaryOperator.operand = epNodeCopy(node->unaryOperator.operand);

        if (copy->unaryOperator.operand == NULL) {
            free(copy);
            return NULL;
        }

        return copy;
    }
    }

    free(copy);
    return NULL;
} // epNodeCopy

void epNodeDtor( EpNode *node ) {
    if (node == NULL)
        return;

    switch (node->type) {
    case EP_NODE_VARIABLE:
        break;

    case EP_NODE_CONSTANT:
        break;

    case EP_NODE_BINARY_OPERATOR: {
        epNodeDtor(node->binaryOperator.lhs);
        epNodeDtor(node->binaryOperator.rhs);
        break;
    }

    case EP_NODE_UNARY_OPERATOR: {
        epNodeDtor(node->unaryOperator.operand);
        break;
    }
    }

    free(node);
} // epNodeDtor

/**
 * @brief node allocation function
 * 
 * @return created node pointer (NULL if allocation failed)
 */
static EpNode * epNodeAlloc( void ) {
    return (EpNode *)calloc(sizeof(EpNode), 1);
} // epNodeAllocZeroed

EpNode * epNodeConstant( double value ) {
    EpNode *node = epNodeAlloc();

    if (node == NULL)
        return NULL;

    node->type = EP_NODE_CONSTANT;
    node->constant = value;

    return node;
} // epNodeConstant

EpNode * epNodeVariable( const char *varName ) {
    assert(varName != NULL);

    const size_t length = strlen(varName);

    if (length > EP_NODE_VAR_MAX - 1)
        return NULL;

    EpNode *node = epNodeAlloc();

    if (node == NULL)
        return NULL;

    node->type = EP_NODE_VARIABLE;
    memcpy(node->variable, varName, length);

    return node;
} // epNodeVariable

EpNode * epNodeBinaryOperator( EpBinaryOperator op, EpNode *lhs, EpNode *rhs ) {
    if (lhs == NULL || rhs == NULL) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return NULL;
    }

    EpNode *node = epNodeAlloc();

    if (node == NULL) {
        epNodeDtor(lhs);
        epNodeDtor(rhs);
        return NULL;
    }

    node->type = EP_NODE_BINARY_OPERATOR;

    node->binaryOperator.op  = op;
    node->binaryOperator.lhs = lhs;
    node->binaryOperator.rhs = rhs;

    return node;
} // epNodeBinaryOperator

EpNode * epNodeUnaryOperator( EpUnaryOperator op, EpNode *operand ) {
    if (operand == NULL)
        return NULL;

    EpNode *node = epNodeAlloc();

    if (node == NULL)
        return NULL;

    node->type = EP_NODE_UNARY_OPERATOR;

    node->unaryOperator.op      = op;
    node->unaryOperator.operand = operand;

    return node;
} // epNodeUnaryOperator

int epBinaryOperatorGetPriority( EpBinaryOperator op ) {
    switch (op) {
    case EP_BINARY_OPERATOR_ADD: return 1;
    case EP_BINARY_OPERATOR_SUB: return 1;
    case EP_BINARY_OPERATOR_MUL: return 2;
    case EP_BINARY_OPERATOR_DIV: return 2;
    case EP_BINARY_OPERATOR_POW: return 3;
    }
} // epBinaryOperatorGetPriority

const char * epNodeTypeStr( EpNodeType ty ) {
    switch (ty) {
    case EP_NODE_VARIABLE        : return "variable";
    case EP_NODE_CONSTANT        : return "constant";
    case EP_NODE_BINARY_OPERATOR : return "binary operator";
    case EP_NODE_UNARY_OPERATOR  : return "unary operator";
    }
} // epNodeTypeStr

const char * epBinaryOperatorStr( EpBinaryOperator op ) {
    switch (op) {
    case EP_BINARY_OPERATOR_ADD : return "+";
    case EP_BINARY_OPERATOR_SUB : return "-";
    case EP_BINARY_OPERATOR_MUL : return "*";
    case EP_BINARY_OPERATOR_DIV : return "/";
    case EP_BINARY_OPERATOR_POW : return "^";
    }
} // epBinaryOperatorStr

const char * epUnaryOperatorStr( EpUnaryOperator op ) {
    switch (op) {
        case EP_UNARY_OPERATOR_LN   : return   "ln";
        case EP_UNARY_OPERATOR_NEG  : return    "-";

        case EP_UNARY_OPERATOR_SIN  : return  "sin";
        case EP_UNARY_OPERATOR_COS  : return  "cos";
        case EP_UNARY_OPERATOR_TAN  : return  "tan";
        case EP_UNARY_OPERATOR_COT  : return  "cot";

        case EP_UNARY_OPERATOR_ASIN : return "asin";
        case EP_UNARY_OPERATOR_ACOS : return "acos";
        case EP_UNARY_OPERATOR_ATAN : return "atan";
        case EP_UNARY_OPERATOR_ACOT : return "acot";
    }
} // epUnaryOperatorStr

double epUnaryOperatorApply( EpUnaryOperator op, double operand ) {
    switch (op) {
    case EP_UNARY_OPERATOR_NEG: return -operand;
    case EP_UNARY_OPERATOR_LN : return log(operand);

    case EP_UNARY_OPERATOR_SIN: return sin(operand);
    case EP_UNARY_OPERATOR_COS: return cos(operand);
    case EP_UNARY_OPERATOR_TAN: return tan(operand);
    case EP_UNARY_OPERATOR_COT: return 1.0 / tan(operand);

    case EP_UNARY_OPERATOR_ASIN: return asin(operand);
    case EP_UNARY_OPERATOR_ACOS: return acos(operand);
    case EP_UNARY_OPERATOR_ATAN: return atan(operand);
    case EP_UNARY_OPERATOR_ACOT: return atan(-operand) + M_PI_2;
    }
} // epUnaryOperatorApply

double epBinaryOperatorApply( EpBinaryOperator op, double lhs, double rhs ) {
    switch (op) {
    case EP_BINARY_OPERATOR_ADD: return lhs + rhs;
    case EP_BINARY_OPERATOR_SUB: return lhs - rhs;
    case EP_BINARY_OPERATOR_MUL: return lhs * rhs;
    case EP_BINARY_OPERATOR_DIV: return lhs / rhs;
    case EP_BINARY_OPERATOR_POW: return pow(lhs, rhs);
    }
} // epBinaryOperatorApply

// ep.c
