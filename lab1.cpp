#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

typedef enum {
    IDENTIFIER, INT10, INT8, INT16, OPERATOR, DELIMITER, KEYWORD
} TokenType;

typedef struct {
    TokenType type;
    char value[64];
} Token;

#define MAX_LEN 64

char buffer[MAX_LEN];
int pos = 0;

const char* tokenTypeToString(TokenType type) {
    switch (type) {
    case IDENTIFIER: return "IDN";
    case INT10:      return "INT10";
    case INT8:       return "INT8";
    case INT16:      return "INT16";
    case OPERATOR:   return "OPERATOR";
    case DELIMITER:  return "DELIMITER";
    case KEYWORD:    return "KEYWORD";
    default:         return "UNKNOWN";
    }
}

void toUpperCase(char* str) {
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = toupper(str[i]);  // 将每个字符转为大写
    }
}

void removeTrailingZeros(char* str) {
    int len = strlen(str);
    while (len > 0 && str[len - 1] == '0') {
        str[len - 1] = '\0';  // Remove zero
        len--;
    }
    if (len > 0 && str[len - 1] == '.') {
        str[len - 1] = '\0';  // Remove the decimal point if it's at the end
    }
}

void outPutToken(Token token) {
    if (token.type == INT16 || token.type == INT8 || token.type == INT10) {
        // 如果是十六进制、八进制或者十进制，转换为十进制并输出
        char* ptr;
        double decimalValue = 0.0;
        int base = (token.type == INT16) ? 16 : (token.type == INT8 ? 8 : 10);

        // 如果有小数点，首先分割整数和小数部分
        char* dotPos = strchr(token.value, '.');
        if (dotPos) {
            // 分割整数部分和小数部分
            *dotPos = '\0';  // 临时将小数点变为字符串结束符
            char* integerPart = token.value;
            char* fractionalPart = dotPos + 1;

            // 将整数部分和小数部分分别转换为十进制
            decimalValue = strtol(integerPart, &ptr, base);  // 处理整数部分
            double fractionValue = 0.0;
            int power = 1;

            // 将小数部分转换为十进制
            for (int i = 0; fractionalPart[i] != '\0'; i++) {
                fractionValue = fractionValue * base + (fractionalPart[i] >= '0' && fractionalPart[i] <= '9' ? fractionalPart[i] - '0' : (fractionalPart[i] - 'a' + 10));
                power *= base;
            }
            fractionValue /= power;
            decimalValue += fractionValue;
        }
        else {
            // 只有整数部分，直接转换
            decimalValue = strtol(token.value, &ptr, base);
        }

        // 将结果格式化为字符串并去掉多余的零
        char result[128];
        sprintf(result, "%.10f", decimalValue); // 格式化为浮动数，最多保留10位小数

        // 调用 helper 函数去掉尾部的零
        removeTrailingZeros(result);

        // 输出去掉零后的结果
        printf("%-15s%s\n", tokenTypeToString(token.type), result);
    }
    else if (token.type == OPERATOR || token.type == KEYWORD) {
        // 输出操作符并加下划线
        toUpperCase(token.value);
        printf("%-15s_\n", token.value);
    }
    else {
        printf("%-15s%s\n", tokenTypeToString(token.type), token.value);
    }
}

Token scan() {
    Token token;
    char lexeme[MAX_LEN] = "";
    int i = 0;
    int hasDecimalPoint = 0;  // 标记是否包含小数点
    int isHex = 0;            // 标记是否为十六进制数
    int isOct = 0;            // 标记是否为八进制数

    while (isspace(buffer[pos])) pos++;

    if (isalpha(buffer[pos]) || buffer[pos] == '_') { // 标识符或关键字
        while (isalnum(buffer[pos]) || buffer[pos] == '_') {
            lexeme[i++] = buffer[pos++];
        }
        lexeme[i] = '\0';

        if (strcmp(lexeme, "if") == 0 || strcmp(lexeme, "then") == 0 ||
            strcmp(lexeme, "else") == 0 || strcmp(lexeme, "while") == 0 ||
            strcmp(lexeme, "do") == 0) {
            token.type = KEYWORD;
        }
        else {
            token.type = IDENTIFIER;
        }
        strcpy(token.value, lexeme);
    }
    else if (isdigit(buffer[pos])) { // 处理整数或浮点数
        if (buffer[pos] == '0') {
            if (buffer[pos + 1] == 'x' || buffer[pos + 1] == 'X') {
                pos += 2;
                isHex = 1;
                while (isxdigit(buffer[pos])) {
                    lexeme[i++] = buffer[pos++];
                }

                if (buffer[pos] == '.') {
                    lexeme[i++] = buffer[pos++];
                    hasDecimalPoint = 1;
                    while (isxdigit(buffer[pos])) {
                        lexeme[i++] = buffer[pos++];
                    }
                }
                token.type = INT16; // 16进制
            }
            else if (buffer[pos + 1] == 'o' || buffer[pos + 1] == 'O') {
                pos += 2;
                isOct = 1;
                while (buffer[pos] >= '0' && buffer[pos] <= '7') {
                    lexeme[i++] = buffer[pos++];
                }

                if (buffer[pos] == '.') {
                    lexeme[i++] = buffer[pos++];
                    hasDecimalPoint = 1;
                    while (buffer[pos] >= '0' && buffer[pos] <= '7') {
                        lexeme[i++] = buffer[pos++];
                    }
                }
                token.type = INT8; // 八进制
            }
            else {
                lexeme[i++] = buffer[pos++];
                token.type = INT10; // 十进制
            }
        }
        else {
            while (isdigit(buffer[pos])) {
                lexeme[i++] = buffer[pos++];
            }

            if (buffer[pos] == '.') {  // 如果有小数点，处理小数部分
                lexeme[i++] = buffer[pos++];
                hasDecimalPoint = 1;
                while (isdigit(buffer[pos])) {
                    lexeme[i++] = buffer[pos++];
                }
            }
            token.type = INT10; // 默认是整数类型（十进制）
        }
        lexeme[i] = '\0';
        strcpy(token.value, lexeme);

        // 如果有小数点，则保留小数点后的部分，但仍归类为整数
        if (hasDecimalPoint) {
            // 这里不改变类型，依然归类为 INT10、INT16 或 INT8
        }
    }
    else { // 运算符和分隔符
        token.type = OPERATOR;
        token.value[0] = buffer[pos++];
        token.value[1] = '\0';
    }
    return token;
}

int main() {
    // 循环处理多行输入
    while (1) {
        pos = 0;
        printf("请输入代码（空行结束输入）：\n");
        fgets(buffer, MAX_LEN, stdin);

        // 检查是否是空行，空行时退出
        if (strcmp(buffer, "\n") == 0) {
            break;
        }

        // 检查并去除末尾的换行符
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';  // 去除换行符
        }

        Token token;
        while (pos < strlen(buffer)) {
            token = scan();
            outPutToken(token);
        }
    }
    return 0;
}
