/**
 * @brief dump implementation file
 */

#include <assert.h>

#include "ep.h"

/**
 * @brief do this part requires bracket surround or not
 * 
 * @param[in] currentPriority current priority
 * @param[in] node            node to surround (or not)
 * 
 * @return true if surrounding required, false if not
 */
static bool epDumpBinaryRequiresSurround( int currentPriorirty, const EpNode *node ) {
    return true
        && node->type == EP_NODE_BINARY_OPERATOR
        && currentPriorirty > epBinaryOperatorGetPriority(node->binaryOperator.op)
    ;
} // epDumpBinaryRequiresSurround

/**
 * @brief checking if unary operator requires surround
 * 
 * @param[in] node node to check
 * 
 * @return true if requires, false if not
 */
static bool epDumpUnaryOperandRequiresSurround( const EpNode *node ) {
    assert(node->type == EP_NODE_UNARY_OPERATOR);

    bool notRequires = true
        && node->unaryOperator.op == EP_UNARY_OPERATOR_NEG
        && (false
            || node->unaryOperator.operand->type == EP_NODE_BINARY_OPERATOR
                && epBinaryOperatorGetPriority(node->unaryOperator.operand->binaryOperator.op)
                    > epBinaryOperatorGetPriority(EP_BINARY_OPERATOR_ADD)
            || node->unaryOperator.operand->type == EP_NODE_UNARY_OPERATOR
            || node->unaryOperator.operand->type == EP_NODE_CONSTANT
            || node->unaryOperator.operand->type == EP_NODE_VARIABLE
        )
    ;

    return !notRequires;
} // epDumpUnaryOperandRequiresSurround

/**
 * @brief node in infix format dumping function
 * 
 * @param[in] out  file to dump to
 * @param[in] node node to dump
 */
static void epDumpInfixExpression( FILE *out, const EpNode *node ) {
    switch (node->type) {
    case EP_NODE_VARIABLE:
        fprintf(out, "%s", node->variable);
        break;

    case EP_NODE_CONSTANT:
        fprintf(out, "%lf", node->constant);
        break;

    case EP_NODE_BINARY_OPERATOR: {
        int priority = epBinaryOperatorGetPriority(node->binaryOperator.op);
        bool surroundLhs = epDumpBinaryRequiresSurround(priority, node->binaryOperator.lhs);
        bool surroundRhs = epDumpBinaryRequiresSurround(priority, node->binaryOperator.rhs);

        if (surroundLhs) fprintf(out, "(");
        epDumpInfixExpression(out, node->binaryOperator.lhs);
        if (surroundLhs) fprintf(out, ")");

        fprintf(out, " ");
        switch (node->binaryOperator.op) {
        case EP_BINARY_OPERATOR_ADD: fprintf(out, "+"); break;
        case EP_BINARY_OPERATOR_SUB: fprintf(out, "-"); break;
        case EP_BINARY_OPERATOR_MUL: fprintf(out, "*"); break;
        case EP_BINARY_OPERATOR_DIV: fprintf(out, "/"); break;
        case EP_BINARY_OPERATOR_POW: fprintf(out, "^"); break;
        }
        fprintf(out, " ");

        if (surroundRhs) fprintf(out, "(");
        epDumpInfixExpression(out, node->binaryOperator.rhs);
        if (surroundRhs) fprintf(out, ")");


        break;
    }


    case EP_NODE_UNARY_OPERATOR: {
        bool surround = epDumpUnaryOperandRequiresSurround(node);

        fprintf(out, "%s", epUnaryOperatorStr(node->unaryOperator.op));
        if (surround) fprintf(out, "(");
        epDumpInfixExpression(out, node->unaryOperator.operand);
        if (surround) fprintf(out, ")");
        break;
    }
    }
} // epDumpInfixExpression

/**
 * @brief dumping to file in TeX format function
 * 
 * @param[in] out  file to dump to
 * @param[in] node tree to dump
 */
static void epDumpTex( FILE *out, const EpNode *node ) {
    fprintf(out, "{");

    switch (node->type) {
    case EP_NODE_VARIABLE:
        fprintf(out, "%s", node->variable);
        break;

    case EP_NODE_CONSTANT:
        if (epDoubleIsSame((double)(long long int)node->constant, node->constant))
            fprintf(out, "%lld", (long long int)node->constant);
        else
            fprintf(out, "%lf", node->constant);
        break;

    case EP_NODE_BINARY_OPERATOR: {

        int priority = epBinaryOperatorGetPriority(node->binaryOperator.op);
        bool surroundLhs = epDumpBinaryRequiresSurround(priority, node->binaryOperator.lhs);
        bool surroundRhs = epDumpBinaryRequiresSurround(priority, node->binaryOperator.rhs);

        if (surroundLhs) fprintf(out, "(");
        epDumpTex(out, node->binaryOperator.lhs);
        if (surroundLhs) fprintf(out, ")");

        switch (node->binaryOperator.op) {
        case EP_BINARY_OPERATOR_ADD: fprintf(out,      "+"); break;
        case EP_BINARY_OPERATOR_SUB: fprintf(out,      "-"); break;
        case EP_BINARY_OPERATOR_MUL: fprintf(out, "\\cdot"); break;
        case EP_BINARY_OPERATOR_DIV: fprintf(out, "\\over"); break;
        case EP_BINARY_OPERATOR_POW: fprintf(out,      "^"); break;
        }

        if (surroundRhs) fprintf(out, "(");
        epDumpTex(out, node->binaryOperator.rhs);
        if (surroundRhs) fprintf(out, ")");

        break;
    }

    case EP_NODE_UNARY_OPERATOR: {
        bool surround = epDumpUnaryOperandRequiresSurround(node);

        fprintf(out, "%s", epUnaryOperatorStr(node->unaryOperator.op));
        if (surround) fprintf(out, "(");
        epDumpTex(out, node->unaryOperator.operand);
        if (surround) fprintf(out, ")");
        break;
    }

    }

    fprintf(out, "}");
} // epDumpTex

void epNodeDump( FILE *out, const EpNode *node, EpDumpFormat format ) {
    assert(node != NULL);

    switch (format) {
    case EP_DUMP_INFIX_EXPRESSION : epDumpInfixExpression (out, node); break;
    case EP_DUMP_TEX              : epDumpTex             (out, node); break;
    }
} // epNodeDump

// ep_dump.c
