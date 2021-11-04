#include "Tokenizer.hpp"

#include <sstream>
#include <optional>

// Default constructor
TokenMatrix::TokenMatrix() {}
TokenMatrix::TokenMatrix(const std::string text) {
    this->addText(std::move(text));
}

// Add a text to the token matrix.
// ATTENTION: If this method is used, all reference kept locally WILL be INVALIDATED
// and the program WILL BREAK
void TokenMatrix::addText(const std::string text) {

    std::istringstream input(std::move(text));
    for (std::string line; std::getline(input, line); ) {
        this->matrix.push_back(TokenMatrix::TokenVector(std::move(line)));
        // this->matrix.push_back(std::make_shared<TokenMatrix::TokenVector>(std::move(line)));
    }
}

std::size_t TokenMatrix::size() const {
    return this->matrix.size();
}

// Generate a string from the Token Matrix
std::string TokenMatrix::str() {
    std::string str;

    for (TokenMatrix::TokenVector& line : this->matrix) {
        for (std::string& token : line.vector) {
            str += token + " ";
        }
        // If there was something on this line, remove last extra space
        if (line.vector.size() != 0)
            str.pop_back();

        str += "\n";
    }

    // Remove last '\n'
    if (this->matrix.size() != 0)
        str.pop_back();

    return str;
}

// Getters and setters in squared brackets form for a TokenVector (representing a line)
TokenMatrix::TokenVector& TokenMatrix::operator[]( int lineIndex ) {
    return this->matrix[lineIndex];
}
const TokenMatrix::TokenVector& TokenMatrix::operator[](int lineIndex) const {
    return this->matrix[lineIndex];
}


int TokenMatrix::findLineIndex(const std::string& lineHead) {
    for (unsigned int i = 0; i < this->matrix.size(); i++)
        if (this->matrix[i].vector[0] == lineHead)
            return i;
    return -1;
}

// Default constructor for TokenVector
TokenMatrix::TokenVector::TokenVector() {}
TokenMatrix::TokenVector::TokenVector(std::string line) {

    auto iss = std::istringstream(std::move(line));
    auto str = std::string{};

    while (iss >> str)
        this->vector.push_back(std::move(str));
}

std::size_t TokenMatrix::TokenVector::size() const {
    return this->vector.size();
}

// Getters and setters in squared brackets form for a string (representing a token)
std::string& TokenMatrix::TokenVector::operator[]( int tokenIndex ) {
    return this->vector[tokenIndex];
}
const std::string& TokenMatrix::TokenVector::operator[]( int tokenIndex ) const {
    return this->vector[tokenIndex];
}