/**
 * @brief dump implementation file
 */

#include <assert.h>

#include "ep.h"

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

// ep_dump.c
