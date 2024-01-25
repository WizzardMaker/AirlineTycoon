#include "FormulaParser.h"
#include <iostream>
#include <fstream>


FormulaParser::FormulaParser() {}

FormulaParser &FormulaParser::getInstance() {
    static FormulaParser instance;
    if (!instance.isInitialized_) {
        instance.initialize();
    }
    return instance;
}

void FormulaParser::initialize() {
    
    readFormulasFromConfig();
    isInitialized_ = true;
}

double FormulaParser::evaluate(const std::string &formulaName, double power, double weight, double speed, double verbrauch, double tank) const {

    auto formulaIter = formulas_.find(formulaName);
    if (formulaIter == formulas_.end()) 
    {
        std::cout << "Formula for " << formulaName << " not found" << std::endl;
        return 0.0;
    }

    const std::string &formula = formulaIter->second;
    SymbolTable symbolTable;
    double p2 = power;
    symbolTable.add_variable("power", power);
    symbolTable.add_variable("weight", weight);
    symbolTable.add_variable("speed", speed);
    symbolTable.add_variable("verbrauch", verbrauch);
    symbolTable.add_variable("tank", tank);

    Expression expression;
    expression.register_symbol_table(symbolTable);

    Parser parser;
    parser.compile(formula, expression);

    auto bla = expression.value();
    return bla;
}


void FormulaParser::readFormulasFromConfig() {
    std::ifstream configFile("Data/calculations.txt");

    if (configFile.is_open()) {
        std::string line;
        while (std::getline(configFile, line)) {
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string name = line.substr(0, delimiterPos);
                std::string formula = line.substr(delimiterPos + 1);
                formulas_[name] = formula;
            }
        }
        configFile.close();
    } else {
        formulas_["CalcSpeed"] = "power <= 0 ? 0 : (power - 6000) * (1000 - 270) / (50000 - 6000) + 270";
        formulas_["CalcReichweite"] = "verbrauch <= 0 ? 0 : tank / verbrauch * speed";
        formulas_["EnoughPower"] = "power * 4 >= weight";
    }
}