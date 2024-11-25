/**
 * @brief dump implementation file
 */

#include <assert.h>

#include "ep.h"

static void epDumpParsedExpression( FILE *out, const EpNode *node ) {
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
        case EP_UNARY_OPERATOR_LN : unaryOperatorText =  "ln"; break;
        }

        fprintf(out, "%s ", unaryOperatorText);
        epDumpParsedExpression(out, node->unaryOperator.operand);
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
        epDumpParsedExpression(out, node->binaryOperator.lhs);
        fprintf(out, " %s ", binaryOperatorText);
        epDumpParsedExpression(out, node->binaryOperator.rhs);
        fprintf(out, ")");
        break;
    }
    }
} // epDumpParsedExpression

static bool epDumpBinaryRequiresSurround( int currentPriorirty, const EpNode *node ) {
    return true
        && node->type == EP_NODE_BINARY_OPERATOR
        && currentPriorirty > epBinaryOperatorGetPriority(node->binaryOperator.op)
    ;
} // epDumpBinaryRequiresSurround

static bool epDumpUnaryOperandRequiresSurround( const EpNode *node ) {
    assert(node->type == EP_NODE_UNARY_OPERATOR);

    bool notRequires = true
        && node->unaryOperator.op == EP_UNARY_OPERATOR_NEG
        && (false
            || node->unaryOperator.operand->type == EP_NODE_BINARY_OPERATOR
                && epBinaryOperatorGetPriority(node->unaryOperator.operand->binaryOperator.op)
                    > epBinaryOperatorGetPriority(EP_BINARY_OPERATOR_ADD)
            || node->unaryOperator.operand->type == EP_NODE_UNARY_OPERATOR
        )
    ;

    return !notRequires;
} // epDumpUnaryOperandRequiresSurround

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

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_NEG : fprintf(out,   "-"); break;
        case EP_UNARY_OPERATOR_SIN : fprintf(out, "sin"); break;
        case EP_UNARY_OPERATOR_COS : fprintf(out, "cos"); break;
        case EP_UNARY_OPERATOR_LN  : fprintf(out,  "ln"); break;
        }
        if (surround) fprintf(out, "(");
        epDumpInfixExpression(out, node->unaryOperator.operand);
        if (surround) fprintf(out, ")");
        break;
    }
    }
} // epDumpInfixExpression

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

        if (node->binaryOperator.op == EP_BINARY_OPERATOR_DIV) {
            fprintf(out, "\\frac");
            epDumpTex(out, node->binaryOperator.lhs);
            epDumpTex(out, node->binaryOperator.rhs);
        } else {
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
            case EP_BINARY_OPERATOR_POW: fprintf(out,      "^"); break;

            case EP_BINARY_OPERATOR_DIV: assert(false && "This case is unreachable");
            }

            if (surroundRhs) fprintf(out, "(");
            epDumpTex(out, node->binaryOperator.rhs);
            if (surroundRhs) fprintf(out, ")");
        }

        break;
    }

    case EP_NODE_UNARY_OPERATOR: {
        bool surround = epDumpUnaryOperandRequiresSurround(node);

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_NEG : fprintf(out,   "-"); break;
        case EP_UNARY_OPERATOR_SIN : fprintf(out, "sin"); break;
        case EP_UNARY_OPERATOR_COS : fprintf(out, "cos"); break;
        case EP_UNARY_OPERATOR_LN  : fprintf(out,  "ln"); break;
        }
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
    case EP_DUMP_PARSED_EXPRESSION: epDumpParsedExpression(out, node); break;
    case EP_DUMP_INFIX_EXPRESSION : epDumpInfixExpression (out, node); break;
    case EP_DUMP_TEX              : epDumpTex             (out, node); break;
    }
} // epNodeDump

// ep_dump.c
