/**
 * @brief node computation implementation file
 */

#include <assert.h>
#include <math.h>
#include <string.h>

#include "ep.h"

EpNodeComputeResult epNodeCompute(
    const EpNode     * node,
    const EpVariable * variables,
    size_t             variableCount
) {
    assert(node != NULL);
    assert(variableCount == 0 || variableCount != 0 && variables != NULL);

    switch (node->type) {
    case EP_NODE_VARIABLE: {
        // try to find corresponding variable in table
        for (size_t i = 0; i < variableCount; i++)
            if (strcmp(node->variable, variables[i].name) == 0)
                return (EpNodeComputeResult) {
                    .status = EP_NODE_COMPUTE_OK,
                    .ok = variables[i].value,
                };
        
        return (EpNodeComputeResult) {
            .status = EP_NODE_COMPUTE_UNKNOWN_VARIABLE,
            .unknownVariable = node->variable
        };
    }

    case EP_NODE_CONSTANT:
        return (EpNodeComputeResult) {
            .status = EP_NODE_COMPUTE_OK,
            .ok = node->constant,
        };

    case EP_NODE_BINARY_OPERATOR: {
        EpNodeComputeResult lResult = epNodeCompute(node->binaryOperator.lhs, variables, variableCount);
        if (lResult.status != EP_NODE_COMPUTE_OK)
            return lResult;
        EpNodeComputeResult rResult = epNodeCompute(node->binaryOperator.rhs, variables, variableCount);
        if (rResult.status != EP_NODE_COMPUTE_OK)
            return rResult;

        double lhs = lResult.ok;
        double rhs = rResult.ok;

        double result = 0.0;

        switch (node->binaryOperator.op) {
        case EP_BINARY_OPERATOR_ADD: result = lhs + rhs;     break;
        case EP_BINARY_OPERATOR_SUB: result = lhs - rhs;     break;
        case EP_BINARY_OPERATOR_MUL: result = lhs * rhs;     break;
        case EP_BINARY_OPERATOR_DIV: result = lhs / rhs;     break;
        case EP_BINARY_OPERATOR_POW: result = pow(lhs, rhs); break;
        }

        return (EpNodeComputeResult) {
            .status = EP_NODE_COMPUTE_OK,
            .ok = result,
        };
    }
    case EP_NODE_UNARY_OPERATOR: {
        EpNodeComputeResult operandResult = epNodeCompute(node->unaryOperator.operand, variables, variableCount);
        if (operandResult.status != EP_NODE_COMPUTE_OK)
            return operandResult;
        double operand = operandResult.ok;
        double result = 0.0;

        switch (node->unaryOperator.op) {
        case EP_UNARY_OPERATOR_SIN: result = sin(operand); break;
        case EP_UNARY_OPERATOR_COS: result = cos(operand); break;
        case EP_UNARY_OPERATOR_NEG: result =   -(operand); break;
        case EP_UNARY_OPERATOR_LN : result = log(operand); break;
        }

        return (EpNodeComputeResult) {
            .status = EP_NODE_COMPUTE_OK,
            .ok = result,
        };
    }
    }
} // epNodeCompute

// ep_compute.c
