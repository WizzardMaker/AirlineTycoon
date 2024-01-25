#pragma once

#define exprtk_disable_cardinal_pow_optimisation
#define exprtk_disable_comments
#define exprtk_disable_break_continue
#define exprtk_disable_sc_andor
#define exprtk_disable_return_statement
#define exprtk_disable_enhanced_features
#define exprtk_disable_string_capabilities
#define exprtk_disable_superscalar_unroll
#define exprtk_disable_rtl_io
#define exprtk_disable_rtl_io_file
#define exprtk_disable_rtl_vecops
#define exprtk_disable_caseinsensitivity

#include "../libs/exprtk/exprtk.hpp"

typedef exprtk::symbol_table<double> SymbolTable;
typedef exprtk::expression<double> Expression;
typedef exprtk::parser<double> Parser;

class FormulaParser {
  public:
    static FormulaParser &getInstance();
    void initialize();
    double evaluate(const std::string &formula, double power, double weight, double speed, double verbrauch, double tank) const;

  private:
    FormulaParser();
    void readFormulasFromConfig();

    std::unordered_map<std::string, std::string> formulas_;
    bool isInitialized_ = false;
};