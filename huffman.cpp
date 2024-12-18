#include <iostream>
#include <stdio.h>
#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <fstream>
#include <clocale>
#include <bitset>
#include <stdlib.h>
#include <map>
#include <Windows.h>
using namespace std;

// Узел бинарного дерева
struct Node {
    char key;
    // Частота встречаемости символа
    unsigned long long frequency;
    Node *left;
    Node *right;
};

// Отдельная структура для хранения корня дерева
struct Tree { 
    Node* root;
};

// Компаратор для того, чтобы создавать приоритет очереди
// Высший приоритет в очереди будет иметь элемент с наименьшей частотой
// Это требуется для того, чтобы идти от листьев к корню
// То есть внизу будут находиться элементы с наименьшей частотой
struct Compare {
    bool operator() (Node* l, Node* r) {
        return r->frequency < l->frequency;
    }
};

// Функция создания узла дерева
Node* CreateNode(char ch = '\0', unsigned long long freq = 0, Node* l = NULL, Node* r = NULL) {
    Node* node = new Node();
    
    node->key        = ch;
    node->frequency  = freq;
    node->left       = l;
    node->right      = r;

    return node;
}


Tree BuildTree(priority_queue<Node*, vector<Node*>, Compare> nodes) {
    // Повторяем действия до тех пор, пока не получим корень дерева
    Tree tree;
    tree.root = NULL;
    
    if (nodes.size() == 0)
        return tree;
        
    if (nodes.size() == 1) {
        Node* leaf = nodes.top();
        tree.root = CreateNode('\0', leaf->frequency, leaf, NULL);
        return tree;
    }
    
    while (nodes.size() != 1) {
        Node* l = nodes.top();
        nodes.pop();
        Node* r = nodes.top();
        nodes.pop();

        // Создаем новый узел, в котором эти элементы будут потомками
        // Приоритет нового узла будет равен сумме приоритетов этих элементов
        nodes.push(CreateNode('\0', l->frequency + r->frequency, l, r));
    }

    tree.root = nodes.top();
    return tree;
}

// Функция для формирования таблицы Хаффмана
void HuffmanCodes(Node* root, unordered_map<char, string>& huffmanCodes, string code) {
    if (root == NULL)
        return;


    // Если дошли до листьев дерева, то заполняем таблицу
    if (root->left == NULL && root->right == NULL) {
        // Обработка одного символа
        if (code == "") {
           huffmanCodes[root->key] = "0";
           return; 
        }

        huffmanCodes[root->key] = code;
    }
    
    // Рекурсивно проходим по всем узлам и формируем код для каждого символа в таблице
    HuffmanCodes(root->left, huffmanCodes, code + "0");
    HuffmanCodes(root->right, huffmanCodes, code + "1");
}


void WriteBinNumInFile(ofstream& out, int number) {
    unsigned char mask = 1;
    unsigned char byte = 0;
    bitset<32> binNumber(number);
    string binStr = binNumber.to_string();
    int counter = 0;
    for (int i = 0; i < binStr.length(); i++) {
        byte <<= mask;

        if (binStr[i] == '1') {
            byte |= mask;
        }
        counter++;
        if (counter == 8) {
            counter = 0;
            out.put(byte);
            byte = 0;
        }
    }
}

int ReadBinNumInFile(ifstream& in) {
    unsigned char mask = 1;
    int result = 0;
    string binStr = "";

    for (int i = 0; i < 4; i++) {
        unsigned char a = in.get();
        bitset<8> b(a);
        binStr += b.to_string();
    }

    bitset<32> tmp(binStr);

    return tmp.to_ullong();
}


// Функция для записи закодированных данных в файл
void WriteToFile(const string& encode, ofstream& out, map<char, int> Tab) {
    unsigned char byte = 0;
    unsigned char mask = 1;
    size_t counter = 0;

    ofstream tempFile("tmp.txt", ios::binary);
    
    counter = 0;
    for (int i = 0; i < encode.size(); i++) {
        byte <<= mask;
        
        if (encode[i] == '1')
            byte |= mask;

        counter++;

        if (counter == 8) {
            tempFile.put(byte);
            byte = 0;
            counter = 0;
        }
    }

    // Обработка не записанных битов
    if (counter > 0) {
        byte <<= (8 - counter); // Дополнение до байта
        tempFile.put(byte);
    }

    tempFile.close();

    ifstream rtempFile("tmp.txt", ios::binary);

    out << counter;
    

    // Запись частот в файл
    for (auto p: Tab) {
        if (p.first == '\n') {
            out << '_';
            WriteBinNumInFile(out, p.second);
            continue;
        }
        out << p.first;
        WriteBinNumInFile(out, p.second);
    }
    out << "\n";

    char current;
    while (rtempFile.get(current))
        out << current;

    rtempFile.close();
    remove("tmp.txt");
}

// Функция записи дерева Хаффмана в файл
// Нужна для декодирования
void WriteHTreeInFile(const unordered_map<char, string>& huffmanCodes, ofstream& out) {
    for (auto p: huffmanCodes) {
        if (p.first == '\n') {
            out << "_" << " " << p.second << endl; // нужно для символа перевода строки
            continue;
        }
        out << p.first << " " << p.second << endl;
    }
    return;
}


// Декодирование по дереву
string DecoderFromTree(Node* root, const string& encode) {
    string decode = "";
    Node* current = root;

    for (int i = 0; i < encode.size(); i++) {
        if (encode[i] == '0')
            current = current->left;
        if (encode[i] == '1')
            current = current->right;

        if (current->left == NULL && current->right == NULL) {
            decode += current->key;
            current = root;
        }
    }

    return decode;
}


// Декодирование при помощи таблицы Хаффмана
string DecoderFromHuffmanCodes(const string& encode, const unordered_map<char, string>& huffmanCodes) {
    string decode = "";
    string code = "";

    // Проверка на один символ
    if (huffmanCodes.size() == 1) {
        for (auto p: huffmanCodes)
            decode += p.first;
        return decode;
    }

    for (int i = 0; i < encode.size(); i++) {
        code += encode[i];
        for (auto p: huffmanCodes) {
            if (p.second == code) {
                if (p.first == '_') {
                    decode += '\n';
                    code = ""; 
                    break;
                }
                decode += p.first;
                code = "";
                break;
            }
        }
    }

    return decode;
}


string ReadInEncodeFile(ifstream& file, map<char, int>& Tab) {
    string encode = "";
    unsigned char byte;
    unsigned char mask = 1;
    size_t counterBits = file.get() - '0';

    char current;
    while (file.get(current)) {

        if (current == '_') {
            Tab['\n'] = ReadBinNumInFile(file);
            continue;
        }
        if (current == '\n') {
            break;
        }
        Tab[current] = ReadBinNumInFile(file);
    }
    

    while (file.get((char &)byte)) {
        for (int i = 7; i >= 0; i--) {
            unsigned char tmp = (byte >> i);
            if (tmp & mask)
                encode += '1';
            else
                encode += '0';
        }
    }

    encode = encode.substr(0, encode.size() - (8 - counterBits));
    
    return encode;
}

void HuffmanCoding(ifstream& in, ofstream& out) {
    string sourceText = "";                               // Исходная строка
    string encodeText = "";                               // Закодированная строка
    map<char, int> Tab;                                   // Таблица частотности символов
    priority_queue<Node*, vector<Node*>, Compare> pQueue; // Очередь с приоритетом
    unordered_map<char, string> huffmanCodes;             // Таблица Хаффмана

    char c;
    while (in.get(c)) {
        sourceText += c;
    }
    // Заполняю таблицу частотности
    for (int i = 0; i < sourceText.size(); i++)
        Tab[sourceText[i]]++;
    

    // Заполняем очередь узлами (изначально листьями)
    for (auto p: Tab)
        pQueue.push(CreateNode(p.first, p.second, NULL, NULL));

    Tree tree = BuildTree(pQueue);

    // Формирую таблицу
    HuffmanCodes(tree.root, huffmanCodes, "");

    // Закодированный файл
    for (int i = 0; i < sourceText.size(); i++)
        encodeText += huffmanCodes[sourceText[i]];
    
    // cout << encodeText << endl;

    size_t bits = 0; 
    WriteToFile(encodeText, out, Tab);

    in.close();
    out.close();

    cout << "Compression completed!" << endl;
    system("pause");
    return;
}


void Decoding(ifstream& in, ofstream& out) {
    string encodeText = "";
    string decodeText = "";
    unordered_map<char, string> huffmanCodes; // Таблица Хаффмана
    map<char, int> Tab;
    priority_queue<Node*, vector<Node*>, Compare> pQueue; // Очередь с приоритетом     
   
    encodeText = ReadInEncodeFile(in, Tab);

    // Заполняем очередь узлами (изначально листьями)
    for (auto p: Tab)
        pQueue.push(CreateNode(p.first, p.second, NULL, NULL));

    Tree tree = BuildTree(pQueue);


    // Формирую таблицу
    HuffmanCodes(tree.root, huffmanCodes, "");

    decodeText = DecoderFromHuffmanCodes(encodeText, huffmanCodes);
    
    out << decodeText;

    in.close();
    out.close();

    cout << "Decompression completed!" << endl;
    cout << "Text save in decode.txt" << endl;
    system("pause");
}


int main() {
    setlocale(LC_ALL, "ru_RU.cp1251");
    system("chcp 1251");
    system("cls");
    std::locale::global(std::locale(""));
    
    int choice = 0;
    cout << "Choose: " << endl;
    cout << "1: Coder" << endl;
    cout << "2: Decoder" << endl;
    
    cin >> choice;

    if (choice == 1) {
        ifstream in("text.txt", ios::binary);
        ofstream out("encode.txt", ios::binary);
        if (!in.is_open() || !out.is_open()) {
            cout << "File is not found!!!" << endl;
            exit(1);
        }

        HuffmanCoding(in, out);
    } else if (choice == 2) {
        ifstream input("encode.txt", ios::binary);
        ofstream out("decode.txt", ios::binary);
        if (!input.is_open() || !out.is_open()) {
            cout << "Files is not found!!!" << endl;
            exit(1);
        }
        
        Decoding(input, out);
    } else 
        cout << "Choose from list!!!" << endl;

    return 0;
}