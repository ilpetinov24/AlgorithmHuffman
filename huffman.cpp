#include <iostream>
#include <vector>
#include <queue>
#include <string>
#include <unordered_map>
#include <fstream>
#include <clocale>


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
    while (nodes.size() != 1) {
        Node* l = nodes.top();
        nodes.pop();
        Node* r = nodes.top();
        nodes.pop();

        // Создаем новый узел, в котором эти элементы будут потомками
        // Приоритет нового узла будет равен сумме приоритетов этих элементов
        nodes.push(CreateNode('\0', l->frequency + r->frequency, l, r));
    }

    Tree tree;
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


//Функция для записи в файл
void WriteToFile(const string& encode, ofstream& out, size_t& counter) {
    unsigned char byte = 0;
    counter = 0;
    unsigned char mask = 1;


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
        byte <<= (8 - counter); // дополнение до байта
        out.put(byte);
    }

    return;
}

// // Функция записи дерева Хаффмана в файл
// // Нужна для декодирования
// void WriteHTreeInFile(const unordered_map<char, string>& huffmanCodes, ofstream& out) {
//     for (auto p: huffmanCodes) {
//         if (p.first == '\n') {
//             out << "_" << " " << p.second << endl; // нужно для символа перевода строки
//             continue;
//         }
//         out << p.first << " " << p.second << endl;
//     }
//     return;
// }


string Decoder(Node* root, const string& encode) {
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

    int extra = 8 - bits;

    if (bits > 0) {
        encode.erase(encode.size() - extra); // С учетом терминального символа
        encode += '\0';
    }

    return encode;
}


int main() {
    // Locale не работает...
    //setlocale(LC_ALL, "ru-RU.UTF-8");
    string source = ""; // исходная строка
    string encode = ""; // Закодированная строка
    unordered_map<char, unsigned long long> Tab; // Таблица частотности
    priority_queue<Node*, vector<Node*>, Compare> pQueue; // Очередь с приоритетом
    unordered_map<char, string> huffmanCodes; // Таблица Хаффмана
    ifstream in("text.txt");
    ofstream out("encode.txt");
    //ofstream ht("huffmanTree.txt");
    ifstream inEncode("encode.txt");
    // if (!in.is_open() || !out.is_open()) {
    //     cout << "File is not opened or not found!\n";
    //     exit(1);
    // }

    // Считываю строку
    char ch;
    while(in.get(ch))
        source += ch;
    
    // Заполняю таблицу частотности
    for (int i = 0; i < source.size(); i++)
        Tab[source[i]]++;

    // Заполняем очередь узлами (листьями)
    for (auto p: Tab)
        pQueue.push(CreateNode(p.first, p.second, NULL, NULL));

    Tree tree = BuildTree(pQueue);
    
    // Формирую таблицу
    HuffmanCodes(tree.root, huffmanCodes, "");
    //WriteHTreeInFile(huffmanCodes, ht);
    for (int i = 0; i < source.size(); i++)
        encode += huffmanCodes[source[i]];
    
    size_t extraBits = 0;
    WriteToFile(encode, out, extraBits);

    out.close();

    cout << "Encode string is " << encode << endl;
    string tmp = ReadInEncodeFile(inEncode, extraBits);
    
    for (int i = 0; i < tmp.size(); i++) {
        if (tmp[i] != encode[i]) {
            cout << "i = " << i << endl;
        }
    }
    cout << "Encode string is " << tmp << endl;

    string result = "";
    result = Decoder(tree.root, encode);

    cout << result << endl;

    // for (auto p: huffmanCodes)
    //     cout << "Char: " << p.first << " Code: " << p.second << endl; 

    return 0;
}