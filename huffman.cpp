#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <fstream>
#include <clocale>
#include <stdlib.h>

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
    if (root->left == NULL && root->right == NULL) 
        huffmanCodes[root->key] = code;
    
    // Рекурсивно проходим по всем узлам и формируем код для каждого символа в таблице
    HuffmanCodes(root->left, huffmanCodes, code + "0");
    HuffmanCodes(root->right, huffmanCodes, code + "1");
}


// Функция для записи закодированных данных в файл
void WriteToFile(const string& encode, ofstream& out, size_t& counter) {
    unsigned char byte = 0;
    unsigned char mask = 1;
    
    counter = 0;
    for (int i = 0; i < encode.size(); i++) {
        byte <<= mask;
        
        if (encode[i] == '1')
            byte |= mask;

        counter++;

        if (counter == 8) {
            out.put(byte);
            byte = 0;
            counter = 0;
        }
    }

    // Обработка не записанных битов
    if (counter > 0) {
        byte <<= (8 - counter); // Дополнение до байта
        out.put(byte);
    }
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


string ReadInEncodeFile(ifstream& in, size_t bits) {
    string encode = "";
    unsigned char byte;
    unsigned char mask = 1;

    while (in.get((char &)byte)) {
        for (int i = 7; i >= 0; i--) {
            unsigned char tmp = (byte >> i);
            if (tmp & mask)
                encode += '1';
            else
                encode += '0';
        }
    }

    if (encode.size() != 0) {
        int extra = 8 - bits;

        if (bits > 0) {
            encode.erase(encode.size() - extra);
        }
    }
    return encode;
}


void HuffmanCoding(ifstream& in, ofstream& out) {
    string sourceText = "";                               // Исходная строка
    string encodeText = "";                               // Закодированная строка
    unordered_map<char, unsigned long long> Tab;          // Таблица частотности символов
    priority_queue<Node*, vector<Node*>, Compare> pQueue; // Очередь с приоритетом
    unordered_map<char, string> huffmanCodes;             // Таблица Хаффмана

    ofstream ht("huffmanCodes.txt");             // Для сохранения таблицы Хаффмана

    if (!ht.is_open()) {
        cout << "File for Huffman Table is not opened!!!" << endl;
        return;
    }

    char c;
    while(in.get(c))
        sourceText += c;

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
    
    size_t bits = 0; 
    WriteToFile(encodeText, out, bits);

    WriteHTreeInFile(huffmanCodes, ht);

    ht << "bits " << bits << endl; 

    in.close();
    out.close();

    cout << "Compression completed!" << endl;

    return;
}


void Decoding(ifstream& in, ifstream& ht) {
    string encodeText = "";
    string decodeText = "";
    unordered_map<char, string> huffmanCodes; // Таблица Хаффмана
    vector<string> fileInVector;

    string line = "";
    while (getline(ht, line))
        fileInVector.push_back(line);

    // До предпоследней строки
    // Так как предпоследняя строка это количество значащих битов
    for (int i = 0; i < fileInVector.size() - 1; i++) {
        string tmp = fileInVector[i];
        huffmanCodes[tmp[0]] = tmp.substr(2);
    }

    line = fileInVector[fileInVector.size() - 1];
    size_t bits = stoi(line.substr(5));

    encodeText = ReadInEncodeFile(in, bits);
    cout << encodeText << endl;
    decodeText = DecoderFromHuffmanCodes(encodeText, huffmanCodes);

    cout << "Decode text is " << decodeText << endl;

    in.close();
    ht.close();
}


int main() {  
    int choice = 0;
    cout << "Choose: " << endl;
    cout << "1: Coder" << endl;
    cout << "2: Decoder" << endl;
    
    cin >> choice;

    if (choice == 1) {
        ifstream in("text.txt");
        ofstream out("encode.txt");
        HuffmanCoding(in, out);
    } else if (choice == 2) {
        ifstream inE("encode.txt");
        ifstream inHT("huffmanCodes.txt");
        Decoding(inE, inHT);
    } else {
        cout << "Choose from list!!!" << endl;
    }

    return 0;
}