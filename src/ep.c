/**
 * @brief expression processor 'core' functionality implementation file
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include "ep.h"

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

    node->unaryOperator.op      = op;
    node->unaryOperator.operand = operand;

    return node;
} // epNodeUnaryOperator

void epPrintExpression( FILE *out, const EpNode *node ) {
    assert(node != NULL);

    switch (node->type) {
    case EP_NODE_CONSTANT: {
        fprintf(out, "%lf", node->constant);
        break;
    }

    case EP_NODE_VARIABLE: {
        fprintf(out, "%s", node->variable);
        break;
    }

    case EP_NODE_UNARY_OPERATOR: {
        const char *unaryOperatorText = "";

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_SIN: unaryOperatorText = "sin"; break;
        case EP_UNARY_OPERATOR_COS: unaryOperatorText = "cos"; break;
        case EP_UNARY_OPERATOR_NEG: unaryOperatorText =   "-"; break;
        }

        fprintf(out, "%s ", unaryOperatorText);
        epPrintExpression(out, node->unaryOperator.operand);
        break;
    }

    case EP_NODE_BINARY_OPERATOR: {
        const char *binaryOperatorText = "";

        switch (node->binaryOperator.op) {
        case EP_BINARY_OPERATOR_ADD: binaryOperatorText = "+"; break;
        case EP_BINARY_OPERATOR_SUB: binaryOperatorText = "-"; break;
        case EP_BINARY_OPERATOR_MUL: binaryOperatorText = "*"; break;
        case EP_BINARY_OPERATOR_DIV: binaryOperatorText = "/"; break;
        case EP_BINARY_OPERATOR_POW: binaryOperatorText = "^"; break;
        }

        fprintf(out, "(");
        epPrintExpression(out, node->binaryOperator.lhs);
        fprintf(out, " %s ", binaryOperatorText);
        epPrintExpression(out, node->binaryOperator.rhs);
        fprintf(out, ")");
        break;
    }
    }
} // epPrintExpression

// ep.c
