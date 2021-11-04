#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <cstddef>
#include <vector>
#include <string>
#include <optional>
#include <functional>


// ATTENTION: It is strongly adivised that no reference to the objects represented by the
//      inner class 'TokenVector' (i.e. do not keep reference to a row of tokens). This is
//      because, when method 'addText' is called, all references will be invalidated and may
//      break the program
class TokenMatrix {
    public:

        // Support class, representing a vector of tokens (strings)
        class TokenVector {
            friend class TokenMatrix;

            public:
                TokenVector();
                // Given a line, separate words into tokens by spaces
                TokenVector(std::string line);

                //Get number of tokens a the line
                std::size_t size() const;

                std::string& operator[]( int tokenIndex );
                const std::string& operator[]( int tokenIndex ) const;
            private:
                std::vector<std::string> vector;
        };

        TokenMatrix();
        // Creta a matrix with all tokens
        TokenMatrix(const std::string text);

        // Add a text to the token matrix.
        // ATTENTION: If this method is used, all reference kept locally WILL be INVALIDATED
        // and the program WILL BREAK
        void addText(const std::string text);

        std::size_t size() const;
        std::string str();
        int findLineIndex(const std::string& lineHead);

        TokenVector& operator[]( int lineIndex );
        const TokenVector& operator[](int lineIndex) const;

    private:
        std::vector<TokenVector> matrix;
};

#endif