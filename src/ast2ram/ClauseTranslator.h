/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ClauseTranslator.h
 *
 * Translator for clauses from AST to RAM
 *
 ***********************************************************************/

#pragma once

#include "souffle/RamTypes.h"
#include "souffle/utility/ContainerUtil.h"
#include <map>
#include <vector>

namespace souffle {
class SymbolTable;
}

namespace souffle::ast {
class Aggregator;
class Argument;
class Atom;
class Clause;
class Constant;
class IntrinsicFunctor;
class Node;
class RecordInit;
}  // namespace souffle::ast

namespace souffle::ram {
class Condition;
class Expression;
class Operation;
class Relation;
class Statement;
}  // namespace souffle::ram

namespace souffle::ast2ram {

class TranslatorContext;
class ValueIndex;

class ClauseTranslator {
public:
    ClauseTranslator(const TranslatorContext& context, SymbolTable& symbolTable)
            : context(context), symbolTable(symbolTable) {}

    /** Generate RAM code for a clause */
    Own<ram::Statement> translateClause(
            const ast::Clause& clause, const ast::Clause& originalClause, int version = 0);

protected:
    const TranslatorContext& context;
    SymbolTable& symbolTable;

    // value index to keep track of references in the loop nest
    Own<ValueIndex> valueIndex = mk<ValueIndex>();

    virtual Own<ram::Operation> createProjection(const ast::Clause& clause) const;
    virtual Own<ram::Condition> createCondition(const ast::Clause& originalClause) const;

    /** apply constraint filters to a given operation */
    Own<ram::Operation> filterByConstraints(size_t level, const std::vector<ast::Argument*>& arguments,
            Own<ram::Operation> op, bool constrainByFunctors = true) const;

private:
    std::vector<const ast::Argument*> generators;
    std::vector<const ast::Node*> operators;

    Own<ast::Clause> getReorderedClause(const ast::Clause& clause, const int version) const;

    int addGeneratorLevel(const ast::Argument* arg);
    int addOperatorLevel(const ast::Node* node);

    void indexClause(const ast::Clause& clause);
    void indexAtoms(const ast::Clause& clause);
    void indexAggregators(const ast::Clause& clause);
    void indexMultiResultFunctors(const ast::Clause& clause);
    void indexNodeArguments(int nodeLevel, const std::vector<ast::Argument*>& nodeArgs);
    void indexAggregator(const ast::Aggregator& agg);

    Own<ram::Statement> createRamQuery(
            const ast::Clause& clause, const ast::Clause& originalClause, int version);
    Own<ram::Operation> addVariableBindingConstraints(Own<ram::Operation> op) const;
    Own<ram::Operation> addBodyLiteralConstraints(const ast::Clause& clause, Own<ram::Operation> op) const;
    Own<ram::Operation> addGeneratorLevels(Own<ram::Operation> op) const;
    Own<ram::Operation> addVariableIntroductions(const ast::Clause& clause, const ast::Clause& originalClause,
            int version, Own<ram::Operation> op);
    Own<ram::Operation> addEntryPoint(const ast::Clause& originalClause, Own<ram::Operation> op) const;
    Own<ram::Operation> addAtomScan(Own<ram::Operation> op, const ast::Atom* atom, const ast::Clause& clause,
            const ast::Clause& originalClause, int curLevel, int version) const;
    Own<ram::Operation> addRecordUnpack(
            Own<ram::Operation> op, const ast::RecordInit* rec, int curLevel) const;

    // Return the write-location for the generator, or {} if an equivalent arg was already seen
    std::optional<int> addGenerator(const ast::Argument& arg);

    static RamDomain getConstantRamRepresentation(SymbolTable& symbolTable, const ast::Constant& constant);
    static Own<ram::Expression> translateConstant(SymbolTable& symbolTable, const ast::Constant& constant);

    Own<ram::Operation> instantiateAggregator(
            Own<ram::Operation> op, const ast::Aggregator* agg, int curLevel) const;
    Own<ram::Operation> instantiateMultiResultFunctor(
            Own<ram::Operation> op, const ast::IntrinsicFunctor* inf, int curLevel) const;
};

}  // namespace souffle::ast2ram
