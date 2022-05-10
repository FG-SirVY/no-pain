#include "parser.h"


std::unique_ptr<Expression> Parser::parseParentheses() {
    auto left = tokenizer->peekNextToken();

    if (left->isType(TokenType::OPEN_PAR)) {
        tokenizer->getNextToken();
        auto ret = parseExpression();
        tokenizer->getNextToken(); // Remove closing parentheses
        return ret;
    } else if (left->isType(TokenType::NAME)) {
        tokenizer->getNextToken();
        return std::make_unique<Name>(*left);
    } else if (left->isType(TokenType::INT) || left->isType(TokenType::FLOAT)) {
        tokenizer->getNextToken();
        return std::make_unique<Literal>(*left);
    } else {
        throw std::exception("Wrong type for left operand");
    }
}


std::unique_ptr<Expression> Parser::parseMulOrDiv() {
    auto leftOperand = parseParentheses();

    while (true) {
        auto middle = tokenizer->peekNextToken();
        if (middle->isType(TokenType::MULTIPLY)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<Multiplication>(
                std::move(leftOperand), parseParentheses());
        } else if (middle->isType(TokenType::DIVIDE)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<Division>(
                std::move(leftOperand), parseParentheses());
        } else {
            return leftOperand;
        }
    }
}

std::unique_ptr<Expression> Parser::parseAddOrSub() {
    auto leftOperand = parseMulOrDiv();

    while (true) {
        auto middle = tokenizer->peekNextToken();
        if (middle->isType(TokenType::ADD)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<Addition>(
                std::move(leftOperand), parseMulOrDiv());
        } else if (middle->isType(TokenType::SUBTRACT)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<Subtraction>(
                std::move(leftOperand), parseMulOrDiv());
        } else {
            return leftOperand;
        }
    }
}

std::unique_ptr<Expression> Parser::parseComparison() {
    auto leftOperand = parseAddOrSub();

    while (true) {
        auto middle = tokenizer->peekNextToken();
        if (middle->isType(TokenType::EQUALS)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<EqualComparison>(
                std::move(leftOperand), parseAddOrSub());
        } else if (middle->isType(TokenType::GREATER)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<GreaterThanComparison>(
                std::move(leftOperand), parseAddOrSub());
        } else if (middle->isType(TokenType::GREATER_OR_EQUALS)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<GreaterThanOrEqualComparison>(
                std::move(leftOperand), parseAddOrSub());
        } else if (middle->isType(TokenType::LESS)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<LessThanComparison>(
                std::move(leftOperand), parseAddOrSub());
        } else if (middle->isType(TokenType::LESS_OR_EQUALS)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<LessThanOrEqualComparison>(
                std::move(leftOperand), parseAddOrSub());
        } else if (middle->isType(TokenType::NOT_EQUALS)) {
            tokenizer->getNextToken();
            leftOperand = std::make_unique<NotEqualComparison>(
                std::move(leftOperand), parseAddOrSub());
        } else {
            return leftOperand;
        }
    }
}


std::unique_ptr<Expression> Parser::parseAnd() {
    auto leftOperand = parseComparison();

    while (true) {
        auto middle = tokenizer->peekNextToken();
        if (middle->isType(TokenType::AND)) {
            tokenizer->getNextToken();

            leftOperand = std::make_unique<AndConnective>(
                std::move(leftOperand), parseComparison());
        } else {
            return leftOperand;
        }
    }
}


std::unique_ptr<Expression> Parser::parseOr() {
    auto leftOperand = parseAnd();

    while (true) {
        auto middle = tokenizer->peekNextToken();
        if (middle->isType(TokenType::OR)) {
            tokenizer->getNextToken();

            leftOperand = std::make_unique<OrConnective>(
                std::move(leftOperand), parseComparison());
        } else {
            return leftOperand;
        }
    }
}


std::unique_ptr<Expression> Parser::parseAssignment() {
    auto left = tokenizer->peekNextToken();
    auto leftOperand = parseOr();

    auto middle = tokenizer->peekNextToken();

    if (left->isType(TokenType::NAME) && middle->isType(TokenType::ASSIGN)) {
        auto name = std::make_unique<Name>(*left);
        tokenizer->getNextToken();
        return std::make_unique<Assignment>(std::move(name), parseExpression());
    } else {
        return leftOperand;
    }
}

std::unique_ptr<Expression> Parser::parseBlock() {
    auto token = tokenizer->getNextToken();
    auto block = std::make_unique<Block>();

    while (!token->isType(TokenType::END_OF_FILE)
            && !token->isType(TokenType::CLOSE_BLOCK)) {
        block->addExpression(parseExpression());
        
        token = tokenizer->getNextToken();
    }

    if (token->isType(TokenType::END_OF_FILE)) {
        throw std::exception("Block not closed");
    }

    return block;
}

std::unique_ptr<Expression> Parser::parseExpression() {
    auto token = tokenizer->peekNextToken();

    if (token->isType(TokenType::OPEN_BLOCK)) {
        return parseBlock();
    } else {
        return parseAssignment();
    }
}
