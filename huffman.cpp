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
void WriteToFile(const string& encode, ofstream& out) {
    unsigned char byte = 0;
    int counter = 0;

    for (int i = 0; i < encode.size(); i++) {
        byte <<= 1;
        
        if (encode[i] == '1')
            byte |= 1;

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


int main() {
    setlocale(LC_ALL, "ru-RU.UTF-8");
    string source = ""; // исходная строка
    string encode = ""; // Закодированная строка
    unordered_map<char, unsigned long long> Tab; // Таблица частотности
    priority_queue<Node*, vector<Node*>, Compare> pQueue; // Очередь с приоритетом
    unordered_map<char, string> huffmanCodes; // Таблица Хаффмана
    ifstream in("text.txt");
    ofstream out("encode.txt");

    if (!in.is_open() || !out.is_open()) {
        cout << "File is not opened or not found!\n";
        exit(1);
    }

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

    for (int i = 0; i < source.size(); i++)
        encode += huffmanCodes[source[i]];
    
    WriteToFile(encode, out);

    cout << "Encode string is " << encode << endl;

    // for (auto p: huffmanCodes)
    //     cout << "Char: " << p.first << " Code: " << p.second << endl; 

    return 0;
}