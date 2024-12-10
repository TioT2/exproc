/**
 * @brief complete TeX document for certain node generation function
 */

#include <string.h>
#include <assert.h>

#include "ep.h"

/**
 * @brief expression-friendly operator
 * 
 * @param[in] out  output file
 * @param[in] node node to display
 */
static void epNodeInfoDumpNode( FILE *out, const EpNode *node ) {
    switch (node->type) {
    case EP_NODE_VARIABLE: {
        fprintf(out, "%s", node->variable);
        break;
    }
    case EP_NODE_CONSTANT: {
        fprintf(out, "%lf", node->constant);
        break;
    }
    case EP_NODE_BINARY_OPERATOR: {
        fprintf(out, "(");
        epNodeInfoDumpNode(out, node->binaryOperator.lhs);
        fprintf(out, "%s", epBinaryOperatorStr(node->binaryOperator.op));
        epNodeInfoDumpNode(out, node->binaryOperator.rhs);
        fprintf(out, ")");
        break;
    }

    case EP_NODE_UNARY_OPERATOR: {
        fprintf(out, "(");

        const char *str = epUnaryOperatorStr(node->unaryOperator.op);

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_NEG:
        case EP_UNARY_OPERATOR_LN:
            fprintf(out, "%s(", str);
            epNodeInfoDumpNode(out, node->unaryOperator.operand);
            fprintf(out, ")");
            break;

        case EP_UNARY_OPERATOR_SIN:
        case EP_UNARY_OPERATOR_COS:
        case EP_UNARY_OPERATOR_TAN:
        case EP_UNARY_OPERATOR_COT:
            fprintf(out, "%s(deg(", str);
            epNodeInfoDumpNode(out, node->unaryOperator.operand);
            fprintf(out, "))");
            break;

        case EP_UNARY_OPERATOR_ASIN:
        case EP_UNARY_OPERATOR_ACOS:
        case EP_UNARY_OPERATOR_ATAN:
        case EP_UNARY_OPERATOR_ACOT:
            fprintf(out, "rad(%s(", str);
            epNodeInfoDumpNode(out, node->unaryOperator.operand);
            fprintf(out, "))");
            break;
        }

        fprintf(out, ")");
        break;
    }
    }
} // epNodeInfoDumpNode

static void epNodeGenNodeGetFunctionParameters(
    const EpNode * node,

    const char **  parameters,
    size_t      *  parameterCount,
    size_t         parameterMax
) {
    switch (node->type) {
    case EP_NODE_VARIABLE: {
        for (size_t i = 0; i < *parameterCount; i++)
            if (strcmp(parameters[i], node->variable) == 0)
                return;

        if (*parameterCount < parameterMax)
            parameters[(*parameterCount)++] = node->variable;
        return;
    }
    case EP_NODE_CONSTANT:
        return;

    case EP_NODE_BINARY_OPERATOR:
        epNodeGenNodeGetFunctionParameters(
            node->binaryOperator.lhs,
            parameters,
            parameterCount,
            parameterMax
        );
        epNodeGenNodeGetFunctionParameters(
            node->binaryOperator.rhs,
            parameters,
            parameterCount,
            parameterMax
        );
        return;

    case EP_NODE_UNARY_OPERATOR:
        epNodeGenNodeGetFunctionParameters(
            node->unaryOperator.operand,
            parameters,
            parameterCount,
            parameterMax
        );
        return;
    }
} // epNodeGenNodeGetFunctionParameters

void epNodeGenNodeFunctionInfo( FILE *out, const EpNode *node ) {
    const char *parameters[64] = {NULL};
    size_t parameterCount = 0;
    EpNode *nodeOptimized = epNodeOptimize(node);

    // get node function parameters
    epNodeGenNodeGetFunctionParameters(nodeOptimized, parameters, &parameterCount, 64);

    fprintf(out, "\\documentclass{article}\n");
    fprintf(out, "\\usepackage{graphicx}\n");
    fprintf(out, "\\usepackage{pgfplots}\n");
    fprintf(out, "\\usepackage{tikz}\n");
    fprintf(out, "\\title{Function exploration}\n");
    fprintf(out, "\\author{Tio 4112}\n");
    fprintf(out, "\\date{December 2024}\n");
    fprintf(out, "\\begin{document}\n");
    fprintf(out, "\\maketitle\n");

    fprintf(out, "\\section{Introduction}\n");
    if (node == NULL) {
        fprintf(out, "Internal error occured...\n");
        goto __epNodeGenNodeFunctionInfo__end;
    }

    if (parameterCount == 0) {
        // node MUST BE optimized into constant in case if there's no parameters found in it.
        assert(node->type == EP_NODE_CONSTANT);

        fprintf(out, "Function is just a constant, there is nothing to look at: %lf\n", node->constant);
    } else {
        fprintf(out, "Function: $$");
        epNodeDump(out, nodeOptimized, EP_DUMP_TEX);
        fprintf(out, "$$\n");
    }

    for (size_t i = 0; i < parameterCount; i++) {
        // substitute zeros
        EpSubstitution substitutions[64] = {};
        size_t substitutionCount = 0;

        EpNode *zero = epNodeConstant(0.0);
        EpNode *subConst = epNodeConstant(2.5);
        EpNode *tVar = epNodeVariable("t");

        // substitute taylor series
        for (size_t j = 0; j < parameterCount; j++) {
            EpSubstitution *s = &substitutions[substitutionCount++];
            s->name = parameters[j];

            s->node = i == j
                ? tVar
                : subConst;
        }


        const char *param = parameters[i];

        fprintf(out, "\\section{Exploring function by \"%s\"}\n", param);

        EpNode *substitutedInit = epNodeSubstitute(nodeOptimized, substitutions, substitutionCount);
        EpNode *substituted = epNodeOptimize(substitutedInit);
        epNodeDtor(substitutedInit);

        EpNode *derivativeByParamInit = epNodeDerivative(substituted, "t");
        EpNode *derivativeByParam = epNodeOptimize(derivativeByParamInit);
        epNodeDtor(derivativeByParamInit);

        // calculate taylor series
        EpNode *taylorSeries[8] = {NULL};
        const size_t taylorSeriesSize = 8;

        for (size_t i = 0; i < taylorSeriesSize; i++) {
            EpNode *taylorInit = epNodeTaylor(substituted, "t", zero, i + 1);
            taylorSeries[i] = epNodeOptimize(taylorInit);
            epNodeDtor(taylorInit);
        }

        fprintf(out, "With %lf substituted to parameters except \"%s\" (renamed to \"t\"): $$", subConst->constant, param);
        epNodeDump(out, substituted, EP_DUMP_TEX);
        fprintf(out, "$$\n");

        fprintf(out, "First derivative by \"t\": $$");
        epNodeDump(out, derivativeByParam, EP_DUMP_TEX);
        fprintf(out, "$$\n");

        fprintf(out, "Taylor expansion around 0: $$");
        epNodeDump(out, taylorSeries[taylorSeriesSize - 1], EP_DUMP_TEX);
        fprintf(out, "$$\n");

        // graphs
        fprintf(out, "\\begin{tikzpicture}\n");
        fprintf(out, "\\begin{axis} [axis lines=center]\n");

        // derivative
        fprintf(out, "\\addplot [color = green, smooth, thick, variable = \\t] { ");
        epNodeInfoDumpNode(out, derivativeByParam);
        fprintf(out, "};\n");

        // taylor
        for (size_t i = 0; i < taylorSeriesSize; i++) {
            fprintf(out, "\\addplot [color = lightgray, smooth, thick, variable = \\t] { ");
            epNodeInfoDumpNode(out, taylorSeries[i]);
            fprintf(out, "};\n");
        }

        // function itself
        fprintf(out, "\\addplot [color = black, smooth, thick, variable = \\t] { ");
        epNodeInfoDumpNode(out, substituted);
        fprintf(out, "};\n");

        fprintf(out, "\\end{axis}\n");
        fprintf(out, "\\end{tikzpicture}\n");


        epNodeDtor(derivativeByParam);
        for (size_t i = 0; i < taylorSeriesSize; i++)
            epNodeDtor(taylorSeries[i]);
        epNodeDtor(zero);
        epNodeDtor(subConst);
        epNodeDtor(tVar);
    }

__epNodeGenNodeFunctionInfo__end:

    epNodeDtor(nodeOptimized);
    fprintf(out, "\\end{document}");
} // epNodeGenNodeFunctionInfo

// ep_info.c
