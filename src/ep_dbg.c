/**
 * @brief debug-only functions implementation file
 */

#include "ep.h"

/**
 * @brief node dumping function implementation
 * 
 * @param[in] out  output file
 * @param[in] node node to dump
 */
static void epDbgNodeDumpDotImpl( FILE *out, const EpNode *node ) {
    fprintf(out, "    node%016zX [label = \"{", (size_t)node);

    fprintf(out, "<ptr>pointer: 0x%016zX|<type>type: %s", (size_t)node, epNodeTypeStr(node->type));

    switch (node->type) {
    case EP_NODE_VARIABLE:
        fprintf(out, "|<var>variable name: \\\"%s\\\"", node->variable);
        break;

    case EP_NODE_CONSTANT:
        fprintf(out, "|<const>constant: %lf", node->constant);
        break;

    case EP_NODE_BINARY_OPERATOR:
        fprintf(out, "|<op>binary operator: \\\"%s\\\"", epBinaryOperatorStr(node->binaryOperator.op));
        fprintf(out, "|{<lhs>lhs|<rhs>rhs}");
        break;

    case EP_NODE_UNARY_OPERATOR:
        fprintf(out, "|<op>unary operator: \\\"%s\\\"", epUnaryOperatorStr(node->unaryOperator.op));
        fprintf(out, "|<operand>operand");
        break;
    }

    fprintf(out, "}\"];\n");

    // display sub-expression
    switch (node->type) {
    case EP_NODE_VARIABLE: break;
    case EP_NODE_CONSTANT: break;

    case EP_NODE_BINARY_OPERATOR:
        fprintf(out, "    node%016zX:lhs -> node%016zX;\n", (size_t)node, (size_t)node->binaryOperator.lhs);
        fprintf(out, "    node%016zX:rhs -> node%016zX;\n", (size_t)node, (size_t)node->binaryOperator.rhs);

        epDbgNodeDumpDotImpl(out, node->binaryOperator.lhs);
        epDbgNodeDumpDotImpl(out, node->binaryOperator.rhs);
        break;

    case EP_NODE_UNARY_OPERATOR:
        fprintf(out, "    node%016zX:operand -> node%016zX;\n", (size_t)node, (size_t)node->unaryOperator.operand);

        epDbgNodeDumpDotImpl(out, node->unaryOperator.operand);
        break;
    }
} // epDbgNodeDumpDot

void epDbgNodeDumpDot( FILE *out, const EpNode *node ) {
    fprintf(out, "digraph{\n");
    fprintf(out, "    node [shape = record];\n");
    fprintf(out, "\n");

    epDbgNodeDumpDotImpl(out, node);

    fprintf(out, "}\n");
} // epDbgNodeDumpDot

// ep_dbg.c
